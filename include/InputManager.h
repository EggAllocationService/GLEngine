#pragma once
#include <map>
#include <string>
#include <functional>
#include <unordered_set>
#include "engine_GLUT.h"
#include "Vectors.h"
#include "glengine_export.h"

// define our own special keys which are offset so we can use the same input functions for all
#define KEY_ARROW_RIGHT GLUT_KEY_RIGHT << 8
#define KEY_ARROW_LEFT GLUT_KEY_LEFT << 8
#define KEY_ARROW_UP GLUT_KEY_UP << 8
#define KEY_ARROW_DOWN GLUT_KEY_DOWN << 8

#define KEY_PAGE_UP GLUT_KEY_PAGE_UP << 8
#define KEY_PAGE_DOWN GLUT_KEY_PAGE_DOWN << 8



namespace glengine {
	class Engine;
}

namespace glengine::input {
	class GLENGINE_EXPORT InputManager {
	public:
		InputManager(Engine* engineRef);

		void Update(double deltaTime);

		/// <summary>
		/// Call when a key has been pressed
		/// </summary>
		void AcceptKeyInput(int key);

		/// <summary>
		/// Call when a key has been released
		/// </summary>
		/// <param name="key"></param>
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
		
		/// <summary>
		/// Adds a keyboard input axis. One key represents a positive input, and one represents a negative input
		/// 
		/// The specified callback will be called with a float representing the current value of this axis, multiplied by deltaTime
		/// </summary>
		/// <param name="positiveKey">the character or key code constant for the positive axis</param>
		/// <param name="negativeKey">the character or key code constant for the negative axis</param>
		/// <param name="listener">the callback to run whenever there's input on this axis</param>
		void AddAxis(int positiveKey, int negativeKey, std::function<void(float)> listener);
		
		/// <summary>
		/// Clears all input mappings
		/// </summary>
		void Reset();

	private:
		std::function<void(float2 value)> mouseHandler_;
		std::map<unsigned int, std::function<void()>> keyHandlers_;
		std::map<unsigned int, std::pair<float, std::function<void(float)>>> axisHandlers;
		std::unordered_set<int> pressedKeys_;
		Engine* engine_;
	};
}