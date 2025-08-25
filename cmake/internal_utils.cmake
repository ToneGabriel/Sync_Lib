
function(_set_default_output_directories target)
    set_target_properties(${target}
        PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
        LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib
        ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib
    )
endfunction()


# create_library(libname, libtype, lang, std, inc, src...)
function(create_library libname libtype lang std inc)
    # libname  -> target name
    # libtype  -> STATIC, SHARED, INTERFACE
    # lang     -> C or CXX
    # std      -> standard number (e.g., 99, 11, 17, 20)
    # inc      -> include directory
    # ${ARGN}  -> source files

    # Convert standard to CMake keyword
    if(lang STREQUAL "C")
        set(std_keyword "c_std_${std}")
    elseif(lang STREQUAL "CXX")
        set(std_keyword "cxx_std_${std}")
    else()
        message(FATAL_ERROR "Unsupported language: ${lang}")
    endif()

    # Create library target
    add_library(${libname} ${libtype})

    if(libtype STREQUAL "INTERFACE")
        # Header-only / interface library
        target_compile_features(${libname} INTERFACE ${std_keyword})
        target_include_directories(${libname} INTERFACE ${inc})
    else()
        # Compiled library
        target_compile_features(${libname} PUBLIC ${std_keyword})
        target_include_directories(${libname} PUBLIC ${inc})
        target_sources(${libname} PRIVATE ${ARGN})

        # Default output directories
        _set_default_output_directories(${libname})
    endif()
endfunction()


# create_executable(exename, libs, src...)
# libs - "lib1;lib2;..."
function(create_executable exename libs)
    # exename   -> target name
    # libs      -> list of library names
    # ${ARGN}   -> source files

    add_executable(${exename} ${ARGN})
    _set_default_output_directories(${exename})

    foreach (lib ${libs})
        target_link_libraries(${exename} PRIVATE ${lib})
    endforeach()
endfunction()
