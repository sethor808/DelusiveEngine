#Gameplay scripts, linked statically into the editor and game

file(GLOB_RECURSE SCRIPT_SOURCES CONFIGURE_DEPENDS
    ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp
)

add_library(DelusiveScripts STATIC ${SCRIPT_SOURCES})

target_include_directories(DelusiveScripts PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(DelusiveScripts PUBLIC DelusiveEngine)
