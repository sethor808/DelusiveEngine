cmake_minimum_required(VERSION 3.20)
project(Delusive LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if(MSVC)
    add_compile_options(/MP)
endif()

#Executables land in bin/ so the "../assets/..." paths in code still resolve
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/lib)

#==== Third party =============================================================
# Windows keeps using the vendored binaries already in include/ and lib/.
# Linux resolves SDL3 and GLEW through the system package manager.

find_package(OpenGL REQUIRED)

# Header-only deps (glm, stb, imgui headers) all live under include/
add_library(delusive_thirdparty INTERFACE)
target_include_directories(delusive_thirdparty INTERFACE ${CMAKE_SOURCE_DIR}/include)

add_library(delusive_sdl3 INTERFACE)
add_library(delusive_glew INTERFACE)

if(WIN32)
    target_include_directories(delusive_sdl3 INTERFACE ${CMAKE_SOURCE_DIR}/include)
    target_link_libraries(delusive_sdl3 INTERFACE ${CMAKE_SOURCE_DIR}/lib/SDL3.lib)

    target_include_directories(delusive_glew INTERFACE ${CMAKE_SOURCE_DIR}/include)
    target_link_libraries(delusive_glew INTERFACE ${CMAKE_SOURCE_DIR}/lib/glew32.lib)
else()
    find_package(SDL3 REQUIRED)
    find_package(GLEW REQUIRED)

    target_link_libraries(delusive_sdl3 INTERFACE SDL3::SDL3)
    target_link_libraries(delusive_glew INTERFACE GLEW::GLEW)
endif()

#==== ImGui ===================================================================
# Built from include/imgui only. DelusiveEngine/src/imgui is a partial duplicate.

add_library(delusive_imgui STATIC
    ${CMAKE_SOURCE_DIR}/include/imgui/imgui.cpp
    ${CMAKE_SOURCE_DIR}/include/imgui/imgui_draw.cpp
    ${CMAKE_SOURCE_DIR}/include/imgui/imgui_tables.cpp
    ${CMAKE_SOURCE_DIR}/include/imgui/imgui_widgets.cpp
    ${CMAKE_SOURCE_DIR}/include/imgui/imgui_demo.cpp
    ${CMAKE_SOURCE_DIR}/include/imgui/backend/imgui_impl_opengl3.cpp
    ${CMAKE_SOURCE_DIR}/include/imgui/backend/imgui_impl_sdl3.cpp
)

target_include_directories(delusive_imgui PUBLIC
    ${CMAKE_SOURCE_DIR}/include
    ${CMAKE_SOURCE_DIR}/include/imgui
)

target_link_libraries(delusive_imgui PUBLIC delusive_sdl3 OpenGL::GL)

#==== Projects ================================================================

add_subdirectory(DelusiveEngine)
add_subdirectory(DelusiveScripts)
add_subdirectory(DelusiveGameEditor)
add_subdirectory(DelusiveTools)
add_subdirectory(Reflections)
