import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QtQuick.Window

ApplicationWindow {
    id: window

    width: 760
    height: 720
    minimumWidth: 680
    minimumHeight: 660
    visible: appController.settingsVisible
    title: "DeskCat"
    color: "#F6F0E7"

    onVisibleChanged: {
        if (visible) {
            raise()
            requestActivate()
        }
    }

    onClosing: function(close) {
        close.accepted = false
        appController.hideSettings()
    }

    component SectionTitle: Text {
        color: "#302B38"
        font.pixelSize: 15
        font.weight: Font.DemiBold
    }

    component TimeCard: Rectangle {
        property string cardTitle: ""
        property string cardSubtitle: ""
        property alias value: spin.value
        property int from: 1
        property int to: 240

        radius: 22
        color: "#FFFDFC"
        border.color: "#E7DDD2"
        border.width: 1
        implicitHeight: 150

        ColumnLayout {
            anchors.fill: parent
            anchors.margins: 20
            spacing: 8

            Text {
                text: parent.parent.cardTitle
                color: "#302B38"
                font.pixelSize: 18
                font.weight: Font.DemiBold
            }

            Text {
                text: parent.parent.cardSubtitle
                color: "#807682"
                font.pixelSize: 13
                Layout.fillWidth: true
                wrapMode: Text.WordWrap
            }

            RowLayout {
                Layout.fillWidth: true
                Item { Layout.fillWidth: true }

                SpinBox {
                    id: spin
                    from: parent.parent.parent.from
                    to: parent.parent.parent.to
                    editable: true
                    font.pixelSize: 22

                    background: Rectangle {
                        implicitWidth: 150
                        implicitHeight: 48
                        radius: 14
                        color: "#F7F1E8"
                        border.color: spin.activeFocus ? "#E47C2B" : "#DED2C5"
                    }
                }

                Text {
                    text: "мин"
                    color: "#807682"
                    font.pixelSize: 15
                }

                Item { Layout.fillWidth: true }
            }
        }
    }

    Rectangle {
        anchors.fill: parent
        color: "#F6F0E7"

        Rectangle {
            anchors.top: parent.top
            anchors.left: parent.left
            anchors.right: parent.right
            height: 280
            color: "#2A2633"
            clip: true

            Rectangle {
                width: 260
                height: 260
                radius: 130
                color: "#403648"
                opacity: 0.55
                anchors.right: parent.right
                anchors.rightMargin: 28
                anchors.top: parent.top
                anchors.topMargin: 10
            }
        }

        ScrollView {
            anchors.fill: parent
            contentWidth: availableWidth
            clip: true

            ColumnLayout {
                width: window.width
                spacing: 18

                Item {
                    Layout.fillWidth: true
                    Layout.preferredHeight: 268

                    SpritePlayer {
                        width: 190
                        height: 190
                        anchors.right: parent.right
                        anchors.rightMargin: 38
                        anchors.verticalCenter: parent.verticalCenter

                        Component.onCompleted: play(
                            "qrc:/assets/sprites/cat_sleep.png", 6, 350, true)
                    }

                    Column {
                        anchors.left: parent.left
                        anchors.leftMargin: 38
                        anchors.verticalCenter: parent.verticalCenter
                        width: 430
                        spacing: 8

                        Text {
                            text: "DeskCat"
                            color: "#FFF8EF"
                            font.pixelSize: 38
                            font.weight: Font.Bold
                        }

                        Text {
                            width: parent.width
                            text: "Ваш личный хранитель перерывов"
                            color: "#D8CEDD"
                            font.pixelSize: 17
                            wrapMode: Text.WordWrap
                        }
                    }
                }

                Rectangle {
                    Layout.leftMargin: 32
                    Layout.rightMargin: 32
                    Layout.fillWidth: true
                    Layout.preferredHeight: 76
                    radius: 20
                    color: "#FFFDFC"
                    border.color: "#E7DDD2"

                    RowLayout {
                        anchors.fill: parent
                        anchors.leftMargin: 22
                        anchors.rightMargin: 22
                        spacing: 14

                        Rectangle {
                            width: 12
                            height: 12
                            radius: 6
                            color: appController.phase === "rest"
                                ? "#79A978"
                                : appController.paused ? "#D7A846" : "#E47C2B"
                        }

                        Column {
                            spacing: 2
                            Text {
                                text: {
                                    if (appController.paused) return "Таймер приостановлен"
                                    if (appController.phase === "work") return "Рабочий цикл"
                                    if (appController.phase === "rest") return "Кот отдыхает"
                                    if (appController.phase === "entering") return "Кот уже идёт"
                                    if (appController.phase === "leaving") return "Перерыв закончен"
                                    return "Настройте первый цикл"
                                }
                                color: "#302B38"
                                font.pixelSize: 15
                                font.weight: Font.DemiBold
                            }
                            Text {
                                text: appController.configured
                                    ? "Следующая смена режима произойдёт автоматически"
                                    : "После сохранения приложение останется в системном трее"
                                color: "#807682"
                                font.pixelSize: 12
                            }
                        }

                        Item { Layout.fillWidth: true }

                        Text {
                            text: appController.remainingText
                            color: "#302B38"
                            font.pixelSize: 28
                            font.weight: Font.Bold
                            font.family: "Consolas"
                        }
                    }
                }

                SectionTitle {
                    text: "Длительность цикла"
                    Layout.leftMargin: 36
                    Layout.topMargin: 4
                }

                RowLayout {
                    Layout.leftMargin: 32
                    Layout.rightMargin: 32
                    Layout.fillWidth: true
                    spacing: 16

                    TimeCard {
                        id: workCard
                        Layout.fillWidth: true
                        cardTitle: "Работа"
                        cardSubtitle: "Сколько кот не будет вас беспокоить"
                        value: appController.workMinutes
                        from: 1
                        to: 240
                    }

                    TimeCard {
                        id: restCard
                        Layout.fillWidth: true
                        cardTitle: "Отдых"
                        cardSubtitle: "Сколько кот будет спать на экране"
                        value: appController.restMinutes
                        from: 1
                        to: 60
                    }
                }

                SectionTitle {
                    text: "Поведение"
                    Layout.leftMargin: 36
                    Layout.topMargin: 4
                }

                Rectangle {
                    Layout.leftMargin: 32
                    Layout.rightMargin: 32
                    Layout.fillWidth: true
                    Layout.preferredHeight: 142
                    radius: 20
                    color: "#FFFDFC"
                    border.color: "#E7DDD2"

                    ColumnLayout {
                        anchors.fill: parent
                        anchors.leftMargin: 20
                        anchors.rightMargin: 20
                        spacing: 0

                        CheckBox {
                            id: dimCheck
                            text: "Затемнять фон во время отдыха"
                            checked: appController.dimBackground
                            font.pixelSize: 14
                            Layout.fillWidth: true
                        }

                        CheckBox {
                            id: strictCheck
                            text: "Строгий режим — для трудоголиков"
                            checked: appController.strictMode
                            font.pixelSize: 14
                            Layout.fillWidth: true
                        }

                        CheckBox {
                            id: autostartCheck
                            text: "Запускать DeskCat вместе с Windows"
                            checked: appController.autostart
                            font.pixelSize: 14
                            Layout.fillWidth: true
                        }
                    }
                }

                SectionTitle {
                    text: "Звук"
                    Layout.leftMargin: 36
                    Layout.topMargin: 4
                }

                Rectangle {
                    Layout.leftMargin: 32
                    Layout.rightMargin: 32
                    Layout.fillWidth: true
                    Layout.preferredHeight: 154
                    radius: 20
                    color: "#FFFDFC"
                    border.color: "#E7DDD2"

                    ColumnLayout {
                        anchors.fill: parent
                        anchors.leftMargin: 20
                        anchors.rightMargin: 20
                        spacing: 0

                        CheckBox {
                            id: walkingSoundsCheck
                            text: "Звуки шагов во время ходьбы"
                            checked: appController.walkingSounds
                            font.pixelSize: 14
                            Layout.fillWidth: true
                        }

                        CheckBox {
                            id: walkingPurrCheck
                            text: "Мурчание во время ходьбы"
                            checked: appController.walkingPurr
                            font.pixelSize: 14
                            Layout.fillWidth: true
                        }

                        CheckBox {
                            id: restPurrCheck
                            text: "Мурчание во время перерыва"
                            checked: appController.restPurr
                            font.pixelSize: 14
                            Layout.fillWidth: true
                        }
                    }
                }

                RowLayout {
                    Layout.leftMargin: 32
                    Layout.rightMargin: 32
                    Layout.bottomMargin: 30
                    Layout.fillWidth: true
                    spacing: 12

                    Button {
                        text: "Посмотреть перерыв"
                        Layout.preferredHeight: 52
                        Layout.preferredWidth: 220
                        font.pixelSize: 15

                        background: Rectangle {
                            radius: 16
                            color: parent.down ? "#E4D8CB" : "#EFE5DA"
                        }

                        onClicked: appController.previewBreak(12)
                    }

                    Button {
                        text: appController.paused ? "Продолжить" : "Пауза"
                        visible: appController.phase === "work"
                        Layout.preferredHeight: 52
                        Layout.preferredWidth: 130
                        onClicked: appController.togglePause()

                        background: Rectangle {
                            radius: 16
                            color: parent.down ? "#E4D8CB" : "#EFE5DA"
                        }
                    }

                    Item { Layout.fillWidth: true }

                    Button {
                        text: appController.configured
                            ? "Сохранить и перезапустить"
                            : "Сохранить и начать"
                        Layout.preferredHeight: 52
                        Layout.preferredWidth: 230
                        font.pixelSize: 15
                        font.weight: Font.DemiBold
                        palette.buttonText: "#FFFFFF"

                        background: Rectangle {
                            radius: 16
                            color: parent.down ? "#C96520" : "#E47C2B"
                        }

                        onClicked: appController.applySettings(
                            workCard.value,
                            restCard.value,
                            dimCheck.checked,
                            strictCheck.checked,
                            autostartCheck.checked,
                            walkingSoundsCheck.checked,
                            walkingPurrCheck.checked,
                            restPurrCheck.checked)
                    }
                }
            }
        }
    }

    Instantiator {
        model: Qt.application.screens

        delegate: RestOverlay {
            required property var modelData
            required property int index

            targetScreen: modelData
            primaryOverlay: index === 0
        }
    }
}
