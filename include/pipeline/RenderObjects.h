#pragma once

#include <optional>

#include "Vectors.h"

#include "glengine_export.h"

#include <vector>

namespace glengine::rendering {

	struct GLENGINE_EXPORT LightInfo {
		float4 position;
		float4 diffuse;
		float4 specular;
		float4 ambient;
	};

	struct GLENGINE_EXPORT FogSettings {
		float4 color;
		float density;
	};
	
	/// <summary>
	/// RenderObjects is a container to track objects that require special handling during rendering, for example lights.
	/// The intent is that Light components (or similar) can submit information to RenderObjects during the Update phase, and then
	/// dedicated code paths in the engine's rendering functions will setup the appropriate state prior to rendering any scene components. 
	/// </summary>
	class GLENGINE_EXPORT RenderObjects {
	public:
		RenderObjects();

		/// Submits a light for rendering next frame
		void PushLight(LightInfo l);

		void SetFog(FogSettings a) {
			fog_ = a;
		}

		/// Sets up all gl light objects
		void InitLights() const;

		/// Sets up fog
		void InitFog() const;

		/// Disables all objects
		void DeInit() const;

		/// Removes all entries from previous frame
		void Reset();
	private:
		std::vector<LightInfo> lights_;
		std::optional<FogSettings> fog_;

	};
}