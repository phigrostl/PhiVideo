#pragma once

#include "PhiVideo/Base/Maths.h"

namespace PhiVideo {

    struct BlockAreaRotateEvent {
        BlockAreaRotateEvent() = default;

        Vec2 anchor;
        float time;
        int easeType;
        int rotation;
    };

    struct BlockAreaMoveEvent {
        BlockAreaMoveEvent() = default;

        Vec2 endPosition;
        float time;
        int easeTypeX;
        int easeTypeY;
    };

    struct BlockAreaScaleEvent {
        BlockAreaScaleEvent() = default;

        Vec2 anchor;
        float time;
        int easeTypeX;
        int easeTypeY;
        Vec2 scale;
    };

    struct BlockAreaEvent {
        BlockAreaEvent() = default;

        Vec2 topRightPercentage;
        Vec2 bottomLeftPercentage;
        float appearTime;
        float disappearTime;
        float enableTime;
        float disableTime;
        bool isSubtract;
        std::vector<BlockAreaRotateEvent> rotateEvents;
        std::vector<BlockAreaMoveEvent> moveEvents;
        std::vector<BlockAreaScaleEvent> scaleEvents;
    };

    struct BlockArea {
        BlockArea() = default;

        Vec2 topRightPercentage;
        Vec2 bottomLeftPercentage;
        bool isEnabled;
        float rotation;
        Vec2 rotationAnchor;
        Vec2 position;
        Vec2 scale;
        Vec2 scaleAnchor;
    };

}
