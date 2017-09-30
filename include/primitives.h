#pragma once

#include <cmath>

struct Vector3
{
    Vector3()
    {
        reset();
    }

    Vector3(double _x, double _y, double _z)
        : x(_x), y(_y), z(_z)
    {
    };

    bool is_initialized() const
    {
        return !(x == 0.0 && y == 0.0 && z == 0.0);
    }

    void reset()
    {
        x = y = z = 0.0;
    }

    bool sameAs(const Vector3 &other) const
    {
        double dx = std::abs(x - other.x);
        double dy = std::abs(y - other.y);
        double dz = std::abs(z - other.z);

        return (dx < 1e-8 && dy < 1e-8 && dz < 1e-8);
    }

public:
    double x, y, z;
};