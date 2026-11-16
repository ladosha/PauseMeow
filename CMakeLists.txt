cmake_minimum_required(VERSION 3.24)

project(PauseMeow VERSION 1.0.0 LANGUAGES CXX)

include(GNUInstallDirs)

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

if(UNIX AND NOT APPLE)
    find_package(Qt6 6.8 REQUIRED COMPONENTS DBus)
endif()

qt_standard_project_setup(REQUIRES 6.8)

qt_add_executable(PauseMeow WIN32
    src/main.cpp
    src/AppController.cpp
    src/AppController.h
    src/CountdownTimer.cpp
    src/CountdownTimer.h
    src/SingleInstance.cpp
    src/SingleInstance.h
    src/AutostartEntry.cpp
    src/AutostartEntry.h
    src/PlatformIntegration.cpp
    src/PlatformIntegration.h
    src/OverlayGeometryController.cpp
    src/OverlayGeometryController.h
)

if(WIN32)
    configure_file(
        src/PauseMeow.rc.in
        "${CMAKE_CURRENT_BINARY_DIR}/PauseMeow.rc"
        @ONLY
    )
    target_sources(PauseMeow PRIVATE "${CMAKE_CURRENT_BINARY_DIR}/PauseMeow.rc")
endif()

qt_add_qml_module(PauseMeow
    URI PauseMeow
    VERSION 1.0
    QML_FILES
        qml/Main.qml
        qml/RestOverlay.qml
        qml/SpritePlayer.qml
)

qt_add_resources(PauseMeow "sprite_assets"
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

target_include_directories(PauseMeow PRIVATE src)

target_compile_definitions(PauseMeow PRIVATE
    PAUSEMEOW_VERSION="${PROJECT_VERSION}"
)

target_link_libraries(PauseMeow PRIVATE
    Qt6::Core
    Qt6::Gui
    Qt6::Multimedia
    Qt6::Qml
    Qt6::Quick
    Qt6::QuickControls2
    Qt6::Network
    Qt6::Widgets
)

if(UNIX AND NOT APPLE)
    target_link_libraries(PauseMeow PRIVATE Qt6::DBus)
    set_target_properties(PauseMeow PROPERTIES
        OUTPUT_NAME pausemeow
        RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/bin"
    )

    install(TARGETS PauseMeow
        RUNTIME DESTINATION "${CMAKE_INSTALL_BINDIR}"
    )
    install(FILES packaging/linux/com.vladmas.PauseMeow.desktop
        DESTINATION "${CMAKE_INSTALL_DATADIR}/applications"
    )
    install(FILES packaging/linux/com.vladmas.PauseMeow.metainfo.xml
        DESTINATION "${CMAKE_INSTALL_DATADIR}/metainfo"
    )
    install(FILES assets/app_icon.png
        DESTINATION "${CMAKE_INSTALL_DATADIR}/icons/hicolor/256x256/apps"
        RENAME com.vladmas.PauseMeow.png
    )
    install(FILES LICENSE
        DESTINATION "${CMAKE_INSTALL_DATADIR}/licenses/PauseMeow"
    )
    install(DIRECTORY THIRD_PARTY_LICENSES
        DESTINATION "${CMAKE_INSTALL_DATADIR}/licenses/PauseMeow"
    )
    install(FILES
        README.md
        ASSET_PROVENANCE.md
        PRIVACY.md
        THIRD_PARTY_NOTICES.md
        DESTINATION "${CMAKE_INSTALL_DATADIR}/doc/PauseMeow"
    )
endif()

include(CTest)
if(BUILD_TESTING)
    find_package(Qt6 6.8 REQUIRED COMPONENTS Test)

    qt_add_executable(PauseMeowTests
        tests/tst_countdowntimer.cpp
        src/CountdownTimer.cpp
        src/CountdownTimer.h
    )
    target_include_directories(PauseMeowTests PRIVATE src)
    target_link_libraries(PauseMeowTests PRIVATE Qt6::Core Qt6::Test)

    add_test(NAME PauseMeowTests COMMAND PauseMeowTests)

    qt_add_executable(PauseMeowAutostartTests
        tests/tst_autostartentry.cpp
        src/AutostartEntry.cpp
        src/AutostartEntry.h
    )
    target_include_directories(PauseMeowAutostartTests PRIVATE src)
    target_link_libraries(PauseMeowAutostartTests PRIVATE Qt6::Core Qt6::Test)

    add_test(NAME PauseMeowAutostartTests COMMAND PauseMeowAutostartTests)

    if(WIN32)
        qt_add_executable(PauseMeowWindowsIntegrationTests
            tests/tst_windowsintegration.cpp
            src/AppController.cpp
            src/AppController.h
            src/CountdownTimer.cpp
            src/CountdownTimer.h
            src/PlatformIntegration.cpp
            src/PlatformIntegration.h
            src/AutostartEntry.cpp
            src/AutostartEntry.h
        )
        target_include_directories(PauseMeowWindowsIntegrationTests PRIVATE src)
        target_compile_definitions(PauseMeowWindowsIntegrationTests PRIVATE
            PAUSEMEOW_TESTING
        )
        set_target_properties(PauseMeowWindowsIntegrationTests PROPERTIES
            WIN32_EXECUTABLE FALSE
        )
        target_link_libraries(PauseMeowWindowsIntegrationTests PRIVATE
            Qt6::Core
            Qt6::Gui
            Qt6::Test
            Qt6::Widgets
        )

        add_test(
            NAME PauseMeowWindowsIntegrationTests
            COMMAND PauseMeowWindowsIntegrationTests
        )
    endif()
endif()
