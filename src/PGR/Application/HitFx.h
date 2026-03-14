#pragma once

#include "PGR/Base/Maths.h"

namespace PGR {
    struct Particles {
        Particles() = default;
        Particles(float w, float h);

        Vec3 pars[4];
    };
}
