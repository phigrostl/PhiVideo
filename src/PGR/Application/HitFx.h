#pragma once

#include "PGR/Base/Maths.h"
#include "PGR/Application/Event.h"

namespace PGR {
    struct Particles {
        Particles() = default;
        Particles(float w, float h, std::vector<float> seeds);

        Vec3 pars[4];
    };
}
