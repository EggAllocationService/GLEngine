//
// Created by Kyle Smith on 2025-10-27.
//

#include "3d/Actor.h"

namespace glengine::world {
    thread_local Engine* CURRENT_ENGINE_CONSTRUCTING = nullptr;
}