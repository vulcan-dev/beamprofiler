cmake_minimum_required(VERSION 2.9)
project(BeamProfiler)

set(SOURCES
    code/src/main.cc
    code/src/interface.cc code/include/interface.h
    code/include/beamprofiler.h
    code/src/config.cc code/include/config.h

    # ImGui
    deps/imgui/imgui.cpp deps/imgui/imgui.h
    deps/imgui/imgui_widgets.cpp
    deps/imgui/imgui_tables.cpp
    deps/imgui/imgui_draw.cpp
    deps/imgui/imgui_demo.cpp

    deps/imgui/backends/imgui_impl_opengl2.cpp deps/imgui/backends/imgui_impl_opengl2.h
    deps/imgui/backends/imgui_impl_sdl2.cpp deps/imgui/backends/imgui_impl_sdl2.h

    deps/imgui/imstb_truetype.h
    deps/imgui/imstb_textedit.h
    deps/imgui/imstb_rectpack.h

    # ImPlot
    deps/implot/implot.cpp deps/implot/implot.h
    deps/implot/implot_items.cpp
    deps/implot/implot_demo.cpp
    deps/implot/implot_internal.h
)

find_package(SDL2 REQUIRED)

add_executable(${PROJECT_NAME} WIN32 ${SOURCES} res/app.o)
set_property(TARGET ${PROJECT_NAME} PROPERTY CXX_STANDARD 11)
target_include_directories(${PROJECT_NAME} PRIVATE ${SDL2_INCLUDE_DIR} code/include deps/imgui deps/imgui/backends deps/implot)
target_link_libraries(${PROJECT_NAME} SDL2::SDL2 opengl32)

# cmake -DCMAKE_PREFIX_PATH="F:\SDK\SDL\Built" ..