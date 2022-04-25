
# 
# External dependencies
# 

message("CMAKE_MODULE_PATH: ${CMAKE_MODULE_PATH}")

# find_package(THIRDPARTY REQUIRED)
find_package(skia CONFIG REQUIRED)
find_package(harfbuzz CONFIG REQUIRED)
#find_package(skia REQUIRED)

# 
# Library name and options
# 

# Target name
set(target artist)

# Exit here if required dependencies are not met
message(STATUS "Lib ${target}")

# Set API export file and macro
string(MAKE_C_IDENTIFIER ${target} target_id)
string(TOUPPER ${target_id} target_id)
set(feature_file         "include/${target}/${target}_features.h")
set(export_file          "include/${target}/${target}_export.h")
set(template_export_file "include/${target}/${target}_api.h")
set(export_macro         "${target_id}_API")


# 
# Sources
# 

# set(include_path "${CMAKE_CURRENT_SOURCE_DIR}/include/${target}")
set(include_path "${CMAKE_CURRENT_SOURCE_DIR}/include")
set(source_path  "${CMAKE_CURRENT_SOURCE_DIR}/src")

set(headers
    ${source_path}/artist/rect.cpp
    ${source_path}/artist/resources.cpp
    ${source_path}/artist/svg_path.cpp
    
    ${CMAKE_CURRENT_SOURCE_DIR}/impl/skia/canvas.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/impl/skia/image.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/impl/skia/font.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/impl/skia/text_layout.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/impl/skia/path.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/impl/skia/detail/harfbuzz.cpp
    
)

set(sources
    ${include_path}/artist/affine_transform.hpp
    ${include_path}/artist/canvas.hpp
    ${include_path}/artist/circle.hpp
    ${include_path}/artist/color.hpp
    ${include_path}/artist/detail
    ${include_path}/artist/font.hpp
    ${include_path}/artist/image.hpp
    ${include_path}/artist/path.hpp
    ${include_path}/artist/point.hpp
    ${include_path}/artist/rect.hpp
    ${include_path}/artist/resources.hpp
    ${include_path}/artist/text_layout.hpp
)

# Group source files
set(header_group "Header Files (API)")
set(source_group "Source Files")
source_group_by_path(${include_path} "\\\\.h$|\\\\.hpp$" 
    ${header_group} ${headers})
source_group_by_path(${source_path}  "\\\\.cpp$|\\\\.c$|\\\\.h$|\\\\.hpp$" 
    ${source_group} ${sources})


# 
# Create library
# 

# Build library
add_library(${target}
    ${sources}
    ${headers}
)

# Create namespaced alias
add_library(${META_PROJECT_NAME}::${target} ALIAS ${target})

# Export library for downstream projects
export(TARGETS ${target} NAMESPACE ${META_PROJECT_NAME}:: FILE ${PROJECT_BINARY_DIR}/cmake/${target}/${target}-export.cmake)

# Create feature detection header
# Compilers: https://cmake.org/cmake/help/v3.1/variable/CMAKE_LANG_COMPILER_ID.html#variable:CMAKE_%3CLANG%3E_COMPILER_ID
# Feature: https://cmake.org/cmake/help/v3.1/prop_gbl/CMAKE_CXX_KNOWN_FEATURES.html

# Check for availability of module; use pre-generated version if not found
# if (WriterCompilerDetectionHeaderFound)
#     write_compiler_detection_header(
#         FILE ${feature_file}
#         PREFIX ${target_id}
#         COMPILERS AppleClang Clang GNU MSVC
#         FEATURES cxx_alignas cxx_alignof cxx_constexpr cxx_final cxx_noexcept cxx_nullptr cxx_sizeof_member cxx_thread_local
#         VERSION 3.2
#     )
# else()
#     file(
#         COPY ${PROJECT_SOURCE_DIR}/source/codegeneration/${target}_features.h
#         DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/include/${target}
#         USE_SOURCE_PERMISSIONS
#     )
# endif()

# Create API export header
generate_export_header(${target}
    EXPORT_FILE_NAME  ${export_file}
    EXPORT_MACRO_NAME ${export_macro}
)

generate_template_export_header(${target}
    ${target_id}
    ${template_export_file}
)


# 
# Project options
# 

set_target_properties(${target}
    PROPERTIES
    ${DEFAULT_PROJECT_OPTIONS}
    INSTALL_RPATH "${LIBRARY_INSTALL_RPATH}"
    FOLDER "${IDE_FOLDER}"
    VERSION "${META_VERSION}"
    SOVERSION "${META_VERSION_MAJOR}"
)


# 
# Include directories
# 

target_include_directories(${target}
    PRIVATE
    ${PROJECT_BINARY_DIR}/source/include
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_CURRENT_BINARY_DIR}/include
    ${CMAKE_CURRENT_SOURCE_DIR}/
    ${CMAKE_CURRENT_SOURCE_DIR}/infra/include

    PUBLIC
    ${DEFAULT_INCLUDE_DIRECTORIES}

    INTERFACE
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}/include>
    $<INSTALL_INTERFACE:include>
)


# 
# Libraries
# 

target_link_libraries(${target}
    PRIVATE
    skia
    harfbuzz
    unibreak
    
    PUBLIC
    ${DEFAULT_LIBRARIES}

    INTERFACE
)

# 
# Compile definitions
# 

target_compile_definitions(${target}
    PRIVATE

    PUBLIC
    
    $<$<PLATFORM_ID:Windows>:WIN32_LEAN_AND_MEAN>
    $<$<PLATFORM_ID:Windows>:NOMINMAX>
    $<$<PLATFORM_ID:Windows>:_UNICODE>
    
    $<$<NOT:$<BOOL:${BUILD_SHARED_LIBS}>>:${target_id}_STATIC_DEFINE>
    
    ${DEFAULT_COMPILE_DEFINITIONS}
    ARTIST_SKIA
    SK_RELEASE
    
    
    INTERFACE
)


# 
# Compile options
# 

target_compile_options(${target}
    PRIVATE

    
    $<$<CXX_COMPILER_ID:MSVC>:/W4 /wd4244 /wd4305 /wd4996 /wd4267 /wd4018 /wd5030 /wd4201 /utf-8>
    
    PUBLIC
    ${DEFAULT_COMPILE_OPTIONS}

    INTERFACE
)


# 
# Linker options
# 

target_link_libraries(${target}
    PRIVATE

    PUBLIC
    ${DEFAULT_LINKER_OPTIONS}

    INTERFACE
)


#
# Target Health
#

perform_health_checks(
    ${target}
    ${sources}
    ${headers}
)


# 
# Deployment
# 

# Library
install(TARGETS ${target}
    EXPORT  "${target}-export"            COMPONENT dev
    RUNTIME DESTINATION ${INSTALL_BIN}    COMPONENT runtime
    LIBRARY DESTINATION ${INSTALL_SHARED} COMPONENT runtime
    ARCHIVE DESTINATION ${INSTALL_LIB}    COMPONENT dev
)

# Header files
install(DIRECTORY
    ${CMAKE_CURRENT_SOURCE_DIR}/include/${target} DESTINATION ${INSTALL_INCLUDE}
    COMPONENT dev
)

# Generated header files
install(DIRECTORY
    ${CMAKE_CURRENT_BINARY_DIR}/include/${target} DESTINATION ${INSTALL_INCLUDE}
    COMPONENT dev
)

# CMake config
install(EXPORT ${target}-export
    NAMESPACE   ${META_PROJECT_NAME}::
    DESTINATION ${INSTALL_CMAKE}/${target}
    COMPONENT   dev
)
