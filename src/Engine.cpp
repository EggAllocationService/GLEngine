#include "Engine.h"
//
// Created by Kyle Smith on 2025-09-26.
//
#include "engine_GLUT.h"
#include "Colors.h"
#include <map>
#include <algorithm>

#include "GLMath.h"
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

    static void specialExec(int keycode, int, int) {
        int currentWindow = glutGetWindow();
        if (Instances.contains(currentWindow)) {
            Instances[currentWindow]->KeyPressed(keycode << 8);
        }
    }

    static void specialUpExec(int keycode, int, int) {
        int currentWindow = glutGetWindow();
        if (Instances.contains(currentWindow)) {
            Instances[currentWindow]->KeyReleased(keycode << 8);
        }
    }

    static void keyExec(unsigned char keycode, int, int) {
        int currentWindow = glutGetWindow();
        if (Instances.contains(currentWindow)) {
            Instances[currentWindow]->KeyPressed(keycode);
        }
    }

    static void keyUpExec(unsigned char keycode, int, int) {
        int currentWindow = glutGetWindow();
        if (Instances.contains(currentWindow)) {
            Instances[currentWindow]->KeyReleased(keycode);
        }
    }

#pragma endregion

    Engine::Engine(const std::string &windowName, int2 size) {
        mouseManager = new input::MouseManager(this);
        inputManager = new input::InputManager(this);
        pawnInputManager = new input::InputManager(this);
        resourceManager = new ResourceManager();
        
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
        glutKeyboardFunc(keyExec);
        glutSpecialFunc(specialExec);
        glutKeyboardUpFunc(keyUpExec);
        glutSpecialUpFunc(specialUpExec);

        setLastUpdate();

        console = AddOnscreenWidget<console::Console>();

        addDefaultCommands();

        // this has to be initialized at the end as its constructor requires an opengl context
        renderObjectManager = new rendering::RenderObjects();
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
        delete inputManager;
        delete pawnInputManager;
        delete resourceManager;
        delete renderObjectManager;
    }

    void Engine::Update() {
        // main update loop

        // track Update time
        auto start = std::chrono::steady_clock::now();

        // if game logic requested a quit on the previous frame, then do that now
        if (flags.quitRequested) {
            glutDestroyWindow(windowHandle);
            exit(0);
        }

        double delta = calculateDeltaTime();

        mouseManager->Update();
        inputManager->Update(delta);
        pawnInputManager->Update(delta);

        renderObjectManager->Reset();

        updateWidgets(delta);

        updateActors(delta);

        setLastUpdate();

        glutPostRedisplay();

        auto end = std::chrono::steady_clock::now();
        auto timeMs = std::chrono::duration_cast<std::chrono::duration<double, std::milli>>(end - start);

        lastUpdateTime = timeMs.count();

    }

    void Engine::KeyPressed(int keyCode) {
        // first priority: send input to the focused widget if there is one
        if (!focusedWidget.expired()) {
            focusedWidget.lock()->KeyPressed(keyCode);
            return; // focused widgets override all other handlers
        }

        // second priority: send input to possessed pawn if mouse is captured or we're allowing non-captured input
        if (mouseManager->GetMouseMode() == input::CAPTIVE || flags.allowNonFocusedPawnInput) {
            if (pawnInputManager->AcceptKeyInput(keyCode)) {
                // input was consumed by the pawn, so don't forward to global handlers
                return;
            }
        }

        // finally, send input to global handlers
        inputManager->AcceptKeyInput(keyCode);
    }

    void Engine::KeyReleased(int keyCode) {
        pawnInputManager->KeyReleased(keyCode);
        inputManager->KeyReleased(keyCode);
    }

    void Engine::FocusWidget(std::shared_ptr<Widget> widget) {
        if (mouseManager->GetMouseMode() == input::CAPTIVE) {
            return; // can't focus widgets while the mouse is captured
        }

        if (!focusedWidget.expired()) {
            focusedWidget.lock()->FocusStateChanged(false);
        }
        focusedWidget = widget;

        if (widget != nullptr) {
            widget->FocusStateChanged(true);
        }
    }

    void Engine::SetMouseMode(const input::MouseMode mode) {
        if (mode == input::CAPTIVE && !focusedWidget.expired()) {
            focusedWidget.lock()->FocusStateChanged(false);
            focusedWidget = std::weak_ptr<Widget>();
        }

        mouseManager->SetMouseMode(mode);
    }

    void Engine::Possess(const std::shared_ptr<world::Pawn>& target) {
        // clear existing input bindings
        pawnInputManager->Reset();

        // if we're already possessing a pawn, call UnPossess
        if (const auto realPossessed = possessedPawn.lock()) {
            realPossessed->OnUnpossess();
        }

        // update stored pawn and call OnPossess
        possessedPawn = target;
        target->OnPossess(pawnInputManager);
    }

    void Engine::SetWindowSize(int2 size) {
        // update stored size and viewport
        windowSize = size;
        glViewport(0, 0, windowSize.x, windowSize.y);
    }

    void Engine::Render() {
        // perform all rendering then swap the double-buffered view
        clearBuffers();

        // track Render time
        auto start = std::chrono::steady_clock::now();

        // render 3d world
        renderWorld();

        // teardown render objects so we don't screw up widgets
        renderObjectManager->DeInit();

        // render UI
        renderWidgets();

        // update last render time variable
        auto end = std::chrono::steady_clock::now();
        auto timeMs = std::chrono::duration_cast<std::chrono::duration<double, std::milli>>(end - start);
        lastRenderTime = timeMs.count();

        glutSwapBuffers();
    }

    double Engine::calculateDeltaTime() const {
        auto now = std::chrono::steady_clock::now();
        auto time = std::chrono::duration_cast<std::chrono::duration<double>>(now - lastUpdate);

        return time.count();
    }

    void Engine::clearBuffers() {
        glClearColor(0, 0, 0, 1);
        glClearDepth(0.0f); // using LH coordinate system, so 0 depth = far plane
        glDepthFunc(GL_GEQUAL);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
    }

    void Engine::setLastUpdate() {
        lastUpdate = std::chrono::steady_clock::now();
    }

    void Engine::renderWidgets() {
        // reset opengl matrices
        glMatrixMode(GL_PROJECTION);
        glLoadIdentity();

        glMatrixMode(GL_MODELVIEW);
        glLoadIdentity();
        glDisable(GL_DEPTH_TEST);
        glDisable(GL_CULL_FACE);

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

        // for each actor
        for (const auto& actor : actors) {
            // update actor
            actor->Update(deltaTime);
            
            // update components
            for (const auto& component : actor->GetComponents()) {
                component->Update(deltaTime);
            }
        }

        // cleanup dead actors
        std::erase_if(actors,
            [](const std::shared_ptr<world::Actor>& actor) { return actor->IsDestroyed(); }
        );
    }

    void Engine::renderWorld() const {
        if (possessedPawn.expired()) {
            return; // before first update
        }

        glEnable(GL_DEPTH_TEST);
        glFrontFace(GL_CW);

        auto viewTarget = possessedPawn.lock();
        auto viewCamera = viewTarget->GetActiveCamera();
        auto cameraTransformMatrix = viewTarget->GetTransformMatrix() * viewCamera->GetTransformMatrix();
        auto viewMatrix = math::viewMatrix(cameraTransformMatrix);

        // first load the camera's projection matrix
        viewCamera->SetProjectionMatrix();

        // next load the view matrix
        glMatrixMode(GL_MODELVIEW);
        glLoadIdentity();
        glLoadMatrixf(static_cast<const float *>(viewMatrix));

        // setup lights
        renderObjectManager->InitLights();

        // setup fog
        renderObjectManager->InitFog();

        // render all scene components
        for (const auto& actor : actors) {
            // check if actor has any renderable components
            // skip if nothing to do, so we don't waste time matrix multiplying
            if (!actor->GetComponent<world::ActorSceneComponent>()) {
                continue;
            }

            // push actor transform matrix
            glPushMatrix();
            auto actorTransform = actor->GetTransformMatrix();
            glMultMatrixf(static_cast<const float *>(actorTransform));

            // render actor scene components
            for (const auto& component : actor->GetComponents()) {
                if (auto sceneComponent = std::dynamic_pointer_cast<world::ActorSceneComponent>(component)) {
                    // push component transform matrix
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

    void Engine::addDefaultCommands() {
        console->AddConsoleCommand("quit", [this](std::string_view) {
           this->Quit();
        });

        console->AddConsoleCommand("detach", [this](std::string_view) {
            const auto newPawn = this->SpawnActor<world::DefaultPawn>();
            const auto currentPawn = this->GetPossessedPawn();
            newPawn->GetTransform()->SetPosition(currentPawn->GetTransform()->GetPosition());

            this->Possess(newPawn);
        });

        // Setup console keybind
        this->GetInputManager()->AddAction('`', [this] {
            this->ShowConsole();
        });
    }
} // glengine
