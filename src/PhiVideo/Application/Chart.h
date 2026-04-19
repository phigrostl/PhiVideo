#pragma once

#include "PhiVideo/Application/JudgeLine.h"

namespace PhiVideo {

    struct ChartInfo {
        ChartInfo() = default;

        std::string level;
        std::string name;
        std::string song;
        std::string picture;
        std::string chart;
        std::string composer;
        std::string illustrator;
        std::string charter;
    };

    struct ChartData {
        ChartData() = default;

        std::vector<JudgeLine> judgeLines;
        std::vector<HitFx> clickEffectCollection;
        std::vector<HitFx> clickCollection;
        int noteCount = 0;
        float time = 0.0f;
        float offset = 0.0f;
        bool sameBPM = true;
    };

    struct Chart {
        Chart() = default;

        ChartInfo info;
        Texture* image = nullptr;
        Texture* imageBlur = nullptr;
        ChartData data;
    };

}
