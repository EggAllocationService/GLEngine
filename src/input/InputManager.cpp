#include "InputManager.h"
#include "Engine.h"

glengine::input::InputManager::InputManager(Engine* engineRef)
{
	engine_ = engineRef;
}

void glengine::input::InputManager::Update(double deltaTime)
{
	auto mouseManager = engine_->GetMouseManager();
	if (mouseHandler_ && mouseManager->GetMouseMode() == CAPTIVE) {
		mouseHandler_(mouseManager->GetMousePosition());
	}

	for (auto key : pressedKeys_) {
		if (axisHandlers.contains(key)) {
			auto& handler = axisHandlers[key];
			handler.second(handler.first * deltaTime);
		}
	}
}

void glengine::input::InputManager::AcceptKeyInput(int key)
{
	if (key >= 'A' && key <= 'Z') {
		// convert capitals to lowercase
		key = key + ('a' - 'A');
	}

	if (keyHandlers_.contains(key)) {
		keyHandlers_[key]();
	}
	else {
		pressedKeys_.insert(key);
	}
}

void glengine::input::InputManager::KeyReleased(int key)
{
	pressedKeys_.erase(key);
}

void glengine::input::InputManager::AddMouseAxis(std::function<void(float2 delta)> listener)
{
	mouseHandler_ = listener;
}

void glengine::input::InputManager::AddAction(int key, std::function<void()> listener)
{
	keyHandlers_[key] = listener;
}

void glengine::input::InputManager::AddAxis(int positiveKey, int negativeKey, std::function<void(float)> listener)
{
	axisHandlers[positiveKey] = { 1.0f, listener };
	axisHandlers[negativeKey] = { -1.0f, listener };
}

void glengine::input::InputManager::Reset()
{
	mouseHandler_ = nullptr;
	keyHandlers_.clear();
	axisHandlers.clear();
}