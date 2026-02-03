#pragma once

#include "PGR/Base/Maths.h"

#include <vector>

namespace PGR {

    struct JudgeLineMoveEvent {
        JudgeLineMoveEvent() = default;

        float startTime, endTime;
        float start, end;
        float start2, end2;
    };

    struct JudgeLineRotateEvent {
        JudgeLineRotateEvent() = default;

        float startTime, endTime;
        float start, end;
    };

    struct JudgeLineDisappearEvent {
        JudgeLineDisappearEvent() = default;

        float startTime, endTime;
        float start, end;
    };

    struct SpeedEvent {
        SpeedEvent() = default;

        float startTime, endTime;
        float value;
        float floorPosition;
    };

    struct EventsValue {
        EventsValue() = default;

        float rotate = 0.0f;
        float x = 0.0f;
        float y = 0.0f;
        float alpha = 1.0f;
        float speed = 0.0f;
    };

    struct SpeedEventLine {
        SpeedEventLine() = default;

        size_t line;
        float stime;
        float durtime;
        float v;
    };

    template<typename T>
    int findEvent(float t, const std::vector<T>& es) {
        if (es.empty()) return -1;

        size_t left = 0;
        size_t right = es.size() - 1;
        int result = -1;

        if (t < es[0].startTime || t > es[right].endTime) {
            return -1;
        }
        while (left <= right) {
            size_t mid = left + (right - left) / 2;
            const T& e = es[mid];

            if (e.startTime <= t && t <= e.endTime) {
                result = static_cast<int>(mid);
                left = mid + 1;
            }
            else if (t < e.startTime)
                right = mid - 1;
            else
                left = mid + 1;
        }

        return result;
    }

    template<typename T>
    float getEventValue(float t, std::vector<T> es) {
        const int i = findEvent(t, es);
        if (i == -1)
            return 0.0f;

        const T& e = es[i];

        return linear(t, e.startTime, e.endTime, e.start, e.end);
    }

    float getPosYEvent(float t, std::vector<JudgeLineMoveEvent> es);
    float getSpeedValue(float t, std::vector<SpeedEvent> es);

}
