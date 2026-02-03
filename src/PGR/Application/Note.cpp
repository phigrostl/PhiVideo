#include "Note.h"

namespace PGR {

    Vec2 Note::getclickEffect(float w, float h, EventsValue ev) {
        Vec2 pos = rotatePoint(
            ev.x * w, ev.y * h,
            positionX * PGRW * w,
            ev.rotate
        );
        return pos;
    }

	NoteImgs::NoteImgs()
		: click(new Texture("Notes\\Tap.png")),
		drag(new Texture("Notes\\Drag.png")),
		hold(new Texture("Notes\\Hold.png")),
		flick(new Texture("Notes\\Flick.png")),
		clickMH(new Texture("Notes\\TapMH.png")),
		dragMH(new Texture("Notes\\DragMH.png")),
		holdMH(new Texture("Notes\\HoldMH.png")),
		flickMH(new Texture("Notes\\FlickMH.png")),
		hitFx(new Texture("Notes\\HitFx.png")) {}

}
