# Copyright (C) 2018-2022 Intel Corporation
# SPDX-License-Identifier: Apache-2.0
#

if(ENABLE_LTO)
    set(CMAKE_INTERPROCEDURAL_OPTIMIZATION_RELEASE ON)
endif()

add_definitions(-DIN_OV_CORE_LIBRARY)

set(OV_CORE_INCLUDE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/include)

if(CMAKE_COMPILER_IS_GNUCXX)
    ie_add_compiler_flags(-Wmissing-declarations)
endif()

file(GLOB_RECURSE LIBRARY_SRC ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp
                              ${CMAKE_CURRENT_SOURCE_DIR}/src/*.hpp)
file(GLOB_RECURSE PUBLIC_HEADERS ${CMAKE_CURRENT_SOURCE_DIR}/include/*.hpp)

add_subdirectory(builder)
add_subdirectory(reference)
add_subdirectory(shape_inference)

set(MIXED_SRC
    "${CMAKE_CURRENT_SOURCE_DIR}/src/runtime/allocator.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/runtime/ov_tensor.cpp")

set_property(SOURCE ${MIXED_SRC}
    APPEND PROPERTY INCLUDE_DIRECTORIES
        $<TARGET_PROPERTY:inference_engine_obj,SOURCE_DIR>/src
        $<TARGET_PROPERTY:inference_engine_plugin_api,INTERFACE_INCLUDE_DIRECTORIES>)

# Create named folders for the sources within the .vcproj
# Empty name lists them directly under the .vcproj

source_group("src" FILES ${LIBRARY_SRC})
source_group("include" FILES ${PUBLIC_HEADERS})

#
# Create ov_core_dev library
#

add_library(ov_core_dev INTERFACE)
add_library(openvino::core::dev ALIAS ov_core_dev)

target_include_directories(ov_core_dev INTERFACE $<BUILD_INTERFACE:${OpenVINO_SOURCE_DIR}/src/core/dev_api>)

set_target_properties(ov_core_dev PROPERTIES EXPORT_NAME core::dev)
openvino_developer_export_targets(COMPONENT core TARGETS openvino::core::dev)

# Install interface libraries for case BUILD_SHARED_LIBS=OFF
ov_install_static_lib(ov_core_dev ${OV_CPACK_COMP_CORE})

#
# Create static or shared library depending on BUILD_SHARED_LIBS
#

add_library(ngraph_obj OBJECT ${LIBRARY_SRC} ${PUBLIC_HEADERS})

if(ENABLE_SYSTEM_PUGIXML)
    # system pugixml has /usr/include as include directories
    # we cannot use them as system ones, leads to compilation errors
    set_target_properties(ngraph_obj PROPERTIES NO_SYSTEM_FROM_IMPORTED ON)
endif()

target_compile_definitions(ngraph_obj PRIVATE IMPLEMENT_OPENVINO_API)

ie_faster_build(ngraph_obj
    UNITY
    PCH PRIVATE "src/precomp.hpp")

addVersionDefines(src/version.cpp CI_BUILD_NUMBER)

target_link_libraries(ngraph_obj PRIVATE ngraph::builder ngraph::reference openvino::util
                                         openvino::pugixml ov_shape_inference openvino::core::dev)

ie_mark_target_as_cc(ngraph_obj)

ov_ncc_naming_style(FOR_TARGET ngraph_obj
                    SOURCE_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/include")

add_clang_format_target(ngraph_clang FOR_TARGETS ngraph_obj)

if(NOT BUILD_SHARED_LIBS)
    target_compile_definitions(ngraph_obj PUBLIC OPENVINO_STATIC_LIBRARY)
endif()

if(CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
    # ngraph is linked against ngraph_builders, ngraph_reference, ov_shape_inference static libraries
    # which include ngraph headers with dllimport attribute. Linker complains about it
    # but no way to fix this: linking with no attribute defaults to dllexport and we have
    # multiple defitions for ngraph symbols.
    #
    # The possible way is to use object libraries for ngraph_builders, ngraph_reference
    # but it's not convinient since these libraries are exported from build tree
    # and it's better to use them as static libraries in 3rd party projects
    if(BUILD_SHARED_LIBS)
        set(link_type PRIVATE)
    else()
        set(link_type PUBLIC)
    endif()

    target_link_options(ngraph_obj ${link_type} "/IGNORE:4217,4286")
endif()

# some sources are located in ngraph, while headers are in inference_engine_transformations
file(GLOB_RECURSE smart_reshape_srcs ${CMAKE_CURRENT_SOURCE_DIR}/src/pass/smart_reshape/*.cpp)
file(GLOB_RECURSE rt_info_srcs ${CMAKE_CURRENT_SOURCE_DIR}/src/pass/rt_info/*.cpp)
set_source_files_properties("${CMAKE_CURRENT_SOURCE_DIR}/src/pass/convert_precision.cpp"
                            "${CMAKE_CURRENT_SOURCE_DIR}/src/pass/convert_fp32_to_fp16.cpp"
                            "${CMAKE_CURRENT_SOURCE_DIR}/src/pass/fix_rt_info.cpp"
                            "${CMAKE_CURRENT_SOURCE_DIR}/src/pass/init_node_info.cpp"
                            "${CMAKE_CURRENT_SOURCE_DIR}/src/pass/serialize.cpp"
                            "${CMAKE_CURRENT_SOURCE_DIR}/src/op/type_relaxed.cpp"
                            "${CMAKE_CURRENT_SOURCE_DIR}/src/preprocess/preprocess_steps_impl.cpp"
                            "${CMAKE_CURRENT_SOURCE_DIR}/src/model.cpp" # for SmartReshape
                            ${smart_reshape_srcs} ${rt_info_srcs}
        PROPERTIES INCLUDE_DIRECTORIES $<TARGET_PROPERTY:inference_engine_transformations,INTERFACE_INCLUDE_DIRECTORIES>)

# Defines macro in C++ to load backend plugin
target_include_directories(ngraph_obj PUBLIC $<BUILD_INTERFACE:${OV_CORE_INCLUDE_PATH}>
                                      PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src)

add_library(ngraph INTERFACE)
target_link_libraries(ngraph INTERFACE openvino::runtime)
# Add an alias so that library can be used inside the build tree, e.g. when testing
add_library(openvino::core ALIAS ngraph)

target_include_directories(ngraph INTERFACE $<BUILD_INTERFACE:${OV_CORE_INCLUDE_PATH}>)

#-----------------------------------------------------------------------------------------------
# Installation logic...
#-----------------------------------------------------------------------------------------------

install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/include/
        DESTINATION ${OV_CPACK_INCLUDEDIR}
        COMPONENT ${OV_CPACK_COMP_CORE_DEV}
        FILES_MATCHING
            PATTERN "*.hpp"
            PATTERN "*.h")

configure_package_config_file(${OpenVINO_SOURCE_DIR}/cmake/templates/ngraphConfig.cmake.in
    ${CMAKE_BINARY_DIR}/ngraphConfig.cmake
    INSTALL_DESTINATION ${OV_CPACK_NGRAPH_CMAKEDIR})

write_basic_package_version_file(${CMAKE_BINARY_DIR}/ngraphConfigVersion.cmake
    VERSION ${OpenVINO_VERSION_MAJOR}.${OpenVINO_VERSION_MINOR}.${OpenVINO_VERSION_PATCH}
    COMPATIBILITY SameMajorVersion)

install(FILES ${CMAKE_BINARY_DIR}/ngraphConfig.cmake
              ${CMAKE_BINARY_DIR}/ngraphConfigVersion.cmake
        DESTINATION ${OV_CPACK_NGRAPH_CMAKEDIR}
        COMPONENT ${OV_CPACK_COMP_CORE_DEV})
