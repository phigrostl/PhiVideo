#pragma once

#include "PhiVideo/Base/Maths.h"

namespace PhiVideo {
    struct Particles {
        Particles() = default;
        Particles(float w, float h, int parNum);

        std::vector<Vec3> pars;

        static void SetSeed(float seed);
    };
}
