enable_testing()
set(SCRIPTS_PATH "${CMAKE_SOURCE_DIR}/scripts")
set(INTERNAL_SCRIPTS_PATH "${SCRIPTS_PATH}/internal")
set(TEST_LOGS_PATH "${CMAKE_SOURCE_DIR}/logs/tests")
file(MAKE_DIRECTORY ${TEST_LOGS_PATH})

if(WIN32)
    set(RUN_AND_OUTPUT_SCRIPT_NAME run_and_output.bat)
else()
    set(RUN_AND_OUTPUT_SCRIPT_NAME run_and_output.sh)
endif()


function(_set_default_output_directories target)
    set_target_properties(${target}
        PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
        LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib
        ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib
    )
endfunction()


function(_create_language_compile_feature_name lang std out_std_keyword)
    if("${lang}" STREQUAL "C")
        set(${out_std_keyword} "c_std_${std}" PARENT_SCOPE)
    elseif("${lang}" STREQUAL "CXX")
        set(${out_std_keyword} "cxx_std_${std}" PARENT_SCOPE)
    else()
        message(FATAL_ERROR "Unsupported language: ${lang}")
    endif()
endfunction()


# create_library(libname, libtype, lang, std, defs, incs)
# defs - "VAR1=3;VAR2;VAR3=\"str\""
# incs - "path1;path2"
function(create_interface_library libname lang std defs incs)
    # libname   -> target name
    # lang      -> C or CXX
    # std       -> standard number (e.g., 99, 11, 17, 20)
    # defs      -> list of compile definitions
    # incs      -> list of include directories

    set(compile_feature_name "")

    add_library(${libname} INTERFACE)
    _create_language_compile_feature_name(${lang} ${std} compile_feature_name)

    target_compile_features(${libname} INTERFACE ${compile_feature_name})
    target_compile_definitions(${libname} INTERFACE ${defs})
    target_include_directories(${libname} INTERFACE ${incs})
endfunction()


# create_library(libname, libtype, lang, std, defs, incs, srcs...)
# defs - "VAR1=3;VAR2;VAR3=\"str\""
# incs - "path1;path2"
function(create_library libname libtype lang std defs incs)
    # libname   -> target name
    # libtype   -> STATIC, SHARED
    # lang      -> C or CXX
    # std       -> standard number (e.g., 99, 11, 17, 20)
    # defs      -> list of compile definitions
    # incs      -> list of include directories
    # ${ARGN}   -> source files

    if((${libtype} STREQUAL "STATIC") OR (${libtype} STREQUAL "SHARED"))
        set(compile_feature_name)

        add_library(${libname} ${libtype})
        _set_default_output_directories(${libname})
        _create_language_compile_feature_name(${lang} ${std} compile_feature_name)

        target_compile_features(${libname} PUBLIC ${compile_feature_name})
        target_compile_definitions(${libname} PRIVATE ${defs})
        target_include_directories(${libname} PUBLIC ${incs})
        target_sources(${libname} PRIVATE ${ARGN})
    elseif(${libtype} STREQUAL "INTERFACE")
        message(FATAL_ERROR "Cannot use INTERFACE type. Call create_interface_library() instead")
    else()
        message(FATAL_ERROR "Unsupported library type: ${libtype}")
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
    target_link_libraries(${exename} PRIVATE ${libs})
endfunction()


function(create_ctest exename)
    add_test(
        NAME ${exename}
        COMMAND "${INTERNAL_SCRIPTS_PATH}/${RUN_AND_OUTPUT_SCRIPT_NAME}" $<TARGET_FILE:${exename}> "${TEST_LOGS_PATH}/${exename}.log"
    )
endfunction()
