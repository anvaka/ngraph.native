#include "stdafx.h"

#include "body.h"

void CBody::set_position(const Vector3 & pos)
{
    m_Position = pos;
}

void CBody::set_mass(double mass)
{
    m_dMass = mass;
}

double CBody::get_mass() const
{
    return m_dMass;
}

void CBody::push_springs(int indxVertex)
{
    ++m_nIncomingLinks;
    m_nSprings.emplace_back(indxVertex);
}

void CBody::set_force(const Vector3 & force)
{
    m_Force = force;
}

void CBody::set_velocity(const Vector3 & velocity)
{
    m_Velocity = velocity;
}

int CBody::get_incoming_links() const
{
    return m_nIncomingLinks;
}

const Vector3 & CBody::get_position() const
{
    return m_Position;
}

const std::vector<int>& CBody::get_springs() const
{
    return m_nSprings;
}

const Vector3 & CBody::get_force() const
{
    return m_Force;
}

void CBody::reset_force()
{
    m_Force.reset();
}

const Vector3 & CBody::get_velocity() const
{
    return m_Velocity;
}

void CBody::reset_velocity()
{
    m_Velocity.reset();
}
