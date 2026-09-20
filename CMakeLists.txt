cmake_minimum_required(VERSION 3.24)

project(DeskCat VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

find_package(Qt6 6.8 REQUIRED COMPONENTS
    Core
    Gui
    Multimedia
    Qml
    Quick
    QuickControls2
    Network
    Widgets
)

qt_standard_project_setup(REQUIRES 6.8)

qt_add_executable(DeskCat WIN32
    src/main.cpp
    src/AppController.cpp
    src/AppController.h
    src/CountdownTimer.cpp
    src/CountdownTimer.h
    src/SingleInstance.cpp
    src/SingleInstance.h
)

if(WIN32)
    target_sources(DeskCat PRIVATE src/DeskCat.rc)
endif()

qt_add_qml_module(DeskCat
    URI DeskCat
    VERSION 1.0
    QML_FILES
        qml/Main.qml
        qml/RestOverlay.qml
        qml/SpritePlayer.qml
)

qt_add_resources(DeskCat "sprite_assets"
    PREFIX "/"
    FILES
        assets/app_icon.png
        assets/audio/cat_steps.ogg
        assets/audio/cat_purr.ogg
        assets/audio/cat_meow.ogg
        assets/sprites/cat_walk.png
        assets/sprites/cat_lie_down.png
        assets/sprites/cat_sleep.png
        assets/sprites/cat_poke.png
        assets/sprites/cat_hiss.png
        assets/sprites/cat_settle.png
        assets/sprites/cat_wake_yawn.png
        assets/sprites/cat_turn.png
)

target_include_directories(DeskCat PRIVATE src)

target_link_libraries(DeskCat PRIVATE
    Qt6::Core
    Qt6::Gui
    Qt6::Multimedia
    Qt6::Qml
    Qt6::Quick
    Qt6::QuickControls2
    Qt6::Network
    Qt6::Widgets
)

include(CTest)
if(BUILD_TESTING)
    find_package(Qt6 6.8 REQUIRED COMPONENTS Test)

    qt_add_executable(DeskCatTests
        tests/tst_countdowntimer.cpp
        src/CountdownTimer.cpp
        src/CountdownTimer.h
    )
    target_include_directories(DeskCatTests PRIVATE src)
    target_link_libraries(DeskCatTests PRIVATE Qt6::Core Qt6::Test)

    add_test(NAME DeskCatTests COMMAND DeskCatTests)
endif()
