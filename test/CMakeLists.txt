
add_executable(rt1w_test test.cpp)

target_sources(rt1w_test
  PRIVATE
    efloat.cpp
    camera.cpp
    geometry.cpp
    ray-shape.cpp
    sampling.cpp
)

target_link_libraries(rt1w_test
  PRIVATE
    rt1w
)

include(CTest)
include(Catch)

catch_discover_tests(rt1w_test)
