#pragma once

#include "PhiVideo/Application/Constant.h"
#include "PhiVideo/Application/Event.h"
#include "PhiVideo/Application/HitFx.h"
#include "PhiVideo/Base/Maths.h"
#include "PhiVideo/Render/Texture.h"

namespace PhiVideo {

    struct Note {
        Note() = default;

        int type = 0;
        float time = 0.0f;
        float holdTime = 0.0f;
        float positionX = 0.0f;
        float speed = 1.0f;
        bool isAbove = true;

        float secTime = 0.0f;
        float secHoldTime = 0.0f;
        float secHoldEndTime = 0.0f;
        float holdLength = 0.0f;
        float floorPosition = 0.0f;

        bool isHold = false;
        bool morebets = false;

        int line = 0;

        Vec2 getclickEffect(float w, float h, EventsValue ev);

    };

    struct NoteImgs {
        NoteImgs();

        Texture* click;
        Texture* drag;
        Texture* hold;
        Texture* flick;

        Texture* holdBody = nullptr;
        Texture* holdHead = nullptr;
        Texture* holdTail = nullptr;

        Texture* clickMH;
        Texture* dragMH;
        Texture* holdMH;
        Texture* flickMH;

        Texture* holdMHBody = nullptr;
        Texture* holdMHHead = nullptr;
        Texture* holdMHTail = nullptr;

        Texture* hitFx;

    };

    struct HitFx {
        HitFx() = default;

        float time;
        Note note;
        Particles particles;
    };
}
