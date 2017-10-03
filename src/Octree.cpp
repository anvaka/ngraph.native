#include <stdafx.h>

#include <Octree.h>

COctreeNode::AllocPool_t COctreeNode::m_AllocPool{ 4096, 0 };

COctreeNode *COctree::createRootNode(std::vector<CBody> &bodies) {
    double end_pos[4]{ INT32_MIN, INT32_MIN, INT32_MIN, 1. };
    double start_pos[4]{ INT32_MAX, INT32_MAX, INT32_MAX, 1. };

    for (const auto& body : bodies)
    {
        const auto& pos = body.get_position();
        bool start_res[coord::num], end_res[coord::num];
        pos.is_less(start_pos, start_res);
        pos.is_great(end_pos, end_res);

        for (int i = 0; i < coord::num; ++i)
        {
            if (start_res[i])
                start_pos[i] = pos[i];

            if (end_res[i])
                end_pos[i] = pos[i];
        }
    }

    // squarify bounds:
    Vector3 v_end_pos{ end_pos };
    Vector3 v_start_pos{ start_pos };

    const Vector3 diff = v_end_pos - v_start_pos;
    double maxSide = std::max(diff[X], std::max(diff[Y], diff[Z]));

    if (maxSide == 0.) {
        maxSide = bodies.size() * 500;
        v_start_pos -= maxSide;
        v_end_pos += maxSide;
    }
    else {
        v_end_pos = v_start_pos + maxSide;
    }

    COctreeNode *root = treeNodes.get();
    root->start = v_start_pos;
    root->end = v_end_pos;

    return root;
}

bool COctree::insert(CBody *body, COctreeNode *node) {
    auto& pos = body->get_position();
    if (!node->body)
    {
        // This is internal node. Update the total mass of the node and center-of-mass.
        const auto mass = body->get_mass();
        node->massVector += pos * mass;
        node->mass += mass;
        // Recursively insert the body in the appropriate quadrant.
        // But first find the appropriate quadrant.
        int quadIdx = 0; // Assume we are in the 0's quad.

        Vector3 new_start{ node->start };
        Vector3 new_end{ (node->end + node->start) / 2. };
        double  left = new_start[X],
                top = new_start[Y],
                back = new_start[Z],
                right = new_end[X],
                bottom = new_end[Y],
                front = new_end[Z];

        bool greats[coord::num];
        pos.is_great(new_end, greats);

        if (greats[X]) // somewhere in the eastern part.
        {
            quadIdx += 1;
            left = right;
            right = node->end[X];
        }

        if (greats[Y]) // and in south.
        {
            quadIdx += 2;
            top = bottom;
            bottom = node->end[Y];
        }

        if (greats[Z]) // and in frontal part
        {
            quadIdx += 4;
            back = front;
            front = node->end[Z];
        }

        COctreeNode *child = node->childs[quadIdx];
        if (child)
            return insert(body, child);

        // The node is internal but this quadrant is not taken. Add subnode to it.
        child = treeNodes.get();
        child->start = { left, top, back };
        child->end = { right, bottom, front };

        child->body = body;
        node->childs[quadIdx] = child;
        return true;
    }

    // We are trying to add to the leaf node.
    // We have to convert current leaf into internal node
    // and continue adding two nodes.
    CBody *oldBody = nullptr;
    std::swap(oldBody, node->body);

    const auto& old_pos = oldBody->get_position();
    if (old_pos.sameAs(pos)) 
    {
        int retriesCount = 3;
        do 
        {
            const double offset = random.nextDouble();
            const Vector3 diff = (node->end - node->start) * offset;

            oldBody->set_position(node->start + diff);
            // Make sure we don't bump it out of the box. If we do, next iteration should fix it
        } while (--retriesCount > 0 && old_pos.sameAs(pos));

        if (retriesCount == 0 && old_pos.sameAs(pos)) 
        {
            // This is very bad, we ran out of precision.
            // We cannot proceed under current root's constraints, so let's
            // throw - this will cause parent to give bigger space for the root
            // node, and hopefully we can fit on the subsequent iteration.
            return false;
        }
    }
    // Next iteration should subdivide node further.
    return insert(oldBody, node) && insert(body, node);
}

void COctree::insertBodies(std::vector<CBody> &bodies) {
    treeNodes.reset(bodies.size() * 2);

    root = createRootNode(bodies);

    if (bodies.size() > 0) {
        root->body = &bodies[0];
    }

    for (size_t i = 1; i < bodies.size(); ++i) {
        if (!insert(&bodies[i], root))
            break;
    }
};

void COctree::updateBodyForce(CBody& sourceBody) 
{
    const auto& src_pos = sourceBody.get_position();
    const double src_mass = sourceBody.get_mass();

    Vector3 coordForce;

    std::stack<COctreeNode *, std::vector<COctreeNode *>> queue;
    queue.emplace(root);
    while (!queue.empty())
    { 
        COctreeNode* node = queue.top(); queue.pop();

        CBody *body = node->body;
        if (*body == sourceBody)
            continue;

        Vector3 start_pos;
        if (body)
        {
            start_pos = body->get_position();
        }
        else
        {
            start_pos = node->massVector / node->mass;
        }

        Vector3 diff = start_pos - src_pos;
        double r = sqrt((diff * diff).summ_elem());

        if (r == 0) 
        {
            diff = Vector3{ random.nextDouble(),random.nextDouble(), random.nextDouble() };
            diff = (diff - Vector3{ 0.5 }) / Vector3{ 50.0 };

            r = sqrt((diff * diff).summ_elem());
        }

        if (body)
        {
            const double v = LayoutSettings::gravity * body->get_mass() * src_mass / (r * r * r);
            coordForce += diff * v;
        }
        else
        {
            // If s / r < θ, treat this internal node as a single body, and calculate the
            // force it exerts on sourceBody, and add this amount to sourceBody's net force.
            if (((node->end[X] - node->start[X]) / r) < LayoutSettings::theta) {
                // in the if statement above we consider node's width only
                // because the region was squarified during tree creation.
                // Thus there is no difference between using width or height.
                const double v = LayoutSettings::gravity * node->mass * src_mass / (r * r * r);
                coordForce += diff * v;
            }
            else {
                // Otherwise, run the procedure recursively on each of the current node's children.
                for (const auto& quad : node->childs)
                {
                    if (quad)
                    {
                        queue.emplace(quad);
                    }
                }
            }
        }
    }

    sourceBody.set_force(coordForce);
}

