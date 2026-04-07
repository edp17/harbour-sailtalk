cmake_minimum_required(VERSION 3.6)
project(harbour-sailtalk LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

find_package(PkgConfig REQUIRED)
find_package(Qt5 REQUIRED COMPONENTS Core Gui Qml Quick Network WebSockets)

pkg_check_modules(GSTREAMER REQUIRED
    gstreamer-1.0
    gstreamer-webrtc-1.0
    gstreamer-sdp-1.0
)

pkg_check_modules(GLIB2 REQUIRED glib-2.0)
pkg_check_modules(SAILFISHAPP REQUIRED sailfishapp)

set(SOURCES
    src/main.cpp
    src/appengine.cpp
    src/webrtcmanager.cpp
    src/signalingclient.cpp
)

set(HEADERS
    src/appengine.h
    src/webrtcmanager.h
    src/signalingclient.h
)

add_executable(harbour-sailtalk
    ${SOURCES}
    ${HEADERS}
)

target_compile_definitions(harbour-sailtalk PRIVATE GST_USE_UNSTABLE_API)

target_include_directories(harbour-sailtalk PRIVATE
    ${GSTREAMER_INCLUDE_DIRS}
    ${GLIB2_INCLUDE_DIRS}
    ${SAILFISHAPP_INCLUDE_DIRS}
    ${CMAKE_SOURCE_DIR}/src
)

target_link_libraries(harbour-sailtalk
    Qt5::Core
    Qt5::Gui
    Qt5::Qml
    Qt5::Quick
    Qt5::Network
    Qt5::WebSockets
    ${GSTREAMER_LIBRARIES}
    ${GLIB2_LIBRARIES}
    ${SAILFISHAPP_LIBRARIES}
)

target_compile_options(harbour-sailtalk PRIVATE
    ${GSTREAMER_CFLAGS_OTHER}
    ${GLIB2_CFLAGS_OTHER}
    ${SAILFISHAPP_CFLAGS_OTHER}
)

target_compile_definitions(harbour-sailtalk PRIVATE GST_USE_UNSTABLE_API)

install(TARGETS harbour-sailtalk
    RUNTIME DESTINATION /usr/bin
)

install(FILES qml/harbour-sailtalk.qml
    DESTINATION /usr/share/harbour-sailtalk/qml
)

install(DIRECTORY qml/pages qml/components qml/cover
    DESTINATION /usr/share/harbour-sailtalk/qml
)

install(FILES icons/172x172/harbour-sailtalk.png
    DESTINATION /usr/share/icons/hicolor/172x172/apps
)

install(FILES harbour-sailtalk.desktop
    DESTINATION /usr/share/applications
)
