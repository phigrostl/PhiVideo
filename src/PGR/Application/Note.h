#pragma once

#include "PGR/Base/Maths.h"
#include "PGR/Application/Constant.h"
#include "PGR/Application/Event.h"
#include "PGR/Application/HitFx.h"
#include "PGR/Render/Texture.h"

namespace PGR {

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

    struct NoteMap {
        NoteMap() = default;

        Note note;
        float time;
        Particles particles;
    };
}
