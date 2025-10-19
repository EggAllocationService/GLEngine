#pragma once
#include <map>
#include <string>
#include <functional>
#include <unordered_set>
#include "engine_GLUT.h"
#include "Vectors.h"
#include "glengine_export.h"

namespace glengine {
	class Engine;
}

namespace glengine::input {
	class GLENGINE_EXPORT InputManager {
	public:
		InputManager(Engine* engineRef);

		void Update(double deltaTime);

		void AcceptKeyInput(int key);

		void KeyReleased(int key);

		/// <summary>
		/// Adds an input handler for mouse movement
		/// </summary>
		/// <param name="listener">The function to call with the mouse delta x/y movement</param>
		void AddMouseAxis(std::function<void(float2 delta)> listener);

		/// <summary>
		/// Adds an input handler for a key action
		/// </summary>
		/// <param name="key">Either a char for a literal key, or one of the GLUT special constants</param>
		/// <param name="listener"The function to call when the key is pressed></param>
		void AddAction(int key, std::function<void()> listener);
		
		void AddAxis(int positiveKey, int negativeKey, std::function<void(float)> listener);
		
		void Reset();
	private:
		std::function<void(float2 value)> mouseHandler_;
		std::map<unsigned int, std::function<void()>> keyHandlers_;
		std::map<unsigned int, std::pair<float, std::function<void(float)>>> axisHandlers;
		std::unordered_set<int> pressedKeys_;
		Engine* engine_;
	};
}