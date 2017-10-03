#pragma once

#include <cmath>

enum coord {
    X = 0,
    Y = 1,
    Z = 2,
    num = 3
};

struct Vector3
{
    inline Vector3()
    {
        reset();
    }

    inline Vector3(const double* value)
    {
        m_Coord[X] = value[X];
        m_Coord[Y] = value[Y];
        m_Coord[Z] = value[Z];
    }

    inline Vector3(const Vector3& rhs)
    {
        *this = rhs;
    };

    inline Vector3(Vector3&& rhs)
    {
        *this = rhs;
    };

    inline Vector3(double s)
        : Vector3(s, s, s)
    {
    };

    inline Vector3(double _x, double _y, double _z)
    {
        m_Coord[X] = _x;
        m_Coord[Y] = _y; 
        m_Coord[Z] = _z;
    };

public:
    inline bool is_initialized() const
    {
        return !(m_Coord[X] == 0.
            && m_Coord[Y] == 0.
            && m_Coord[Z] == 0.);
    }

    inline void reset()
    {
        *this = Vector3{ 0. };
    }

    inline bool sameAs(const Vector3 &other) const
    {
        const Vector3 diff = *this - (other);

        return std::abs(diff[X]) < 1e-8
            && std::abs(diff[Y]) < 1e-8
            && std::abs(diff[Z]) < 1e-8;
    }

    inline void is_great(const Vector3 &other, bool* result) const
    {
        result[X] = m_Coord[X] > other.m_Coord[X];
        result[Y] = m_Coord[Y] > other.m_Coord[Y];
        result[Z] = m_Coord[Z] > other.m_Coord[Z];
    }

    inline void is_less(const Vector3 &other, bool* result) const
    {
        result[X] = m_Coord[X] < other.m_Coord[X];
        result[Y] = m_Coord[Y] < other.m_Coord[Y];
        result[Z] = m_Coord[Z] < other.m_Coord[Z];
    }
public:
    inline double summ_elem() const
    {
        return m_Coord[X] + m_Coord[Y] + m_Coord[Z];
    }
public:
    inline double operator[](const int pos) const
    {
        return m_Coord[pos];
    }

    inline Vector3& operator=(const Vector3 & rhs)
    {
        m_Coord[X] = rhs.m_Coord[X];
        m_Coord[Y] = rhs.m_Coord[Y];
        m_Coord[Z] = rhs.m_Coord[Z];
        return *this;
    }

    inline Vector3& operator=(Vector3&& rhs)
    {
        m_Coord[X] = rhs.m_Coord[X];
        m_Coord[Y] = rhs.m_Coord[Y];
        m_Coord[Z] = rhs.m_Coord[Z];
        return *this;
    }

    inline Vector3 operator+(const Vector3& add) const
    {
        return Vector3{
            m_Coord[X] + add.m_Coord[X],
            m_Coord[Y] + add.m_Coord[Y],
            m_Coord[Z] + add.m_Coord[Z],
        };
    }

    inline Vector3 operator+(Vector3&& add) const
    {
        return Vector3{
            m_Coord[X] + add.m_Coord[X],
            m_Coord[Y] + add.m_Coord[Y],
            m_Coord[Z] + add.m_Coord[Z],
        };
    }

    inline Vector3 operator+(const double add) const
    {
        const Vector3 scalar{ add };
        return operator+(scalar);
    }

    inline Vector3& operator+=(const Vector3& add)
    {
        *this = operator+(add);
        return *this;
    }

    inline Vector3& operator+=(Vector3&& add)
    {
        *this = operator+(add);
        return *this;
    }

    inline Vector3& operator+=(const double add)
    {
        *this = operator+(add);
        return *this;
    }

    inline Vector3 operator-(const Vector3& sub) const
    {
        return Vector3{
            m_Coord[X] - sub.m_Coord[X],
            m_Coord[Y] - sub.m_Coord[Y],
            m_Coord[Z] - sub.m_Coord[Z],
        };
    }

    inline Vector3 operator-(Vector3&& sub) const
    {
        return Vector3{
            m_Coord[X] - sub.m_Coord[X],
            m_Coord[Y] - sub.m_Coord[Y],
            m_Coord[Z] - sub.m_Coord[Z],
        };
    }

    inline Vector3 operator-(const double sub) const
    {
        const Vector3 scalar{ sub };
        return operator-(scalar);
    }

    inline Vector3& operator-=(const Vector3& sub)
    {
        *this = operator-(sub);
        return *this;
    }

    inline Vector3& operator-=(const double sub)
    {
        *this = operator-(sub);
        return *this;
    }

    inline Vector3& operator-=(Vector3&& sub)
    {
        *this = operator-(sub);
        return *this;
    }

    inline Vector3 operator*(const Vector3& mul) const
    {
        return Vector3{
            m_Coord[X] * mul.m_Coord[X],
            m_Coord[Y] * mul.m_Coord[Y],
            m_Coord[Z] * mul.m_Coord[Z],
        };
    }

    inline Vector3 operator*(Vector3&& mul) const
    {
        return Vector3{
            m_Coord[X] * mul.m_Coord[X],
            m_Coord[Y] * mul.m_Coord[Y],
            m_Coord[Z] * mul.m_Coord[Z],
        };
    }

    inline Vector3& operator*=(const Vector3& mul)
    {
        *this = operator*(mul);
        return *this;
    }

    inline Vector3& operator*=(Vector3&& mul)
    {
        *this = operator*(mul);
        return *this;
    }

    inline Vector3 operator*(const double mul) const
    {
        const Vector3 scalar{ mul };
        return operator*(scalar);
    }

    inline Vector3& operator*=(const double mul)
    {
        *this = operator*(mul);
        return *this;
    }

    inline Vector3 operator/(const Vector3& div) const
    {
        return Vector3{
            m_Coord[X] / div.m_Coord[X],
            m_Coord[Y] / div.m_Coord[Y],
            m_Coord[Z] / div.m_Coord[Z],
        };
    }

    inline Vector3 operator/(Vector3&& div) const
    {
        return Vector3{ 
            m_Coord[X] / div.m_Coord[X],
            m_Coord[Y] / div.m_Coord[Y],
            m_Coord[Z] / div.m_Coord[Z],
        };
    }

    inline Vector3& operator/=(const Vector3& div)
    {
        *this = operator/(div);
        return *this;
    }
    inline Vector3& operator/=(Vector3&& div)
    {
        *this = operator/(div);
        return *this;
    }

    inline Vector3 operator/(const double div) const
    {
        const Vector3 scalar{ div };
        return operator/(scalar);
    }

    inline Vector3& operator/=(const double div)
    {
        *this = operator/(div);
        return *this;
    }
public:
    double m_Coord[coord::num];
};