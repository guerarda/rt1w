#include "catch.hpp"

#include "geometry.hpp"

TEMPLATE_TEST_CASE("Bounds", "", int32_t, float, double)
{
    auto a = Bounds3<TestType>();
    auto b = Bounds3<TestType>{ { -1, -1, -1 }, { 1, 1, 1 } };

    auto u = Union(a, b);

    REQUIRE(u.lo.x == Approx(b.lo.x));
    REQUIRE(u.hi.x == Approx(b.hi.x));
}

TEMPLATE_TEST_CASE("Vector2", "", int32_t, uint32_t, float, double)
{
    auto a = Vector2<TestType>();
    auto b = Vector2<TestType>{ 1, 2 };

    auto u = a + b;

    REQUIRE(u.x == Approx(b.x));
}

TEMPLATE_TEST_CASE("Vector3", "", int32_t, uint32_t, float, double)
{
    auto a = Vector3<TestType>();
    auto b = Vector3<TestType>{ 1, 2, 3 };

    auto u = a + b;

    REQUIRE(u.x == Approx(b.x));
    REQUIRE(u.y == Approx(b.y));
    REQUIRE(u.z == Approx(b.z));
}
