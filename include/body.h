#pragma once

#include "primitives.h"

class CBody
{
public:
    inline void set_position(const Vector3& pos);

    inline void set_mass(double mass);

    inline void push_springs(int indxVertex);

    inline void set_force(const Vector3& force);

    inline void set_velocity(const Vector3& velocity);

    inline double get_mass() const;

    inline int get_incoming_links() const;

    inline const Vector3& get_position() const;

    inline const std::vector<int>& get_springs() const;

    inline const Vector3& get_force() const;

    inline void reset_force();

    inline const Vector3& get_velocity() const;

    inline void reset_velocity();

private:
    double m_dMass = 1.0;

    Vector3 m_Position;
    Vector3 m_Force;
    Vector3 m_Velocity;

    int m_nIncomingLinks = 0;
    std::vector<int> m_nSprings;
};

#include "body.inl"
