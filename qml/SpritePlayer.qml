import QtQuick

Item {
    id: root

    property bool mirrored: false

    function play(sheetUrl, frames, duration, looping) {
        sprite.stop()
        sprite.source = sheetUrl
        sprite.frameCount = frames
        sprite.frameDuration = duration
        sprite.loops = looping ? AnimatedSprite.Infinite : 1
        sprite.finishBehavior = AnimatedSprite.FinishAtFinalFrame
        sprite.restart()
    }

    AnimatedSprite {
        id: sprite
        anchors.fill: parent
        running: false
        frameWidth: 640
        frameHeight: 640
        interpolate: false
        smooth: false

        transform: Scale {
            origin.x: sprite.width / 2
            origin.y: sprite.height / 2
            xScale: root.mirrored ? -1 : 1
            yScale: 1
        }
    }
}

