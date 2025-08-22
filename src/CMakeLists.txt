find_package(OpenGL REQUIRED)
find_package(glfw3 REQUIRED)
find_package(OpenCV REQUIRED)   # <---- Add this

add_library(glad STATIC glad.c)
target_include_directories(glad PUBLIC ${PROJECT_SOURCE_DIR}/include)

# ==== ImGui integration ====
set(IMGUI_DIR ${PROJECT_SOURCE_DIR}/extern/imgui)

set(IMGUI_SRC
    ${IMGUI_DIR}/imgui.cpp
    ${IMGUI_DIR}/imgui_draw.cpp
    ${IMGUI_DIR}/imgui_tables.cpp
    ${IMGUI_DIR}/imgui_widgets.cpp
    ${IMGUI_DIR}/backends/imgui_impl_glfw.cpp
    ${IMGUI_DIR}/backends/imgui_impl_opengl3.cpp
)

add_library(imgui STATIC ${IMGUI_SRC})
target_include_directories(imgui PUBLIC
    ${IMGUI_DIR}
    ${IMGUI_DIR}/backends
)

target_link_libraries(imgui PUBLIC
    glfw
    OpenGL::GL
)

# ==== Main executable ====
add_executable(project_exe
    main.cpp
)

target_include_directories(project_exe PUBLIC
    ${PROJECT_SOURCE_DIR}/include
    ${OpenCV_INCLUDE_DIRS}
)

target_link_libraries(project_exe PRIVATE
    glad
    imgui
    glfw
    OpenGL::GL
    ${OpenCV_LIBS}
)
