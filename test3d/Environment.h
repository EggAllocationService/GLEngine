//
// Created by Kyle Smith on 2026-06-03.
//

#ifndef GLENGINE_ENVIRONMENT_H
#define GLENGINE_ENVIRONMENT_H
#include "3d/Actor.h"


class Environment : public glengine::world::Actor {
public:
    Environment ();
    void Update(double deltaTime) override;
};


#endif //GLENGINE_ENVIRONMENT_H
