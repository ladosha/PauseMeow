import QtQuick
import QtQuick.Controls
import QtMultimedia
import QtQuick.Window

Window {
    id: root

    required property var targetScreen
    property bool primaryOverlay: false
    property bool shouldShow: appController.phase === "entering"
        || appController.phase === "rest"
        || appController.phase === "leaving"
    property string responseMode: ""

    MediaPlayer {
        id: walkingSound
        source: "qrc:/assets/audio/cat_steps.ogg"
        loops: MediaPlayer.Infinite
        audioOutput: AudioOutput { volume: 0.50 }
    }

    MediaPlayer {
        id: walkingPurrSound
        source: "qrc:/assets/audio/cat_purr.ogg"
        loops: MediaPlayer.Infinite
        audioOutput: AudioOutput { volume: 1.0 }
    }

    MediaPlayer {
        id: restPurrSound
        source: "qrc:/assets/audio/cat_purr.ogg"
        loops: MediaPlayer.Infinite
        audioOutput: AudioOutput { volume: 1.0 }
    }

    MediaPlayer {
        id: meowSound
        source: "qrc:/assets/audio/cat_meow.ogg"
        audioOutput: AudioOutput { volume: 1.0 }
    }

    screen: targetScreen
    visibility: shouldShow ? Window.FullScreen : Window.Hidden
    flags: Qt.FramelessWindowHint | Qt.WindowStaysOnTopHint | Qt.Tool
    color: "transparent"

    onShouldShowChanged: {
        if (shouldShow) {
            raise()
            requestActivate()
        }
    }

    function centerCat() {
        cat.x = root.width / 2 - cat.width / 2
        cat.y = root.height * 0.48 - cat.height / 2
    }

    function playSleep() {
        responseMode = ""
        speechBubble.opacity = 0
        cat.mirrored = false
        cat.play("qrc:/assets/sprites/cat_sleep.png", 6, 350, true)
    }

    function startWalkingAudio() {
        if (appController.walkingSounds)
            walkingSound.play()
        if (appController.walkingPurr)
            walkingPurrSound.play()
    }

    function stopWalkingAudio() {
        walkingSound.stop()
        walkingPurrSound.stop()
    }

    function startRestAudio() {
        stopWalkingAudio()
        if (appController.restPurr)
            restPurrSound.play()
    }

    function stopAllAudio() {
        stopWalkingAudio()
        restPurrSound.stop()
    }

    function stopResponses() {
        pokeTimer.stop()
        hissTimer.stop()
        settleTimer.stop()
        responseMode = ""
        speechBubble.opacity = 0
    }

    function startEntering() {
        if (!primaryOverlay)
            return
        stopResponses()
        stopAllAudio()
        leaveSequence.stop()
        centerCat()
        cat.x = -cat.width - 40
        cat.mirrored = false
        enterSequence.restart()
    }

    function startLeaving() {
        if (!primaryOverlay)
            return
        stopResponses()
        stopAllAudio()
        enterSequence.stop()
        leaveSequence.restart()
    }

    function handlePhase() {
        if (!primaryOverlay)
            return

        if (appController.phase === "entering") {
            Qt.callLater(startEntering)
        } else if (appController.phase === "rest") {
            enterSequence.stop()
            centerCat()
            playSleep()
            startRestAudio()
        } else if (appController.phase === "leaving") {
            Qt.callLater(startLeaving)
        } else {
            enterSequence.stop()
            leaveSequence.stop()
            stopResponses()
            stopAllAudio()
        }
    }

    Rectangle {
        anchors.fill: parent
        color: appController.dimBackground ? "#E0181721" : "transparent"

        Behavior on color {
            ColorAnimation { duration: 300 }
        }
    }

    Item {
        id: focusArea
        visible: primaryOverlay
        width: Math.min(root.width * 0.72, 1040)
        height: Math.min(root.height * 0.72, 760)
        anchors.centerIn: parent
    }

    Text {
        visible: primaryOverlay
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.top: parent.top
        anchors.topMargin: Math.max(44, root.height * 0.08)
        text: appController.phase === "entering"
            ? appController.translations["overlay.entering.title"]
            : appController.phase === "leaving"
                ? appController.translations["overlay.leaving.title"]
                : appController.translations["overlay.rest.title"]
        color: "#FFF6EA"
        font.pixelSize: Math.max(24, Math.min(40, root.width / 35))
        font.weight: Font.DemiBold
    }

    Text {
        visible: primaryOverlay
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.top: parent.top
        anchors.topMargin: Math.max(90, root.height * 0.14)
        text: {
            if (appController.phase === "rest")
                return appController.translations["overlay.rest.subtitle"]
            if (appController.phase === "leaving")
                return appController.translations["overlay.leaving.subtitle"]
            return appController.translations["overlay.entering.subtitle"]
        }
        color: "#BEB2C3"
        font.pixelSize: Math.max(14, Math.min(20, root.width / 70))
    }

    SpritePlayer {
        id: cat
        visible: primaryOverlay
        width: Math.min(640, root.width * 0.46)
        height: width
        z: 2

        MouseArea {
            anchors.fill: parent
            enabled: root.primaryOverlay && appController.phase === "rest"
            z: 200
            onClicked: appController.registerCatClick()
        }
    }

    Rectangle {
        id: speechBubble
        parent: cat
        visible: primaryOverlay
        opacity: 0
        width: bubbleText.implicitWidth + 44
        height: 56
        radius: 20
        color: "#FFF7ED"
        x: cat.width / 2 - width / 2
        y: cat.height * 0.31 - height - 16
        z: 101

        Text {
            id: bubbleText
            anchors.centerIn: parent
            text: appController.translations["overlay.hiss"]
            color: "#3A303E"
            font.pixelSize: 16
            font.weight: Font.DemiBold
        }

        Behavior on opacity {
            NumberAnimation { duration: 180 }
        }
    }

    Rectangle {
        id: timerPanel
        parent: cat
        visible: primaryOverlay && appController.phase === "rest"
        width: Math.min(360, root.width * 0.4)
        height: 104
        radius: 30
        color: "#F4E9DC"
        x: cat.width / 2 - width / 2
        y: cat.height - height + 20
        z: 100

        Column {
            anchors.centerIn: parent
            spacing: 2

            Text {
                anchors.horizontalCenter: parent.horizontalCenter
                text: appController.remainingText
                color: "#2F2936"
                font.pixelSize: 42
                font.weight: Font.Bold
                font.family: "Consolas"
            }

            Text {
                anchors.horizontalCenter: parent.horizontalCenter
                text: appController.translations["overlay.untilWake"]
                color: "#7D7182"
                font.pixelSize: 13
            }
        }

    }

    Button {
        parent: focusArea
        visible: root.primaryOverlay
            && appController.phase === "rest"
            && !appController.strictMode
        anchors.right: parent.right
        anchors.top: parent.top
        anchors.margins: 28
        z: 10
        text: appController.translations["overlay.wake"]
        palette.buttonText: "#EADFEA"

        background: Rectangle {
            radius: 14
            color: parent.down ? "#55485C" : "#3C3342"
            border.color: "#66566D"
        }

        onClicked: appController.finishBreakEarly()
    }

    SequentialAnimation {
        id: enterSequence

        ScriptAction {
            script: {
                cat.play(
                    "qrc:/assets/sprites/cat_walk.png", 8, 90, true)
                root.startWalkingAudio()
            }
        }

        NumberAnimation {
            target: cat
            property: "x"
            to: root.width / 2 - cat.width / 2
            duration: 1650
            easing.type: Easing.OutCubic
        }

        ScriptAction {
            script: root.stopWalkingAudio()
        }

        ScriptAction {
            script: cat.play(
                "qrc:/assets/sprites/cat_lie_down.png", 12, 80, false)
        }

        PauseAnimation { duration: 980 }

        ScriptAction {
            script: appController.beginRest()
        }
    }

    SequentialAnimation {
        id: leaveSequence

        ScriptAction {
            script: {
                root.centerCat()
                cat.mirrored = false
                cat.play(
                    "qrc:/assets/sprites/cat_wake_yawn.png", 12, 150, false)
            }
        }

        PauseAnimation { duration: 1820 }

        ScriptAction {
            script: cat.play(
                "qrc:/assets/sprites/cat_turn.png", 4, 100, false)
        }

        PauseAnimation { duration: 420 }

        ScriptAction {
            script: {
                cat.mirrored = true
                cat.play(
                    "qrc:/assets/sprites/cat_walk.png", 8, 90, true)
                root.startWalkingAudio()
            }
        }

        NumberAnimation {
            target: cat
            property: "x"
            to: -cat.width - 40
            duration: 1650
            easing.type: Easing.InCubic
        }

        ScriptAction {
            script: root.stopWalkingAudio()
        }

        ScriptAction {
            script: appController.completeLeaving()
        }
    }

    Timer {
        id: pokeTimer
        interval: 440
        repeat: false
        onTriggered: {
            if (appController.phase === "rest")
                root.playSleep()
        }
    }

    Timer {
        id: hissTimer
        interval: 1400
        repeat: false
        onTriggered: {
            if (appController.phase === "rest") {
                cat.play(
                    "qrc:/assets/sprites/cat_settle.png", 6, 100, false)
                settleTimer.restart()
            }
        }
    }

    Timer {
        id: settleTimer
        interval: 620
        repeat: false
        onTriggered: {
            if (appController.phase === "rest")
                root.playSleep()
        }
    }

    Connections {
        target: appController

        function onPhaseChanged() {
            root.handlePhase()
        }

        function onCatPokeRequested() {
            if (!root.primaryOverlay
                    || appController.phase !== "rest"
                    || root.responseMode !== "")
                return

            root.responseMode = "poke"
            cat.play("qrc:/assets/sprites/cat_poke.png", 6, 70, false)
            pokeTimer.restart()
        }

        function onCatHissRequested() {
            if (!root.primaryOverlay || appController.phase !== "rest")
                return

            pokeTimer.stop()
            settleTimer.stop()
            root.responseMode = "hiss"
            speechBubble.opacity = 1
            cat.play("qrc:/assets/sprites/cat_hiss.png", 10, 90, false)
            meowSound.play()
            hissTimer.restart()
        }
    }

    Component.onCompleted: handlePhase()
}
