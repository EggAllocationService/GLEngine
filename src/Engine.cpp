#include "Engine.h"
//
// Created by Kyle Smith on 2025-09-26.
//
#include "engine_GLUT.h"
#include "Colors.h"
#include <map>
#include <algorithm>
#include <stack>

#include "3d/ActorSceneComponent.h"
#include "3d/DefaultPawn.h"


namespace glengine {
    // since GLUT doesn't let us store custom state on windows,
    // we need a static map to keep track of which Engine instance
    // owns which window
    static std::map<int, Engine *> Instances;

#pragma region event handlers
    /// These event handlers are needed because we can't pass state-owning functions (i.e. lambdas) to C as
    /// raw function pointers. These will find the Engine instance that owns the active window ID and
    /// call the appropriate instance function
    static void renderExec() {
        int currentWindow = glutGetWindow();
        if (Instances.contains(currentWindow)) {
            Instances[currentWindow]->Render();
        }
    }

    static void updateExec() {
        int currentWindow = glutGetWindow();
        if (Instances.contains(currentWindow)) {
            Instances[currentWindow]->Update();
        }
    }

    static void reshapeExec(int x, int y) {
        int currentWindow = glutGetWindow();
        if (Instances.contains(currentWindow)) {
            Instances[currentWindow]->SetWindowSize(int2(x, y));
        }
    }

    static void clickExec(int button, int state, int x, int y) {
        int currentWindow = glutGetWindow();
        if (Instances.contains(currentWindow)) {
            // have to invert y-axis
            float2 pos = float2(x, y);
            pos.y = Instances[currentWindow]->GetWindowSize().y - pos.y;

            Instances[currentWindow]->GetMouseManager()->Click(button, state, pos);
        }
    }

    static void mouseMoveExec(int x, int y) {
        int currentWindow = glutGetWindow();
        if (Instances.contains(currentWindow)) {
            // invert y coordinate
            float2 pos = float2(x, Instances[currentWindow]->GetWindowSize().y - y);

            Instances[currentWindow]->GetMouseManager()->HandleMotion(pos);
        }
    }
#pragma endregion

    Engine::Engine(const std::string &windowName, int2 size) {
        mouseManager = new input::MouseManager(this);

        windowSize = size;
        glutInitWindowSize(windowSize.x, windowSize.y);
        glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE | GLUT_DEPTH | GLUT_STENCIL);

        windowHandle = glutCreateWindow(windowName.c_str());
        Instances[windowHandle] = this;

        glutIdleFunc(updateExec);
        glutDisplayFunc(renderExec);
        glutReshapeFunc(reshapeExec);
        glutMouseFunc(clickExec);
        glutPassiveMotionFunc(mouseMoveExec);
        glutMotionFunc(mouseMoveExec);

