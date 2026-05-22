cmake_minimum_required(VERSION 3.16)
project(TegraRcmGUI VERSION 2.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find Qt6
find_package(Qt6 COMPONENTS Core Gui Widgets Quick QuickControls2 Concurrent REQUIRED)

# Core Backend files (No MFC dependencies)
set(BACKEND_SOURCES
    TegraRcmSmash.cpp
    PayloadController.cpp
    SettingsManager.cpp
)

set(BACKEND_HEADERS
    TegraRcmSmash.h
    PayloadController.h
    SettingsManager.h
    libusbk_int.h
    ScopeGuard.h
    Types.h
    Win32Def.h
    WinHandle.h
)

# Qt Quick frontend files
set(FRONTEND_SOURCES
    main.cpp
    AppController.cpp
)

set(FRONTEND_HEADERS
    AppController.h
)

set(FRONTEND_RESOURCES
    qml.qrc
)

# Add autogen tools for Qt
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

add_executable(${PROJECT_NAME}_Qt
    ${BACKEND_SOURCES}
    ${BACKEND_HEADERS}
    ${FRONTEND_SOURCES}
    ${FRONTEND_HEADERS}
    ${FRONTEND_RESOURCES}
)

# Link against Qt6 and required Windows libraries
target_link_libraries(${PROJECT_NAME}_Qt PRIVATE 
    Qt6::Core 
    Qt6::Gui 
    Qt6::Widgets
    Qt6::Quick
    Qt6::QuickControls2
    Qt6::QuickDialogs
    Qt6::Concurrent
    setupapi
    # libusbk.lib (Need to ensure paths to static libs are set if required)
)
