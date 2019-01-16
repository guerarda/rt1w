#include "catch.hpp"

#include "efloat.hpp"
#include "rng.hpp"
#include "utils.hpp"

#include <string>

enum struct EFloatErrorMode { None, Typical, Large, Extreme };

static std::string ErrorModeString(EFloatErrorMode mode)
{
    switch (mode) {
    case EFloatErrorMode::None: return "None";
    case EFloatErrorMode::Typical: return "Typical";
    case EFloatErrorMode::Large: return "Large";
    case EFloatErrorMode::Extreme: return "Extreme";
    }
}

static EFloat RandomEFloat(RNG &rng,
                           EFloatErrorMode mode,
                           float minExp = -6.f,
                           float maxExp = 6.f)
{
    float logu = Lerp(rng.f32(), minExp, maxExp);
    float val = std::powf(10, logu);
    float sign = rng.f32() < .5 ? -1. : 1.;

    if (mode == EFloatErrorMode::Typical) {
        uint32_t ulpError = rng.u32(1024);
        float offset = BitsToFloat(FloatToBits(val) + ulpError);
        float error = std::abs(offset - val);

        return EFloat(sign * val, error);
    }
    if (mode == EFloatErrorMode::Large) {
        uint32_t ulpError = rng.u32(1024 * 1024);
        float offset = BitsToFloat(FloatToBits(val) + ulpError);
        float error = std::abs(offset - val);

        return EFloat(sign * val, error);
    }
    if (mode == EFloatErrorMode::Extreme) {
        float error = (4.f * rng.f32()) * std::abs(val);
        return EFloat(sign * val, error);
    }
    return EFloat(sign * val);
}

static double RandomPrecise(const EFloat &ef, RNG &rng)
{
    switch (rng.u32(3)) {
    case 0: return ef.hi();
    case 1: return ef.lo();
    default: return Clamp(Lerp(rng.f32(), ef.lo(), ef.hi()), ef.lo(), ef.hi());
    }
}

constexpr size_t kIterations = 100000;

TEST_CASE("RandomPrecise", "[efloat]")
{
    size_t n = 0;
    uptr<RNG> rng = RNG::create();

    for (size_t i = 0; i < kIterations; ++i) {
        EFloat ef = RandomEFloat(*rng, (EFloatErrorMode)rng->u32(4));
        double precise = RandomPrecise(ef, *rng);

        if (precise < ef.lo() || precise > ef.hi()) {
            ++n;
        }
    }
    REQUIRE(n == 0);
}

TEST_CASE("EFloat Add", "[efloat]")
{
    uptr<RNG> rng = RNG::create();

    for (int32_t mode = 0; mode < 4; ++mode) {
        DYNAMIC_SECTION(ErrorModeString((EFloatErrorMode)mode))
        {
            size_t n = 0;
            for (size_t i = 0; i < kIterations; ++i) {
                EFloat ef[2] = { RandomEFloat(*rng, (EFloatErrorMode)mode),
                                 RandomEFloat(*rng, (EFloatErrorMode)mode) };
                double p[2] = { RandomPrecise(ef[0], *rng), RandomPrecise(ef[1], *rng) };

                EFloat r = ef[0] + ef[1];
                float pr = (float)(p[0] + p[1]);

                if (pr < r.lo() || pr > r.hi()) {
                    ++n;
                }
            }
            CHECK(n == 0);
        }
    }
}

TEST_CASE("EFloat Sub", "[efloat]")
{
    uptr<RNG> rng = RNG::create();

    for (int32_t mode = 0; mode < 4; ++mode) {
        DYNAMIC_SECTION(ErrorModeString((EFloatErrorMode)mode))
        {
            size_t n = 0;
            for (size_t i = 0; i < kIterations; ++i) {
                EFloat ef[2] = { RandomEFloat(*rng, (EFloatErrorMode)mode),
                                 RandomEFloat(*rng, (EFloatErrorMode)mode) };
                double p[2] = { RandomPrecise(ef[0], *rng), RandomPrecise(ef[1], *rng) };

                EFloat r = ef[0] - ef[1];
                float pr = (float)(p[0] - p[1]);

                if (pr < r.lo() || pr > r.hi()) {
                    ++n;
                }
            }
            CHECK(n == 0);
        }
    }
}

TEST_CASE("EFloat Mul", "[efloat]")
{
    uptr<RNG> rng = RNG::create();

    for (int32_t mode = 0; mode < 4; ++mode) {
        DYNAMIC_SECTION(ErrorModeString((EFloatErrorMode)mode))
        {
            size_t n = 0;
            for (size_t i = 0; i < kIterations; ++i) {
                EFloat ef[2] = { RandomEFloat(*rng, (EFloatErrorMode)mode),
                                 RandomEFloat(*rng, (EFloatErrorMode)mode) };
                double p[2] = { RandomPrecise(ef[0], *rng), RandomPrecise(ef[1], *rng) };

                EFloat r = ef[0] * ef[1];
                float pr = (float)(p[0] * p[1]);

                if (pr < r.lo() || pr > r.hi()) {
                    ++n;
                }
            }
            CHECK(n == 0);
        }
    }
}

TEST_CASE("EFloat Div", "[efloat]")
{
    uptr<RNG> rng = RNG::create();

    for (int32_t mode = 0; mode < 4; ++mode) {
        DYNAMIC_SECTION(ErrorModeString((EFloatErrorMode)mode))
        {
            size_t n = 0;
            for (size_t i = 0; i < kIterations; ++i) {
                EFloat ef[2] = { RandomEFloat(*rng, (EFloatErrorMode)mode),
                                 RandomEFloat(*rng, (EFloatErrorMode)mode) };
                double p[2] = { RandomPrecise(ef[0], *rng), RandomPrecise(ef[1], *rng) };

                if (ef[1].error() >= .5f * ef[1].min()) {
                    continue;
                }
                EFloat r = ef[0] / ef[1];
                float pr = (float)(p[0] / p[1]);

                if (pr < r.lo() || pr > r.hi()) {
                    ++n;
                }
            }
            CHECK(n == 0);
        }
    }
}

TEST_CASE("EFloat Abs", "[efloat]")
{
    uptr<RNG> rng = RNG::create();

    for (int32_t mode = 0; mode < 4; ++mode) {
        DYNAMIC_SECTION(ErrorModeString((EFloatErrorMode)mode))
        {
            size_t n = 0;
            for (size_t i = 0; i < kIterations; ++i) {
                EFloat ef = RandomEFloat(*rng, (EFloatErrorMode)mode);
                double p = RandomPrecise(ef, *rng);

                EFloat r = abs(ef);
                float pr = (float)(std::abs(p));

                if (pr < r.lo() || pr > r.hi()) {
                    ++n;
                }
            }
            CHECK(n == 0);
        }
    }
}

TEST_CASE("EFloat Sqrt", "[efloat]")
{
    uptr<RNG> rng = RNG::create();

    for (int32_t mode = 0; mode < 4; ++mode) {
        DYNAMIC_SECTION(ErrorModeString((EFloatErrorMode)mode))
        {
            size_t n = 0;
            for (size_t i = 0; i < kIterations; ++i) {
                EFloat ef = RandomEFloat(*rng, (EFloatErrorMode)mode);
                double p = RandomPrecise(ef, *rng);

                if (ef.lo() < ef.error() || ef.hi() < ef.error()) {
                    continue;
                }
                EFloat r = sqrt(ef);
                float pr = (float)(std::sqrt(p));

                if (pr < r.lo() || pr > r.hi()) {
                    ++n;
                }
            }
            CHECK(n == 0);
        }
    }
}
