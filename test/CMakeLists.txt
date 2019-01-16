
SET(RT1W_TESTS
    efloat.cpp
    geometry.cpp
    sampling.cpp
    camera.cpp
)

ADD_EXECUTABLE(rt1w_test
  test.cpp
  ${RT1W_TESTS}
)

TARGET_LINK_LIBRARIES(rt1w_test
  PRIVATE
  rt1w
)

INCLUDE(CTest)
INCLUDE(Catch)

CATCH_DISCOVER_TESTS(rt1w_test)
