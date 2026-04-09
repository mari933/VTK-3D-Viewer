cmake_minimum_required(VERSION 3.10)
project(VTUViewer)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_AUTOMOC ON)

find_package(Qt5 REQUIRED COMPONENTS Widgets OpenGL)

find_package(VTK REQUIRED COMPONENTS 
    CommonCore
    FiltersSources
    IOGeometry
    IOParallelXML
    RenderingCore
    RenderingOpenGL2
    InteractionStyle
    GUISupportQt
    RenderingQt
    RenderingAnnotation
)

include(${VTK_USE_FILE})

add_executable(VTUViewer 
    main.cpp 
    MainWindow.cpp
)

target_link_libraries(VTUViewer
    Qt5::Widgets
    Qt5::OpenGL
    ${VTK_LIBRARIES}
)

target_include_directories(VTUViewer PRIVATE ${VTK_INCLUDE_DIRS})
