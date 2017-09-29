
inline void CBody::set_position(const Vector3 & pos)
{
    m_Position = pos;
}

inline void CBody::set_mass(double mass)
{
    m_dMass = mass;
}

inline double CBody::get_mass() const
{
    return m_dMass;
}

inline void CBody::push_springs(int indxVertex)
{
    ++m_nIncomingLinks;
    m_nSprings.emplace_back(indxVertex);
}

inline void CBody::set_force(const Vector3 & force)
{
    m_Force = force;
}

inline void CBody::set_velocity(const Vector3 & velocity)
{
    m_Velocity = velocity;
}

inline int CBody::get_incoming_links() const
{
    return m_nIncomingLinks;
}

inline const Vector3 & CBody::get_position() const
{
    return m_Position;
}

inline const std::vector<int>& CBody::get_springs() const
{
    return m_nSprings;
}

inline const Vector3 & CBody::get_force() const
{
    return m_Force;
}

inline void CBody::reset_force()
{
    m_Force.reset();
}

inline const Vector3 & CBody::get_velocity() const
{
    return m_Velocity;
}

inline void CBody::reset_velocity()
{
    m_Velocity.reset();
}
