cmake_minimum_required(VERSION 3.6 FATAL_ERROR)

project(RT1W)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/")

# Options
option(RT1W_WITH_OIDN "Build with Intel's OpenImageDenoise Library" OFF)
option(RT1W_WITH_TESTS "Creates a target for running the suite of unit tests" OFF)
option(RT1W_WITH_METAL "Build with Metal Acceleration" OFF)

# External dependencies
## Lib PNG
find_package(PNG REQUIRED)

## Open Image Denoise
if (RT1W_WITH_OIDN)
  message("Building with OpenImageDenoise")
  find_package(OpenImageDenoise REQUIRED)
endif()

## Metal
if(RT1W_WITH_METAL)
  message("Building with Metal Acceleration")
endif()

# RT1W
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(WARNING_FLAGS
  -pedantic
  -Wall
  -Wcast-align
  -Wconversion
  -Wextra
  -Wfloat-equal
  -Wformat=2
  -Wmissing-declarations
  -Wmissing-noreturn
  -Wmissing-prototypes
  -Wnon-virtual-dtor
  -Wnull-dereference
  -Woverloaded-virtual
  -Wshadow
  -Wsign-conversion
  -Wunreachable-code
  -Wunused
)
set(NOWARNING_FLAGS
  -Wno-old-style-cast
  $<$<CXX_COMPILER_ID:AppleClang>:
    -Wno-c++98-compat
    -Wno-pragma-once-outside-header
    -Wno-weak-vtables>

  $<$<CXX_COMPILER_ID:Clang>:
    -Wno-c++98-compat
    -Wno-pragma-once-outside-header
    -Wno-weak-vtables>

  $<$<CXX_COMPILER_ID:GNU>:
    -Wno-unknown-pragmas
    -Wno-shadow
    -Wno-conversion
    -Wno-class-memaccess>
)

## Standard
set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED YES)
set(CMAKE_C_EXTENSIONS OFF)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED YES)
set(CMAKE_CXX_EXTENSIONS OFF)

## Threads support
find_package(Threads REQUIRED)
set(CMAKE_THREAD_PREFER_PTHREAD TRUE)

## Shapes
add_library(shapes
  OBJECT
    src/shapes/sphere.cpp
    src/shapes/mesh.cpp
)
target_include_directories(shapes
  PRIVATE
    include
    src
)

## Accelerators
add_library(accelerators
  OBJECT
    src/accelerators/bvh.cpp
    src/accelerators/bvhbuilder.cpp
    src/accelerators/qbvh.cpp
)

target_include_directories(accelerators
  PRIVATE
    include
    src
)

## Integrators
add_library(integrators
  OBJECT
    src/integrators/whitted.cpp
    src/integrators/path.cpp
)
target_include_directories(integrators
  PRIVATE
    include
    src
)

## Core
add_library(rt1w
  src/core/accelerator.cpp
  src/core/arena.cpp
  src/core/bxdf.cpp
  src/core/camera.cpp
  src/core/context.cpp
  src/core/denoise.cpp
  src/core/error.cpp
  src/core/event.cpp
  src/core/fresnel.cpp
  src/core/geometry.cpp
  src/core/image.cpp
  src/core/imageio.c
  src/core/integrator.cpp
  src/core/interaction.cpp
  src/core/light.cpp
  src/core/material.cpp
  src/core/params.cpp
  src/core/primitive.cpp
  src/core/ray.cpp
  src/core/rng.cpp
  src/core/sampler.cpp
  src/core/sampling.cpp
  src/core/scene.cpp
  src/core/shape.cpp
  src/core/spectrum.cpp
  src/core/sptr.cpp
  src/core/sync.c
  src/core/task.cpp
  src/core/texture.cpp
  src/core/transform.cpp
  src/core/utils.cpp
  src/core/value.cpp
  src/core/workq.cpp
  src/tools/loader_obj.cpp
  $<TARGET_OBJECTS:accelerators>
  $<TARGET_OBJECTS:shapes>
  $<TARGET_OBJECTS:integrators>
)

## Requirements
target_compile_options(rt1w
  PRIVATE
    ${WARNING_FLAGS}
    ${NOWARNING_FLAGS}
)
target_include_directories(rt1w
  PUBLIC
    include
  PRIVATE
    src
    src/thirdparty
    ${PNG_INCLUDE_DIRS}
)
target_link_libraries(rt1w
  PRIVATE
    PNG::PNG
    Threads::Threads
)

if (RT1W_WITH_OIDN)
  target_compile_definitions(rt1w
    PRIVATE
      RT1W_WITH_OIDN
  )
  target_include_directories(rt1w
    PRIVATE
      ${OPENIMAGEDENOISE_INCLUDE_DIRS}
  )
  target_link_libraries(rt1w
    PRIVATE
      OpenImageDenoise
  )
endif()

## Executable
add_executable(rt1w_exe src/main/rt1w.cpp)
set_target_properties(rt1w_exe PROPERTIES OUTPUT_NAME rt1w)

target_compile_options(rt1w_exe
  PRIVATE
    ${WARNING_FLAGS}
    ${NOWARNING_FLAGS}
)
target_link_libraries(rt1w_exe
  PRIVATE
    rt1w
)

# Tests

if (RT1W_WITH_TESTS)
  enable_testing()

  include(CTest)
  include(Catch)

  add_executable(rt1w_test
    test/test.cpp
    test/accelerator.cpp
    test/camera.cpp
    test/efloat.cpp
    test/geometry.cpp
    test/ray-shape.cpp
    test/sampling.cpp
  )
  target_include_directories(rt1w_test
    PRIVATE
      src
  )
  target_link_libraries(rt1w_test
    PRIVATE
      rt1w
  )
  catch_discover_tests(rt1w_test)
endif()
