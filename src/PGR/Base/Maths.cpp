#include "Maths.h"

namespace PGR {

    Vec2 operator+ (const Vec2& left, const Vec2& right) {
        return Vec2{ left.X + right.X, left.Y + right.Y };
    }
    Vec2 operator- (const Vec2& left, const Vec2& right) {
        return Vec2{ left.X - right.X, left.Y - right.Y };
    }

    Vec3 operator+ (const Vec3& left, const Vec3& right) {
        return Vec3{ left.X + right.X, left.Y + right.Y, left.Z + right.Z };
    }
    Vec3 operator- (const Vec3& left, const Vec3& right) {
        return left + (-1.0f * right);
    }
    Vec3 operator* (const float left, const Vec3& right) {
        return Vec3{ left * right.X, left * right.Y, left * right.Z };
    }
    Vec3 operator* (const Vec3& left, const float right) {
        return right * left;
    }
    Vec3 operator* (const Vec3& left, const Vec3& right) {
        return { left.X * right.X, left.Y * right.Y, left.Z * right.Z };
    }
    Vec3 operator/ (const Vec3& left, const float right) {
        return left * (1.0f / right);
    }
    Vec3 operator/ (const Vec3& left, const Vec3& right) {
        return left * Vec3{ 1.0f / right.X, 1.0f / right.Y, 1.0f / right.Z };
    }
    Vec3& operator*= (Vec3& left, const float right) {
        left = left * right;
        return left;
    }
    Vec3& operator/= (Vec3& left, const float right) {
        left = left / right;
        return left;
    }
    Vec3& operator+= (Vec3& left, const Vec3& right) {
        left = left + right;
        return left;
    }

    Vec4 operator+ (const Vec4& left, const Vec4& right) {
        return Vec4{ left.X + right.X, left.Y + right.Y, left.Z + right.Z, left.W + right.W };
    }
    Vec4 operator- (const Vec4& left, const Vec4& right) {
        return Vec4{ left.X - right.X, left.Y - right.Y, left.Z - right.Z, left.W - right.W };
    }
    Vec4 operator* (const float left, const Vec4& right) {
        return Vec4{ left * right.X, left * right.Y, left * right.Z, left * right.W };
    }
    Vec4 operator* (const Vec4& left, const float right) {
        return right * left;
    }
    Vec4 operator* (const Vec4& left, const Vec4& right) {
        return { left.X * right.X, left.Y * right.Y, left.Z * right.Z, left.W * right.W };
    }
    Vec4 operator/ (const Vec4& left, const float right) {
        return left * (1.0f / right);
    }

    float Clamp(const float val, const float min, const float max) {
        return Max(min, Min(val, max));
    }

    Vec3 Clamp(const Vec3& vec, const float min, const float max) {
        return Vec3{
            Max(min, Min(vec.X, max)),
            Max(min, Min(vec.Y, max)),
            Max(min, Min(vec.Z, max))
        };
    }

    Vec4 Clamp(const Vec4& vec, const float min, const float max) {
        return Vec4{
            Max(min, Min(vec.X, max)),
            Max(min, Min(vec.Y, max)),
            Max(min, Min(vec.Z, max)),
            Max(min, Min(vec.W, max))
        };
    }

    float Max(const float right, const float left) {
        return std::max<float>(right, left);
    }

    float Min(const float right, const float left) {
        return std::min<float>(right, left);
    }

    unsigned char Float2UChar(const float f) {
        return static_cast<unsigned char>(f * 255.0f + 0.5f);
    }

    float UChar2Float(const unsigned char c) {
        return (float)c / 255.0f;
    }

    float linear(float t, float st, float et, float sv, float ev) {
        if (st == et)
            return sv;
        return sv + (t - st) / (et - st) * (ev - sv);
    }

    float randf(const std::vector<float>& seeds, float min, float max) {
        uint64_t seed = 0;
        for (const auto& val : seeds)
        {
            const uint32_t* float_as_uint = reinterpret_cast<const uint32_t*>(&val);
            seed = seed * 31 + *float_as_uint;
        }
        const uint32_t* min_as_uint = reinterpret_cast<const uint32_t*>(&min);
        const uint32_t* max_as_uint = reinterpret_cast<const uint32_t*>(&max);
        seed = seed * 31 + *min_as_uint;
        seed = seed * 31 + *max_as_uint;

        std::mt19937 rng_engine(static_cast<unsigned int>(seed));

        std::uniform_real_distribution<float> dist(min, max);

        return dist(rng_engine);
    }

    float randf(float seed, float min, float max) {
        std::mt19937 rng_engine(static_cast<unsigned int>(seed));

        std::uniform_real_distribution<float> dist(min, max);

        return dist(rng_engine);
    }

    std::vector<float> getSeeds(const std::vector<float> seeds, int count) {
        std::vector<float> s;
        for (size_t i = 0; i < seeds.size(); i++) {
            float r = seeds[i];
            for (int j = 0; j < count; j++) {
                r = randf(r * 2.0f, 0.0f, 1007.0f);
            }
            s.push_back(r);
        }
        return s;
    }

    Vec2 rotatePoint(float x, float y, float r, float deg) {
        const float rad = deg * PI_OVER_180;
        const float cos_rad = cos(rad);
        const float sin_rad = sin(rad);

        return Vec2{
            x + r * cos_rad,
            y + r * sin_rad
        };
    }

}
