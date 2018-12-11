CMAKE_MINIMUM_REQUIRED(VERSION 3.6 FATAL_ERROR)

PROJECT(RT1W)

SET(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/")
SET(CMAKE_EXPORT_COMPILE_COMMANDS ON)

SET(WARNING_FLAGS
  ""
  "all"
  "cast-align"
  "conversion"
  "extra"
  "float-equal"
  "format=2"
  "missing-declarations"
  "missing-noreturn"
  "missing-prototypes"
  "non-virtual-dtor"
  "null-dereference"
  "overloaded-virtual"
  "shadow"
  "sign-conversion"
  "unreachable-code"
  "unused"
)
SET(NO_WARNING_FLAGS
  ""
  "c++98-compat"
  "old-style-cast"
  "pragma-once-outside-header"
  "weak-vtables"
)
STRING(REGEX REPLACE ";" " -W" WARNING_FLAGS "${WARNING_FLAGS}")
STRING(REGEX REPLACE ";" " -Wno-" NO_WARNING_FLAGS "${NO_WARNING_FLAGS}")
SET(WARNING_FLAGS "-pedantic ${WARNING_FLAGS} ${NO_WARNING_FLAGS}")

SET(CMAKE_C_STANDARD 11)
SET(CMAKE_C_STANDARD_REQUIRED YES)
SET(CMAKE_C_EXTENSIONS OFF)

SET(CMAKE_CXX_STANDARD 14)
SET(CMAKE_CXX_STANDARD_REQUIRED YES)
SET(CMAKE_CXX_EXTENSIONS OFF)

SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${WARNING_FLAGS}")
SET(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -g -O0")
SET(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} -O2")

SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${WARNING_FLAGS}")
SET(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -g -O0")
SET(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O2")

# External dependencies

## Lib PNG
FIND_PACKAGE(PNG REQUIRED)

# RT1W

ADD_LIBRARY(rt1w
  src/arena.cpp
  src/bvh.cpp
  src/camera.cpp
  src/context.cpp
  src/event.cpp
  src/error.cpp
  src/geometry.cpp
  src/image.cpp
  src/imageio.c
  src/integrator.cpp
  src/light.cpp
  src/material.cpp
  src/mesh.cpp
  src/params.cpp
  src/primitive.cpp
  src/ray.cpp
  src/rng.cpp
  src/sampler.cpp
  src/scene.cpp
  src/shape.cpp
  src/sphere.cpp
  src/sync.c
  src/transform.cpp
  src/texture.cpp
  src/utils.cpp
  src/workq.cpp
  src/value.cpp
  src/tools/loader_obj.cpp
)

TARGET_INCLUDE_DIRECTORIES(rt1w
  PUBLIC
  src
  PRIVATE
  src/thirdparty
  ${PNG_INCLUDE_DIRS}
)

TARGET_LINK_LIBRARIES(rt1w
  PRIVATE
  PNG::PNG
)

ADD_EXECUTABLE(rt1w_exe src/main.cpp)
SET_TARGET_PROPERTIES(rt1w_exe PROPERTIES OUTPUT_NAME rt1w)

TARGET_LINK_LIBRARIES(rt1w_exe
  PRIVATE
  rt1w
)

# Test

ENABLE_TESTING()
ADD_SUBDIRECTORY(test)

# clang-tidy

FIND_PROGRAM(CLANG_TIDY clang-tidy)
IF (CLANG_TIDY)
  SET_TARGET_PROPERTIES(rt1w
    PROPERTIES CXX_CLANG_TIDY ${CLANG_TIDY})
ENDIF ()
