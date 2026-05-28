cmake_minimum_required(VERSION 3.16)
project(TegraRcmGUI VERSION 2.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find Qt6 (QuickControls2 is merged into Quick in Qt6)
find_package(Qt6 COMPONENTS Core Gui Widgets Quick Concurrent REQUIRED)

# Core Backend files (No MFC dependencies)
set(BACKEND_SOURCES
    TegraRcmSmash.cpp
    PayloadController.cpp
)

set(BACKEND_HEADERS
    TegraRcmSmash.h
    PayloadController.h
    libusbk_int.h
    ScopeGuard.h
    Types.h
    Win32Def.h
    WinHandle.h
)

# Add autogen tools for Qt
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)

# Create the executable with main.cpp and backend sources
add_executable(${PROJECT_NAME}_Qt
    main.cpp
    ${BACKEND_SOURCES}
    ${BACKEND_HEADERS}
)

# Register the QML module, modernizing resource management and linking C++ files
qt_add_qml_module(${PROJECT_NAME}_Qt
    URI TegraRcm
    VERSION 1.0
    QML_FILES
        main.qml
    SOURCES
        AppController.h
        AppController.cpp
)

# Link against Qt6 and required Windows libraries
target_link_libraries(${PROJECT_NAME}_Qt PRIVATE 
    Qt6::Core 
    Qt6::Gui 
    Qt6::Widgets
    Qt6::Quick
    Qt6::Concurrent
    setupapi
)

