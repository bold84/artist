
# 
# External dependencies
# 

message("CMAKE_MODULE_PATH: ${CMAKE_MODULE_PATH}")

# find_package(THIRDPARTY REQUIRED)
find_package(harfbuzz CONFIG REQUIRED)
find_package(Fontconfig REQUIRED)
find_package(WebP CONFIG REQUIRED)
find_package(expat CONFIG REQUIRED)
find_package(JPEG REQUIRED)
find_package(OpenGL REQUIRED)
find_package(skia CONFIG REQUIRED)

find_package(libunibreak CONFIG REQUIRED)

#find_package(PkgConfig REQUIRED)
#pkg_check_modules(LIBUNIBREAK REQUIRED libunibreak)

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

set(PLATFORM_SPECIFIC_HEADERS)
if(CMAKE_SYSTEM_NAME STREQUAL "Darwin")
elseif(CMAKE_SYSTEM_NAME STREQUAL "Windows")
    list(APPEND PLATFORM_SPECIFIC_HEADERS
        ${CMAKE_CURRENT_SOURCE_DIR}/impl/skia/opaque.hpp
        ${CMAKE_CURRENT_SOURCE_DIR}/impl/skia/detail/harfbuzz.hpp
    )
elseif(CMAKE_SYSTEM_NAME STREQUAL "Linux")
elseif(CMAKE_SYSTEM_NAME STREQUAL "iOS")
elseif(CMAKE_SYSTEM_NAME STREQUAL "Android")
elseif(CMAKE_SYSTEM_NAME STREQUAL "Emscripten")
endif()

set(headers
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
    ${include_path}/artist/detail/canvas_impl.hpp
    
    ${PLATFORM_SPECIFIC_HEADERS}
    
)


set(PLATFORM_SPECIFIC_SOURCES)
if(CMAKE_SYSTEM_NAME STREQUAL "Darwin")
    list(APPEND PLATFORM_SPECIFIC_SOURCES
        ${SKIA_INCLUDE_DIR}/tools/sk_app/MetalWindowContext.mm
        ${SKIA_INCLUDE_DIR}/tools/sk_app/mac/MetalWindowContext_mac.mm
        
        # ${SKIA_INCLUDE_DIR}/tools/sk_app/mac/GLWindowContext_mac.mm
        # ${SKIA_INCLUDE_DIR}/tools/sk_app/mac/RasterWindowContext_mac.mm

        ${SKIA_INCLUDE_DIR}/tools/sk_app/mac/Window_mac.mm
        #${SKIA_INCLUDE_DIR}/tools/sk_app/mac/main_mac.mm
    )
elseif(CMAKE_SYSTEM_NAME STREQUAL "Windows")
    list(APPEND PLATFORM_SPECIFIC_SOURCES
        ${SKIA_INCLUDE_DIR}/tools/sk_app/win/D3D12WindowContext_win.cpp
        ${SKIA_INCLUDE_DIR}/tools/sk_app/win/RasterWindowContext_win.cpp
        ${SKIA_INCLUDE_DIR}/tools/sk_app/win/GLWindowContext_win.cpp
        ${SKIA_INCLUDE_DIR}/tools/sk_app/GLWindowContext.cpp
        
        ${SKIA_INCLUDE_DIR}/tools/sk_app/win/Window_win.cpp
        #${SKIA_INCLUDE_DIR}/tools/sk_app/win/main_win.cpp
        
        ${SKIA_INCLUDE_DIR}/tools/gpu/d3d/D3DTestUtils.cpp
        ${SKIA_INCLUDE_DIR}/src/gpu/ganesh/gl/GrGlUtil.cpp
        ${SKIA_INCLUDE_DIR}/src/core/SkUtils.cpp
        ${SKIA_INCLUDE_DIR}/src/core/SkIDChangeListener.cpp
        ${SKIA_INCLUDE_DIR}/src/core/SkUnPreMultiply.cpp
        ${SKIA_INCLUDE_DIR}/src/core/SkMatrix.cpp
        ${SKIA_INCLUDE_DIR}/src/core/SkString.cpp
        ${SKIA_INCLUDE_DIR}/src/utils/win/SkWGL_win.cpp
        
        ${CMAKE_CURRENT_SOURCE_DIR}/host/windows/skia_app.cpp
        
    )
elseif(CMAKE_SYSTEM_NAME STREQUAL "Linux")
elseif(CMAKE_SYSTEM_NAME STREQUAL "iOS")
elseif(CMAKE_SYSTEM_NAME STREQUAL "Android")
elseif(CMAKE_SYSTEM_NAME STREQUAL "Emscripten")
endif()

