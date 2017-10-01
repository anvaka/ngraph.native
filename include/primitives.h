#pragma once

#include <cmath>

struct Vector3
{
    inline Vector3()
    {
        reset();
    }

    inline Vector3(const Vector3& rhs)
        : x(rhs.x), y(rhs.y), z(rhs.z)
    {
    };

    inline Vector3(Vector3&& rhs)
        : x(rhs.x), y(rhs.y), z(rhs.z)
    {
    };

    inline Vector3(double _x, double _y, double _z)
        : x(_x), y(_y), z(_z)
    {
    };


public:
    inline bool is_initialized() const
    {
        return !(x == 0.0 && y == 0.0 && z == 0.0);
    }

    inline void reset()
    {
        x = y = z = 0.0;
    }

    inline bool sameAs(const Vector3 &other) const
    {
        double dx = std::abs(x - other.x);
        double dy = std::abs(y - other.y);
        double dz = std::abs(z - other.z);

        return (dx < 1e-8 && dy < 1e-8 && dz < 1e-8);
    }

public:
    inline Vector3& operator =(const Vector3 & rhs)
    {
        x = rhs.x;
        y = rhs.y;
        z = rhs.z;
        return *this;
    }

    inline Vector3& operator =(Vector3&& rhs)
    {
        x = rhs.x;
        y = rhs.y;
        z = rhs.z;
        return *this;
    }
public:
    double x, y, z;
};