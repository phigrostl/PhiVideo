#include "HitFx.h"

namespace PGR {

    Particles::Particles(float w, float h) {
        for (int i = 0; i < 4; i++) {
            float rotate = randf(0.0f, 360.0f);
            float size = 33.0f * 0.75f;
            float r = randf(185.0f, 265.0f);
            pars[i] = Vec3(rotate, size, r);
        }
    }

}
