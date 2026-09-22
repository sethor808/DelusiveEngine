#Editor entry point - supplies main() and drives the engine in editor mode

add_executable(DelusiveGameEditor DelusiveGameEditor.cpp)

target_link_libraries(DelusiveGameEditor PRIVATE
    DelusiveEngine
    DelusiveScripts
)

#Windows needs the runtime DLLs beside the executable - see THIRDPARTY.md
if(WIN32)
    foreach(DLL SDL3.dll glew32.dll)
        if(EXISTS ${CMAKE_SOURCE_DIR}/lib/${DLL})
            add_custom_command(TARGET DelusiveGameEditor POST_BUILD
                COMMAND ${CMAKE_COMMAND} -E copy_if_different
                    ${CMAKE_SOURCE_DIR}/lib/${DLL}
                    $<TARGET_FILE_DIR:DelusiveGameEditor>
            )
        endif()
    endforeach()
endif()
