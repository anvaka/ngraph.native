#include "stdafx.h"

#include "Octree.h"

COctreeNode *COctree::createRootNode(std::vector<CBody> &bodies) {
    double x1 = INT32_MAX, x2 = INT32_MIN,
        y1 = INT32_MAX, y2 = INT32_MIN,
        z1 = INT32_MAX, z2 = INT32_MIN;

    for (const auto& body : bodies)
    {
        const auto& pos = body.get_position();
        if (pos.x < x1) x1 = pos.x;
        if (pos.x > x2) x2 = pos.x;

        if (pos.y < y1) y1 = pos.y;
        if (pos.y > y2) y2 = pos.y;

        if (pos.z < z1) z1 = pos.z;
        if (pos.z > z2) z2 = pos.z;
    }

    // squarify bounds:
    double maxSide = std::max(x2 - x1, std::max(y2 - y1, z2 - z1));

    if (maxSide == 0) {
        maxSide = bodies.size() * 500;
        x1 -= maxSide;
        y1 -= maxSide;
        z1 -= maxSide;
        x2 += maxSide;
        y2 += maxSide;
        z2 += maxSide;
    }
    else {
        x2 = x1 + maxSide;
        y2 = y1 + maxSide;
        z2 = z1 + maxSide;
    }

    COctreeNode *root = treeNodes.get();
    root->left = x1;
    root->right = x2;
    root->top = y1;
    root->bottom = y2;
    root->back = z1;
    root->front = z2;

    return root;
}

bool COctree::insert(CBody *body, COctreeNode *node) {
    const auto& pos = body->get_position();
    if (!node->body)
    {
        // This is internal node. Update the total mass of the node and center-of-mass.
        const auto mass = body->get_mass();
        node->mass += mass;
        node->massVector.x += mass * pos.x;
        node->massVector.y += mass * pos.y;
        node->massVector.z += mass * pos.z;
        // Recursively insert the body in the appropriate quadrant.
        // But first find the appropriate quadrant.
        int quadIdx = 0; // Assume we are in the 0's quad.
        double left = node->left,
            right = (node->right + left) / 2,
            top = node->top,
            bottom = (node->bottom + top) / 2,
            back = node->back,
            front = (node->front + back) / 2;

        if (pos.x > right) { // somewhere in the eastern part.
            quadIdx += 1;
            left = right;
            right = node->right;
        }
        if (pos.y > bottom) { // and in south.
            quadIdx += 2;
            top = bottom;
            bottom = node->bottom;
        }
        if (pos.z > front) { // and in frontal part
            quadIdx += 4;
            back = front;
            front = node->front;
        }

        COctreeNode *child = node->childs[quadIdx];
        if (child)
            return insert(body, child);

        // The node is internal but this quadrant is not taken. Add subnode to it.
        child = treeNodes.get();
        child->left = left;
        child->top = top;
        child->right = right;
        child->bottom = bottom;
        child->back = back;
        child->front = front;
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
            const double offset = random.nextDouble(),
                dx = (node->right - node->left) * offset,
                dy = (node->bottom - node->top) * offset,
                dz = (node->front - node->back) * offset;

            oldBody->set_position(
                Vector3{
                    node->left + dx,
                    node->top + dy,
                    node->back + dz
                });
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

    double fx = 0, fy = 0, fz = 0;

    std::stack<COctreeNode *> queue;
    queue.emplace(root);
    while (!queue.empty())
    {
        COctreeNode* node = queue.top(); queue.pop();

        CBody *body = node->body;
        bool differentBody = !(*body == sourceBody);
        if (!differentBody)
            continue;

        Vector3 start_pos;
        if (body)
        {
            start_pos = body->get_position();
        }
        else
        {
            start_pos = Vector3{
                node->massVector.x / node->mass,
                node->massVector.y / node->mass,
                node->massVector.z / node->mass
            };
        }
            
        double dx = start_pos.x - src_pos.x;
        double dy = start_pos.y - src_pos.y;
        double dz = start_pos.z - src_pos.z;

        double r = sqrt(dx * dx + dy * dy + dz * dz);

        if (r == 0) {
            dx = (random.nextDouble() - 0.5) / 50;
            dy = (random.nextDouble() - 0.5) / 50;
            dz = (random.nextDouble() - 0.5) / 50;
            r = sqrt(dx * dx + dy * dy + dz * dz);
        }

        if (body)
        {
            const double v = LayoutSettings::gravity * body->get_mass() * src_mass / (r * r * r);
            fx += v * dx;
            fy += v * dy;
            fz += v * dz;
        }
        else
        {
            // If s / r < θ, treat this internal node as a single body, and calculate the
            // force it exerts on sourceBody, and add this amount to sourceBody's net force.
            if ((node->right - node->left) / r < LayoutSettings::theta) {
                // in the if statement above we consider node's width only
                // because the region was squarified during tree creation.
                // Thus there is no difference between using width or height.
                const double v = LayoutSettings::gravity * node->mass * src_mass / (r * r * r);
                fx += v * dx;
                fy += v * dy;
                fz += v * dz;
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

    const auto& force = sourceBody.get_force();
    sourceBody.set_force(
        Vector3 {
            force.x + fx,
            force.y + fy,
            force.z + fz
        });
}

