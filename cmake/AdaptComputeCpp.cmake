# ************************************************************************
#
#   SYCL Conformance Test Suite
#
#   Copyright:	(c) 2018 by Codeplay Software LTD. All Rights Reserved.
#
# ************************************************************************

add_library(SYCL::SYCL INTERFACE IMPORTED GLOBAL)
target_link_libraries(SYCL::SYCL INTERFACE ComputeCpp::ComputeCpp)
target_compile_definitions(SYCL::SYCL INTERFACE SYCL_LANGUAGE_VERSION=${SYCL_LANGUAGE_VERSION})
#set_target_properties(SYCL::SYCL PROPERTIES
#  INTERFACE_DEVICE_COMPILE_DEFINITIONS "SYCL_LANGUAGE_VERSION=${SYCL_LANGUAGE_VERSION}"
#  INTERFACE_DEVICE_COMPILE_OPTIONS     "-sycl"
#)

# build_spir
# Runs the device compiler on a single source file, creating the stub and the bc files.
function(build_spir exe_name spir_target_name source_file output_path)
    set(target ${spir_target_name})

    set(stub_file  ${spir_target_name}.cpp.sycl)
    set(bc_file    ${spir_target_name}.cpp.bc)

    set(output_bc "${output_path}/${bc_file}")
    set(output_stub "${output_path}/${stub_file}")

    if(WIN32 AND MSVC)
        set(platform_specific_args -fdiagnostics-format=msvc)
    endif()

    #message(STATUS "SYCL_LANGUAGE_VERSION: ${SYCL_LANGUAGE_VERSION}")

    #set(device_compile_definitions "$<TARGET_PROPERTY:SYCL::SYCL,INTERFACE_DEVICE_COMPILE_DEFINITIONS>")
    #set(device_compile_options "$<TARGET_PROPERTY:SYCL::SYCL,INTERFACE_DEVICE_COMPILE_OPTIONS>")
    set(include_directories "$<TARGET_PROPERTY:${exe_name},INCLUDE_DIRECTORIES>")
    set(compile_definitions "$<TARGET_PROPERTY:${exe_name},COMPILE_DEFINITIONS>")

    add_custom_command(
    OUTPUT  ${output_bc} ${output_stub}
    # We prepend the compiler launcher to enable SYCL_CTS_MEASURE_BUILD_TIMES
    # to also measure device compiler times.
    COMMAND ${CMAKE_CXX_COMPILER_LAUNCHER}
            "${ComputeCpp_DEVICE_COMPILER_EXECUTABLE}"
            ${COMPUTECPP_DEVICE_COMPILER_FLAGS}
            ${COMPUTECPP_USER_FLAGS}
            ${platform_specific_args}
            $<$<BOOL:${include_directories}>:-I\"$<JOIN:${include_directories},\"\;-I\">\">
            $<$<BOOL:${compile_definitions}>:-D$<JOIN:${compile_definitions},\;-D>>
            #$<$<BOOL:${device_compile_definitions}>:-D$<JOIN:${device_compile_definitions},\;-D>>
            #$<JOIN:${device_compile_options},\;>
            -sycl
            -o "${output_bc}"
            -c "${source_file}"
    COMMAND_EXPAND_LISTS
    DEPENDS "${source_file}"
    WORKING_DIRECTORY "${output_path}"
    COMMENT "Building SPIR object ${output_bc}")

    add_custom_target(${spir_target_name}_spir DEPENDS ${output_stub})
    set_property(TARGET ${spir_target_name}_spir PROPERTY FOLDER "Tests/${exe_name}/${exe_name}_spir")
endfunction()

# add_sycl_executable_implementation function
# Builds a SYCL program, compiling multiple SYCL test case source files into a test executable, invoking a single-source/device compiler
# Parameters are:
#   - NAME             Name of the test executable
#   - OBJECT_LIBRARY   Name of the object library of all the compiled test cases
#   - TESTS            List of SYCL test case source files to be built into the test executable
function(add_sycl_executable_implementation)
    cmake_parse_arguments(args "" "NAME;OBJECT_LIBRARY" "TESTS" ${ARGN})
    set(exe_name              ${args_NAME})
    set(object_lib_name       ${args_OBJECT_LIBRARY})
    set(test_cases_list       ${args_TESTS})
    set(destination_stub_path "${PROJECT_BINARY_DIR}/bin/intermediate")

    add_library(${object_lib_name} OBJECT ${test_cases_list})
    add_executable(${exe_name} $<TARGET_OBJECTS:${object_lib_name}>)
    set_target_properties(${object_lib_name} PROPERTIES
        INCLUDE_DIRECTORIES $<TARGET_PROPERTY:${exe_name},INCLUDE_DIRECTORIES>
        COMPILE_DEFINITIONS $<TARGET_PROPERTY:${exe_name},COMPILE_DEFINITIONS>
        COMPILE_OPTIONS     $<TARGET_PROPERTY:${exe_name},COMPILE_OPTIONS>
        COMPILE_FEATURES    $<TARGET_PROPERTY:${exe_name},COMPILE_FEATURES>
        POSITION_INDEPENDENT_CODE ON)
    set_target_properties(${object_lib_name} ${exe_name} PROPERTIES
        COMPILE_DEFINITIONS "CL_TARGET_OPENCL_VERSION=300;SYCL_LANGUAGE_VERSION=${SYCL_LANGUAGE_VERSION}")

    foreach(source_file ${test_cases_list})
        get_filename_component(spir_target_name ${source_file} NAME_WE)
        build_spir("${exe_name}" "${spir_target_name}" "${source_file}" "${destination_stub_path}")
        add_dependencies(${exe_name} ${spir_target_name}_spir)
        set(output_stub "${destination_stub_path}/${spir_target_name}.cpp.sycl")
        if(MSVC)
            set_source_files_properties(${source_file} PROPERTIES
                OBJECT_DEPENDS "${output_stub}"
                COMPILE_FLAGS  /FI\"${output_stub}\")
        else()
            set_source_files_properties(${source_file} PROPERTIES
                OBJECT_DEPENDS "${output_stub}"
                COMPILE_FLAGS  "-include ${output_stub} -Wno-nonportable-include-path")
	    endif()
    endforeach()

    set_target_properties(${exe_name} PROPERTIES
        FOLDER         "Tests/${exe_name}"
        #LINK_LIBRARIES "ComputeCpp::ComputeCpp;$<$<BOOL:${WIN32}>:-SAFESEH:NO>"
        BUILD_RPATH    "$<TARGET_FILE_DIR:ComputeCpp::ComputeCpp>")
endfunction()