        setLastUpdate();
    }

    Engine::~Engine() {
        // destroy window
        glutDestroyWindow(windowHandle);

        // remove from global instances table
        auto iter = Instances.find(windowHandle);
        if (iter != Instances.end()) {
            Instances.erase(iter);
        }

        // destroy owned objects
        delete mouseManager;
    }

    void Engine::Update() {
        // main update loop

        // if game logic requested a quit on the previous frame, then do that now
        if (quitRequested) {
            glutDestroyWindow(windowHandle);
            exit(0);
        }

        double delta = calculateDeltaTime();

        mouseManager->Update();

        updateWidgets(delta);

        updateActors(delta);

        setLastUpdate();

        glutPostRedisplay();
    }

    void Engine::Possess(std::shared_ptr<world::Pawn> target) {
        possessedPawn = target;
        target->GetActiveCamera()->SetProjectionMatrix();
    }

    void Engine::SetWindowSize(int2 size) {
        windowSize = size;
        glViewport(0, 0, windowSize.x, windowSize.y);

        if (auto pawn = possessedPawn.lock()) {
            pawn->GetActiveCamera()->SetProjectionMatrix();
        }
    }

    void Engine::Render() {
        // perform all rendering then swap the double-buffered view
        clearBuffers();

        renderWorld();

        renderWidgets();

        glutSwapBuffers();
    }

    double Engine::calculateDeltaTime() const {
        auto now = std::chrono::steady_clock::now();
        auto time = std::chrono::duration_cast<std::chrono::duration<double> >(now - lastUpdate);

        return time.count();
    }

    void Engine::clearBuffers() {
        glClearColor(0, 0, 0, 1);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
    }

    void Engine::setLastUpdate() {
        lastUpdate = std::chrono::steady_clock::now();
    }

    void Engine::renderWidgets() {
        // reset opengl matrices
        glMatrixMode(GL_PROJECTION);
        float savedProj[16];
        glGetFloatv(GL_PROJECTION_MATRIX, savedProj);
        glLoadIdentity();

        glMatrixMode(GL_MODELVIEW);
        glLoadIdentity();

        // a matrix stack lets us render nested widgets very conveniently
        // also provides valuable tools for widgets to render parent-child objects
        MatrixStack2D stack = MatrixStack2D();

        // generate screen pixel coord -> gl coord matrix
        // this matrix scales pixel coordinates by 2.0 / (window size), giving us coords in the range
        // 0 to 2. it then translates by -1 on both axis to fit into the OpenGL coordinate space of -1 to 1
        mat3 screenTransform = mat3::identity();
        screenTransform[0]->set(0, 2.0f / windowSize.x);
        screenTransform[1]->set(1, 2.0f / windowSize.y);
        screenTransform[2]->set(0, -1.0f);
        screenTransform[2]->set(1, -1.0f);

        stack.Push(screenTransform);

        // render each widget one by one
        // the `widgets` array was sorted back-to-front by z-index at the end of updateWidgets(), so we don't
        // need to worry about that here.
        for (const auto& widget: widgets) {
            stack.Push(widget->GetTransformMatrix());
            widget->Draw(stack);
            stack.Pop();
        }

        glFlush();

        // reload projection matrix
        glMatrixMode(GL_PROJECTION);
        glLoadIdentity();
        glLoadMatrixf(savedProj);
    }

    void Engine::updateWidgets(double deltaTime) {
        // call UpdateAll on each widget to automatically update it and its children recursively
        for (const auto& widget: widgets) {
            widget->UpdateAll(deltaTime);
        }

        // cleanup widgets that need to be destroyed
        std::erase_if(widgets,
                      [](const std::shared_ptr<Widget> &widget) { return widget->IsDestroyed(); }
        );

        // sort widgets back-to-front, in case an Update implementation changed a Z-Index
        std::ranges::sort(widgets,
                          [](const std::shared_ptr<Widget> &a, const std::shared_ptr<Widget> &b) {
                              return a->ZIndex < b->ZIndex;
                          });
    }

    void Engine::updateActors(double deltaTime) {
        // first, make sure there's actually a pawn possessed at the moment
        // this is done during engine startup
        if (possessedPawn.expired()) {
            auto pawn = SpawnActor<world::DefaultPawn>();
            Possess(pawn);
        }

        for (const auto& actor : actors) {
            actor->Update(deltaTime);
            // update components
            for (const auto& component : actor->GetComponents()) {
                component->Update(deltaTime);
            }
        }
    }

    void Engine::renderWorld() const {
        if (possessedPawn.expired()) {
            return; // before first update
        }

        glEnable(GL_DEPTH_TEST);
        glEnable(GL_CULL_FACE);

        glMatrixMode(GL_MODELVIEW);
        glLoadIdentity();

        auto activeCamera = possessedPawn.lock()->GetActiveCamera();
        auto cameraPos = activeCamera->GetAbsolutePosition();
        auto cameraCenter = cameraPos + activeCamera->GetForwardVector();

        gluLookAt(cameraPos.x, cameraPos.y, 10,
                0, 0, 0,
                0.0f, 1.0f, 0.0f
            );

        // render all scene components
        for (const auto& actor : actors) {
            glPushMatrix();
            auto actorTransform = actor->GetTransformMatrix();
            glMultMatrixf(static_cast<const float *>(actorTransform));

            for (const auto& component : actor->GetComponents()) {
                if (auto sceneComponent = std::dynamic_pointer_cast<world::ActorSceneComponent>(component)) {
                    auto componentTransform = sceneComponent->GetTransformMatrix();

                    glPushMatrix();
                    glMultMatrixf(static_cast<const float *>(componentTransform));

                    sceneComponent->Render();

                    glPopMatrix();
                }
            }
            glPopMatrix();
        }

        glFlush();
    }
} // glengine
