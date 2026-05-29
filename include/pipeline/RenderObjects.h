#pragma once

#include <optional>

#include "Vectors.h"

#include "glengine_export.h"

#include <vector>

#include "RenderObject.h"

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
		RenderObjects(pipeline::wgpu::WGPURenderer* renderer);

		template<typename T>
		std::shared_ptr<T> Create() {
			static_assert(std::is_base_of_v<pipeline::RenderObject, T>, "T must be a RenderObject");
			pipeline::RenderObject::CURRENT_RENDERER = renderer;
			auto inst = std::make_shared<T>();
			objects.push_back(inst);
			return inst;
		}

		template <typename T>
		std::shared_ptr<T> GetObject() {
			static_assert(std::is_base_of_v<pipeline::RenderObject, T>, "T must be a RenderObject");
			for (const auto &component : objects) {
				if (auto result= std::dynamic_pointer_cast<T>(component)) {
					return result;
				}
			}
			return nullptr;
		}

		std::vector<std::shared_ptr<pipeline::RenderObject>> objects;
	private:
		pipeline::wgpu::WGPURenderer* renderer;
	};
}
