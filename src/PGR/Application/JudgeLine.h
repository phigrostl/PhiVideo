#pragma once

#include "PGR/Application/Note.h"

#include <vector>

namespace PGR {

    struct JudgeLine {
        JudgeLine() = default;

        float bpm = 0.0f;
        std::vector<JudgeLineMoveEvent> moveEvents;
        std::vector<JudgeLineRotateEvent> rotateEvents;
        std::vector<JudgeLineDisappearEvent> disappearEvents;
        std::vector<SpeedEvent> speedEvents;

        std::vector<Note> notes;

        float sec2beat(float t, float offset) {
            return (t - offset) / (PGRBEAT / bpm);
        }

        float beat2sec(float t, float offset) {
            return t * (PGRBEAT / bpm) + offset;
        }

        void initSpeedEvents() {
            float fp = 0.0f;

            for (auto& e : speedEvents) {
                e.floorPosition = fp;
                fp += (e.endTime - e.startTime) * e.value;
            }
        }

        float getFp(float t) {
            int i = findEvent(t, speedEvents);
            if (i == -1)
                return 0.0f;

            SpeedEvent e = speedEvents[i];
            return e.floorPosition + (t - e.startTime) * e.value;
        }

        void initNoteFp() {
            for (auto& n : notes)
                n.floorPosition = getFp(n.time);
        }

        EventsValue getState(float t, float offset) {
            float beatt = sec2beat(t, offset);
            if (t < 0.0f) beatt = -t;
            float rotate = getEventValue(beatt, rotateEvents);
            float x = getEventValue(beatt, moveEvents);
            float y = getPosYEvent(beatt, moveEvents);
            float alpha = getEventValue(beatt, disappearEvents);
            float speed = getSpeedValue(beatt, speedEvents);
            return { rotate, x, y, alpha, speed };
        }

    };

}
