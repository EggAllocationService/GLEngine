#include "Engine.h"
//
// Created by Kyle Smith on 2025-09-26.
//
#include "engine_GLUT.h"
#include "Colors.h"
#include <map>
#include "unistd.h"
#include <algorithm>

#include "GLMath.h"
#include "3d/ActorSceneComponent.h"
#include "3d/DefaultPawn.h"
#include "pipeline/gl/GLRenderer.h"

namespace glengine {

#pragma region event handlers
    /// These event handlers are needed because we can't pass state-owning functions (i.e. lambdas) to C as
    /// raw function pointers. These will find the Engine instance that owns the active window ID and
    /// call the appropriate instance function

    static void reshapeExec(GLFWwindow* window, int x, int y) {
        Engine* engine = (Engine*)glfwGetWindowUserPointer(window);
        engine->SetWindowSize(int2(x, y));
    }

    static void mouseMoveExec(GLFWwindow* window, double x, double y) {
        auto engine = (Engine*)glfwGetWindowUserPointer(window);
        engine->GetMouseManager()->HandleMotion(float2(x, y));
    }

    static void keyExec(GLFWwindow* window, int key, int scancode, int action, int flags) {
        auto engine = (Engine*)glfwGetWindowUserPointer(window);

        if (action == GLFW_PRESS || action == GLFW_REPEAT) {
            engine->KeyPressed(key);
        } else {
            engine->KeyReleased(key);
        }
    }

#pragma endregion

    Engine::Engine(const std::string &windowName, int2 size) {
        windowSize = size;
        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
        window = glfwCreateWindow(windowSize.x, windowSize.y, windowName.c_str(), nullptr, nullptr);
        mouseManager = new input::MouseManager(this);
        inputManager = new input::InputManager(this);
        pawnInputManager = new input::InputManager(this);
        renderer = new pipeline::wgpu::WGPURenderer(window);
        resourceManager = new ResourceManager(renderer);

        glfwSetWindowUserPointer(window, this);

        //glutIdleFunc(updateExec);
        //glutDisplayFunc(renderExec);
        glfwSetFramebufferSizeCallback(window, reshapeExec);
        //glutMouseFunc(clickExec);

        glfwSetCursorPosCallback(window, mouseMoveExec);
        //glutPassiveMotionFunc(mouseMoveExec);
        //glutMotionFunc(mouseMoveExec);
        glfwSetKeyCallback(window, keyExec);

        setLastUpdate();

        // this has to be initialized at the end as its constructor requires an opengl context
        renderObjectManager = new rendering::RenderObjects();
    }

    Engine::~Engine() {
        // destroy owned objects
        delete mouseManager;
        delete inputManager;
        delete pawnInputManager;
        delete resourceManager;
        delete renderObjectManager;
        delete renderer;
    }

    void Engine::MainLoop() {
        SetWindowSize(windowSize);
        while (!glfwWindowShouldClose(window)) {
            glfwPollEvents();

            Update();

            Render();
        }
    }

    void Engine::Update() {
        // main update loop
        flags.didUpdate = true;

        // track Update time
        auto start = std::chrono::steady_clock::now();

        // if game logic requested a quit on the previous frame, then do that now
        if (flags.quitRequested) {
            exit(0);
        }

        double delta = calculateDeltaTime();

        inputManager->Update(delta);
        pawnInputManager->Update(delta);

        updateActors(delta);

        setLastUpdate();

        auto end = std::chrono::steady_clock::now();
        auto timeMs = std::chrono::duration_cast<std::chrono::duration<double, std::milli>>(end - start);

        lastUpdateTime = timeMs.count();

    }

    void Engine::KeyPressed(int keyCode) {

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

    void Engine::SetMouseMode(const input::MouseMode mode) {

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
        renderer->Resize(size);
    }

    void Engine::Render() {
        // dont' render before at least one update call
        if (!flags.didUpdate) return;

        // perform all rendering then swap the double-buffered view
        // clearBuffers();

        // track Render time
        auto start = std::chrono::steady_clock::now();

        // render 3d world
        renderWorld();


        // update last render time variable
        auto end = std::chrono::steady_clock::now();
        auto timeMs = std::chrono::duration_cast<std::chrono::duration<double, std::milli>>(end - start);
        lastRenderTime = timeMs.count();
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


        auto viewTarget = possessedPawn.lock();
        auto viewCamera = viewTarget->GetActiveCamera();
        auto cameraTransformMatrix = viewTarget->GetTransformMatrix() * viewCamera->GetTransformMatrix();
        auto viewMatrix = math::viewMatrix(cameraTransformMatrix);

        // first load the camera's projection matrix
        auto projectionMatrix = viewCamera->GetProjectionMatrix();

        pipeline::wgpu::RenderUniforms uniforms = {
            .projectionViewMatrix = projectionMatrix * viewMatrix,
            .projectionMatrix = projectionMatrix,
            .viewMatrix = viewMatrix,
            .lightCount = 0
        };
        // set matrices
        auto bundle = renderer->BeginRendering(uniforms);
        if (!bundle.valid) {
            return;
        }

        auto stack = MatrixStack();

        // render all scene components
        for (const auto& actor : actors) {
            // check if actor has any renderable components
            // skip if nothing to do, so we don't waste time matrix multiplying
            if (!actor->GetComponent<world::ActorSceneComponent>()) {
                continue;
            }

            // push actor transform matrix
            stack.Push(actor->GetTransformMatrix());

            // render actor scene components
            for (const auto& component : actor->GetComponents()) {
                if (auto sceneComponent = std::dynamic_pointer_cast<world::ActorSceneComponent>(component)) {
                    // push component transform matrix
                    stack.Push(sceneComponent->GetTransformMatrix());

                    sceneComponent->Render(bundle, stack);

                    stack.Pop();
                }
            }
            stack.Pop();
        }

       renderer->FinishRendering(bundle);
    }
} // glengine
