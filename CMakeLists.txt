cmake_minimum_required(VERSION 3.6 FATAL_ERROR)

project(RT1W)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/")

# Options
option(RT1W_WITH_OIDN "Build with OpenImageDenoise Library" OFF)

# External dependencies
## Lib PNG
find_package(PNG REQUIRED)

## Open Image Denoise
if (RT1W_WITH_OIDN)
  message("Building with OpenImageDenoise")
  find_package(OpenImageDenoise REQUIRED)
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
  -Wno-c++98-compat
  -Wno-old-style-cast
  -Wno-pragma-once-outside-header
  -Wno-weak-vtables
)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED YES)
set(CMAKE_C_EXTENSIONS OFF)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED YES)
set(CMAKE_CXX_EXTENSIONS OFF)

## Core
add_library(rt1w
  src/arena.cpp
  src/bxdf.cpp
  src/bvh.cpp
  src/camera.cpp
  src/context.cpp
  src/denoise.cpp
  src/event.cpp
  src/error.cpp
  src/fresnel.cpp
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
  src/sampling.cpp
  src/sampler.cpp
  src/scene.cpp
  src/shape.cpp
  src/sphere.cpp
  src/sync.c
  src/task.cpp
  src/transform.cpp
  src/texture.cpp
  src/utils.cpp
  src/workq.cpp
  src/value.cpp
  src/tools/loader_obj.cpp
)

target_compile_options(rt1w
  PRIVATE
    ${WARNING_FLAGS} ${NOWARNING_FLAGS}
)

target_include_directories(rt1w
  PUBLIC
    src
  PRIVATE
    src/thirdparty
    ${PNG_INCLUDE_DIRS}
)
target_link_libraries(rt1w
  PRIVATE
    PNG::PNG
)

if (RT1W_WITH_OIDN)
  target_compile_definitions(rt1w
    PRIVATE
      RT1W_WITH_OIDN
  )
  target_compile_options(rt1w
    PRIVATE
      ${OPENIMAGEDENOISE_INCLUDE_DIRS}
  )
  target_link_libraries(rt1w
    PRIVATE
      OpenImageDenoise
  )
endif()

## Executable
add_executable(rt1w_exe src/main.cpp)
set_target_properties(rt1w_exe PROPERTIES OUTPUT_NAME rt1w)

target_compile_options(rt1w_exe
  PRIVATE
    ${WARNING_FLAGS} ${NOWARNING_FLAGS}
)

target_link_libraries(rt1w_exe
  PRIVATE
  rt1w
)

## Tests
enable_testing()
add_subdirectory(test)
