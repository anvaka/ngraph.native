#pragma once

#include "primitives.h"

class CBody
{
public:
    void set_position(const Vector3& pos);

    void set_mass(double mass);

    void push_springs(int indxVertex);

    void set_force(const Vector3& force);

    void set_velocity(const Vector3& velocity);

    double get_mass() const;

    int get_incoming_links() const;

    const Vector3& get_position() const;

    const std::vector<int>& get_springs() const;

    const Vector3& get_force() const;

    void reset_force();

    const Vector3& get_velocity() const;

    void reset_velocity();

private:
    double m_dMass = 1.0;

    Vector3 m_Position;
    Vector3 m_Force;
    Vector3 m_Velocity;

    int m_nIncomingLinks = 0;
    std::vector<int> m_nSprings;
};
