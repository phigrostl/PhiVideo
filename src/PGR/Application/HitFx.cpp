#include "HitFx.h"

namespace PGR {

    struct Seeds {
        Seeds() = default;

        std::vector<float> seeds;
        int num = 1;
    };

    std::vector<Seeds> seedses;

    Particles::Particles(float w, float h, std::vector<float> seeds) {
        bool isRepeat = false;
        int num = 0;
        for (int i = 0; i < seedses.size(); i++) {
            if (seedses[i].seeds == seeds) {
                isRepeat = true;
                seedses[i].num++;
                num = seedses[i].num;
                break;
            }
        }

        for (int i = 0; i < 4; i++) {
            std::vector<float> seedsa = getSeeds(seeds, i + num * 4);
            float rotate = randf(randf(seedsa, 0.0f, 1007.0f), 0.0f, 360.0f);
            float size = 33.0f * 0.75f;
            float r = randf(randf(seedsa, 0.0f, 10007.0f), 185.0f, 265.0f);
            pars[i] = Vec3(rotate, size, r);
        }

        if (!isRepeat) {
            Seeds s;
            s.seeds = seeds;
            s.num = 0;
            seedses.push_back(s);
        }
    }

}
