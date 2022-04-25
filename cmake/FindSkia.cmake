# CMake module to find the Skia library and include files in a skia build directory.
#
# Provides:
#
#   SKIA_FOUND
#   Skia_INCLUDE_DIR
#   Skia_LIBRARY

SET(Skia_BUILD_DIR "$ENV{Skia_BUILD_DIR}" CACHE PATH "Skia build directory")

FIND_PATH(Skia_INCLUDE_DIR
  core/SkCanvas.h
  HINTS include/skia
)

FIND_LIBRARY(Skia_LIBRARY
  skia
  PATH lib)

# INCLUDE(FindPackageHandleStandardArgs)
# FIND_PACKAGE_HANDLE_STANDARD_ARGS(Skia DEFAULT_MSG Skia_LIBRARY Skia_INCLUDE_DIR)

MARK_AS_ADVANCED(Skia_INCLUDE_DIR Skia_LIBRARY)