set(sources

    ${source_path}/artist/rect.cpp
    ${source_path}/artist/resources.cpp
    ${source_path}/artist/svg_path.cpp
    
    ${CMAKE_CURRENT_SOURCE_DIR}/impl/skia/canvas.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/impl/skia/image.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/impl/skia/font.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/impl/skia/text_layout.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/impl/skia/path.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/impl/skia/detail/harfbuzz.cpp
    
    ${SKIA_INCLUDE_DIR}/tools/sk_app/Window.cpp
    ${SKIA_INCLUDE_DIR}/tools/sk_app/WindowContext.cpp
    
    ${PLATFORM_SPECIFIC_SOURCES}
    
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
    ${headers}
    ${sources}
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

# if (IPO_SUPPORTED AND CMAKE_BUILD_TYPE STREQUAL "Release")
#    message(STATUS "Enabling LTO for artist")
#    set_target_properties(artist PROPERTIES INTERPROCEDURAL_OPTIMIZATION TRUE)
# endif()

set_target_properties(${target}
    PROPERTIES
    ${DEFAULT_PROJECT_OPTIONS}
    INSTALL_RPATH "${LIBRARY_INSTALL_RPATH}"
    FOLDER "${IDE_FOLDER}"
    VERSION "${META_VERSION}"
    SOVERSION "${META_VERSION_MAJOR}"
    #INTERPROCEDURAL_OPTIMIZATION $<$<CONFIG:Debug>:false_string,true_string>
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
    
    ${JPEG_INCLUDE_DIR}

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

set(PLATFORM_SPECIFIC_LIBRARIES)
if(CMAKE_SYSTEM_NAME STREQUAL "Darwin")
    list(APPEND PLATFORM_SPECIFIC_LIBRARIES
        #"-framework Foundation"
        #"-framework AppKit"
        "-framework QuartzCore"
        "-framework ApplicationServices"
        "-framework Metal"
        "-framework Cocoa"
    )
elseif(CMAKE_SYSTEM_NAME STREQUAL "Windows")
    list(APPEND PLATFORM_SPECIFIC_LIBRARIES
        "shcore" # for SetProcessDpiAwareness
    )
elseif(CMAKE_SYSTEM_NAME STREQUAL "Linux")
elseif(CMAKE_SYSTEM_NAME STREQUAL "iOS")
elseif(CMAKE_SYSTEM_NAME STREQUAL "Android")
elseif(CMAKE_SYSTEM_NAME STREQUAL "Emscripten")
endif()

target_link_libraries(${target}
    PRIVATE
    skia::skia
    harfbuzz::harfbuzz
    ${Fontconfig_LIBRARIES}
    
    WebP::webp 
    WebP::webpdemux 
    WebP::libwebpmux 
    WebP::webpdecoder
    expat::expat
    ${JPEG_LIBRARIES}
    ${OPENGL_LIBRARIES}
    
    
    libunibreak::libunibreak
    
    ${PLATFORM_SPECIFIC_LIBRARIES}
    
    PUBLIC
    ${DEFAULT_LIBRARIES}

    INTERFACE
)

# 
# Compile definitions
# 

set(ARTIST_COMPILE_DEFINITIONS)
if(CMAKE_SYSTEM_NAME STREQUAL "Darwin")
    list(APPEND ARTIST_COMPILE_DEFINITIONS
        SK_BUILD_FOR_MAC
    )
elseif(CMAKE_SYSTEM_NAME STREQUAL "Windows")
    list(APPEND ARTIST_COMPILE_DEFINITIONS
        WIN32_LEAN_AND_MEAN
        NOMINMAX
        _UNICODE
        SK_BUILD_FOR_WIN
    )
elseif(CMAKE_SYSTEM_NAME STREQUAL "Linux")
    list(APPEND ARTIST_COMPILE_DEFINITIONS
        SK_BUILD_FOR_UNIX
    )
elseif(CMAKE_SYSTEM_NAME STREQUAL "iOS")
    list(APPEND ARTIST_COMPILE_DEFINITIONS
        SK_BUILD_FOR_IOS
    )
elseif(CMAKE_SYSTEM_NAME STREQUAL "Android")
    list(APPEND ARTIST_COMPILE_DEFINITIONS
        SK_BUILD_FOR_ANDROID
    )
elseif(CMAKE_SYSTEM_NAME STREQUAL "Emscripten")
endif()

target_compile_definitions(${target}
    PRIVATE

    PUBLIC
    ${ARTIST_COMPILE_DEFINITIONS}
    ARTIST_DEBUG=$<$<CONFIG:Debug>:true_string,false_string>
    
    $<$<NOT:$<BOOL:${BUILD_SHARED_LIBS}>>:${target_id}_STATIC_DEFINE>
    
    ${DEFAULT_COMPILE_DEFINITIONS}
    ARTIST_SKIA

    
    
    INTERFACE
)


# 
# Compile options
# 

set(ARTIST_COMPILE_OPTIONS)
if(CMAKE_CXX_COMPILER_ID MATCHES "GNU" OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    list(APPEND ARTIST_COMPILE_OPTIONS 
        "-ftemplate-backtrace-limit=0"
        #$<$<CXX_COMPILER_ID:AppleClang>:-fobjc-arc>
        
        "-Wno-unused-parameter" #temporary
    )
elseif(CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
    list(APPEND ARTIST_COMPILE_OPTIONS
        "/wd4244"
        "/wd4305"
        "/wd4996"
        "/wd4267"
        "/wd4018"
        "/wd5030"
        "/wd4201"
        "/wd4100"
        "/utf-8"
    )
endif()

target_compile_options(${target}
    PRIVATE
        ${DEFAULT_COMPILE_OPTIONS}
        ${ARTIST_COMPILE_OPTIONS}
        ${Fontconfig_COMPILE_OPTIONS}
    
    PUBLIC
    

    INTERFACE
)


# 
# Linker options
# 
target_link_options(${target}
    PRIVATE

    PUBLIC
    ${DEFAULT_LINKER_OPTIONS}

    INTERFACE
)

target_link_directories(${target}
  PRIVATE
    # TODO: must me corrected
    ${LIBUNIBREAK_LIBDIR}

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
