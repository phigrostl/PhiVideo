#include "Event.h"

namespace PGR {

    float getPosYEvent(float t, std::vector<JudgeLineMoveEvent> es) {
        const int i = findEvent(t, es);
        if (i == -1)
            return 0.0f;
        const JudgeLineMoveEvent& e = es[i];

        return linear(t, e.startTime, e.endTime, e.start2, e.end2);
    }

    float getSpeedValue(float t, std::vector<SpeedEvent> es) {
        const int i = findEvent(t, es);
        if (i == -1)
            return 0.0f;
        const SpeedEvent& e = es[i];

        return e.value;
    }

}
