CMAKE_MINIMUM_REQUIRED(VERSION 3.2 FATAL_ERROR)

PROJECT(RT1W)

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
  "padded"
  "pragma-once-outside-header"
  "weak-vtables"
)
STRING(REGEX REPLACE ";" " -W" WARNING_FLAGS "${WARNING_FLAGS}")
STRING(REGEX REPLACE ";" " -Wno-" NO_WARNING_FLAGS "${NO_WARNING_FLAGS}")
SET(WARNING_FLAGS "-pedantic ${WARNING_FLAGS} ${NO_WARNING_FLAGS}")

SET(CMAKE_C_STANDARD 11)
SET(CMAKE_C_STANDARD_REQUIRED YES)
SET(CMAKE_CXX_STANDARD 11)
SET(CMAKE_CXX_STANDARD_REQUIRED YES)

SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${WARNING_FLAGS}")
SET(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -g -O0")
SET(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} -O2")

SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${WARNING_FLAGS}")
SET(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -g -O0")
SET(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O2")

INCLUDE_DIRECTORIES(src/thirdparty)

SET(RT1W_SOURCES
  src/bvh.cpp
  src/camera.cpp
  src/event.cpp
  src/error.cpp
  src/geometry.cpp
  src/imageio.c
  src/material.cpp
  src/params.cpp
  src/primitive.cpp
  src/ray.cpp
  src/scene.cpp
  src/shape.cpp
  src/sphere.cpp
  src/sync.c
  src/texture.cpp
  src/workq.cpp
  src/value.cpp
)

FIND_PACKAGE(PNG REQUIRED)
INCLUDE_DIRECTORIES(${PNG_INCLUDE_DIR})

ADD_EXECUTABLE(rt1w
  src/main.cpp
  ${RT1W_SOURCES})

TARGET_LINK_LIBRARIES(rt1w ${PNG_LIBRARY})

ADD_CUSTOM_TARGET(analyze
  COMMAND make clean
  COMMAND /usr/local/opt/llvm/bin/scan-build
  -k
  -v
  make
)
