function(anybox_add_system_library target)
    add_library(${target} SHARED ${ARGN})
    target_compile_definitions(${target} PRIVATE ANYBOX_LIBRARY_NAME="${target}.dll")
    target_link_libraries(${target} PRIVATE anybox_common PUBLIC anybox_options)
    set_target_properties(${target} PROPERTIES
        PREFIX ""
        OUTPUT_NAME "${target}"
        RUNTIME_OUTPUT_DIRECTORY "${ANYBOX_RUNTIME_DIRECTORY}"
        PDB_OUTPUT_DIRECTORY "${ANYBOX_RUNTIME_DIRECTORY}"
        ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib/$<CONFIG>"
        WINDOWS_EXPORT_ALL_SYMBOLS OFF
    )
    if(MINGW AND CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
        get_filename_component(compilerDirectory "${CMAKE_CXX_COMPILER}" DIRECTORY)
        foreach(runtime IN ITEMS libgcc_s_seh-1.dll libstdc++-6.dll libwinpthread-1.dll)
            set(runtimePath "${compilerDirectory}/${runtime}")
            if(NOT IS_ABSOLUTE "${runtimePath}" OR NOT EXISTS "${runtimePath}")
                message(FATAL_ERROR "Required compiler runtime was not found: ${runtime}")
            endif()
            add_custom_command(TARGET ${target} POST_BUILD COMMAND "${CMAKE_COMMAND}" -E copy_if_different "${runtimePath}" "$<TARGET_FILE_DIR:${target}>" VERBATIM)
        endforeach()
    endif()
    add_dependencies(libs ${target})
endfunction()
