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
  "shadow"
  "unreachable-code"
)
SET(NO_WARNING_FLAGS
  ""
  "c++98-compat"
  "old-style-cast"
  "padded"
  "weak-vtables"
)
STRING(REGEX REPLACE ";" " -W" WARNING_FLAGS "${WARNING_FLAGS}")
STRING(REGEX REPLACE ";" " -Wno-" NO_WARNING_FLAGS "${NO_WARNING_FLAGS}")
SET(WARNING_FLAGS "${WARNING_FLAGS} ${NO_WARNING_FLAGS}")

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

SET(RT1W_SOURCES
  bvh.cpp
  camera.cpp
  event.cpp
  hitablelist.cpp
  material.cpp
  ray.cpp
  sphere.cpp
  sync.c
  vec.cpp
  wqueue.cpp
)

ADD_EXECUTABLE(rt1w
  main.cpp
  ${RT1W_SOURCES})
