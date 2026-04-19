#include "HitFx.h"

namespace PhiVideo {

    int randNum = 0;

    Particles::Particles(float w, float h, int parNum) {
        pars.clear();
        for (int i = 0; i < parNum; i++) {
            float rotate = randf(0.0f, 360.0f, randNum++);
            float size = 33.0f * 0.75f;
            float r = randf(185.0f, 265.0f, randNum++);
            pars.push_back(Vec3(rotate, size, r));
        }
    }

    void Particles::SetSeed(float seed) {
        randNum = (int)(seed * RAND_NUM);
    }

}
