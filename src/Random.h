#pragma once 
#include <cstdlib>
#include <chrono>

using namespace std::chrono;

class CRandom 
{
public:
    CRandom(long nSeed)
        : m_nSeed(nSeed)
    {
    }

    CRandom() {
        m_nSeed = duration_cast< milliseconds >(system_clock::now().time_since_epoch()).count();
    }

    double nextDouble()
    {
        // Robert Jenkins' 32 bit integer hash function.
        m_nSeed = ((m_nSeed + 0x7ed55d16) + (m_nSeed << 12)) & 0xffffffff;
        m_nSeed = ((m_nSeed ^ 0xc761c23c) ^ (m_nSeed >> 19)) & 0xffffffff;
        m_nSeed = ((m_nSeed + 0x165667b1) + (m_nSeed << 5)) & 0xffffffff;
        m_nSeed = ((m_nSeed + 0xd3a2646c) ^ (m_nSeed << 9)) & 0xffffffff;
        m_nSeed = ((m_nSeed + 0xfd7046c5) + (m_nSeed << 3)) & 0xffffffff;
        m_nSeed = ((m_nSeed ^ 0xb55a4f09) ^ (m_nSeed >> 16)) & 0xffffffff;
        return static_cast <double> (m_nSeed & 0xfffffff) / static_cast <double> (0x10000000);
    }

    double next(double max)
    {
        return nextDouble() * max;
    }
private:
    long m_nSeed;
};
