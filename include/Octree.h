#pragma once

#include <vector>
#include <deque>
#include <boost/pool/pool_alloc.hpp>

#include "body.h"
#include "Random.h"
#include "primitives.h"
#include "layout_settings.h"

struct COctreeNode 
{
    using Ptr_t = COctreeNode*;
    using AllocPool_t = boost::fast_pool_allocator<COctreeNode>;
public:
    Ptr_t childs[8];
    CBody *body;
    double mass;
    Vector3 massVector;
    double left;
    double right;
    double top;
    double bottom;
    double front;
    double back;

    void reset() 
    {
        childs[0] = childs[1] = childs[2] = childs[3] = childs[4] = childs[5] = childs[6] = childs[7] = NULL;
        body = NULL;
        massVector.reset();
        mass = 0;
        left = right = top = bottom = front = back = 0;
    }

    static void* operator new(size_t)
    {
        return m_AllocPool.allocate();
    }

    static void operator delete(void* obj)
    {
        m_AllocPool.deallocate((Ptr_t)obj);
    }
private:
    static AllocPool_t m_AllocPool;
};

class COctreeNodePool 
{
public:
    COctreeNodePool()
    {
        reset(0);
    }

    void reset(size_t count)
    {
        pool.reserve(count);
        for (size_t i = pool.size(); i < count; ++i)
        {
            pool.emplace_back(new COctreeNode());
        }

        currentAvailable = 0;
    }

    COctreeNode* get()
    {
        COctreeNode *result;
        if (currentAvailable < pool.size()) {
            result = pool[currentAvailable];
            result->reset();
        }
        else {
            result = new COctreeNode();
            pool.emplace_back(result);
        }
        currentAvailable += 1;
        return result;
    }

private:
    size_t currentAvailable = 0;
    std::vector<COctreeNode*> pool;
};

class COctree 
{
public:
    COctree() 
        : random(1984)
    {
    }

    void insertBodies(std::vector<CBody> &bodies);
    void updateBodyForce(CBody& sourceBody);

private:
    COctreeNode *createRootNode(std::vector<CBody> &bodies);
    bool insert(CBody* body, COctreeNode* node);

private:
    CRandom random;
    COctreeNodePool treeNodes;
    COctreeNode *root;
};
