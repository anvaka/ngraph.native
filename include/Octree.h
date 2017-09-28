#pragma once

#include <vector>
#include <deque>
#include "body.h"
#include "Random.h"
#include "primitives.h"
#include "layout_settings.h"

struct COctreeNode 
{
    COctreeNode *quads[8];
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
        quads[0] = quads[1] = quads[2] = quads[3] = quads[4] = quads[5] = quads[6] = quads[7] = NULL;
        body = NULL;
        massVector.reset();
        mass = 0;
        left = right = top = bottom = front = back = 0;
    }
};

class COctreeNodePool 
{
public:
    COctreeNodePool()
    {
        reset(0);
    }

    void reset(int count)
    {
        pool.resize(count);
        current_available = pool.begin();
    }

    COctreeNode* get()
    {
        COctreeNode *result;
        if (current_available == pool.end())
        {
            current_available = pool.insert(pool.end(), COctreeNode());
        }
        
        result = &(*current_available);
        result->reset();

        ++current_available;

        return result;
    }

private:
    std::vector<COctreeNode>::iterator current_available;
    std::vector<COctreeNode> pool;
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
    void insert(CBody *body, COctreeNode *node);

private:
    CRandom random;
    COctreeNodePool treeNodes;
    COctreeNode *root;
};
