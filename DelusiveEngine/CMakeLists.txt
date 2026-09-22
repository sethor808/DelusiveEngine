#Engine is a static library - DelusiveGameEditor supplies main()

file(GLOB_RECURSE ENGINE_SOURCES CONFIGURE_DEPENDS
    ${CMAKE_CURRENT_SOURCE_DIR}/src/Delusive/*.cpp
)

#Orphaned: its header was removed during the serialization overhaul
list(REMOVE_ITEM ENGINE_SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/src/Delusive/Runtime/UI/UIEquipScreen.cpp
)

add_library(DelusiveEngine STATIC ${ENGINE_SOURCES})

target_include_directories(DelusiveEngine PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_SOURCE_DIR}/include
)

target_link_libraries(DelusiveEngine PUBLIC
    delusive_thirdparty
    delusive_imgui
    delusive_sdl3
    delusive_glew
    OpenGL::GL
)

target_compile_definitions(DelusiveEngine PUBLIC SDL_MAIN_HANDLED)
