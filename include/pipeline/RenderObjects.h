#pragma once

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
	
	/// <summary>
	/// RenderObjects is a container to track objects that require special handling during rendering, for example lights
	/// The intent is that Light components (or similar) can submit information to RenderObjects during the Update phase, and then
	/// dedicated code paths in the engine's rendering functions will setup the appropriate state prior to rendering any scene components. 
	/// </summary>
	class GLENGINE_EXPORT RenderObjects {
	public:
		RenderObjects();

		void PushLight(LightInfo l);

		void InitLights() const;

		void Reset();
	private:
		std::vector<LightInfo> lights_;

	};
}