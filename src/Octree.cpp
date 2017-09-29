#include "stdafx.h"

#include "Octree.h"

#include <cmath>

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
    bool result = true;
    if (!node->body) {
        const auto& pos = body->get_position();
        // This is internal node. Update the total mass of the node and center-of-mass.
        double x = pos.x;
        double y = pos.y;
        double z = pos.z;
        node->mass += body->get_mass();
        node->massVector.x += body->get_mass() * x;
        node->massVector.y += body->get_mass() * y;
        node->massVector.z += body->get_mass() * z;
        // Recursively insert the body in the appropriate quadrant.
        // But first find the appropriate quadrant.
        int quadIdx = 0; // Assume we are in the 0's quad.
        double left = node->left,
            right = (node->right + left) / 2,
            top = node->top,
            bottom = (node->bottom + top) / 2,
            back = node->back,
            front = (node->front + back) / 2;

        if (x > right) { // somewhere in the eastern part.
            quadIdx += 1;
            left = right;
            right = node->right;
        }
        if (y > bottom) { // and in south.
            quadIdx += 2;
            top = bottom;
            bottom = node->bottom;
        }
        if (z > front) { // and in frontal part
            quadIdx += 4;
            back = front;
            front = node->front;
        }

        COctreeNode *child = node->quads[quadIdx];
        if (!child) {
            // The node is internal but this quadrant is not taken. Add subnode to it.
            child = treeNodes.get();
            child->left = left;
            child->top = top;
            child->right = right;
            child->bottom = bottom;
            child->back = back;
            child->front = front;
            child->body = body;
            node->quads[quadIdx] = child;
        }
        else {
            // continue searching in this quadrant.
            result = insert(body, child);
        }
    }
    else {
        // We are trying to add to the leaf node.
        // We have to convert current leaf into internal node
        // and continue adding two nodes.
        CBody *oldBody = node->body;
        auto& pos = oldBody->get_position();
        node->body = NULL; // internal nodes do not carry bodies
        if (pos.sameAs(body->get_position())) {
            int retriesCount = 3;
            do {
                double offset = random.nextDouble(),
                    dx = (node->right - node->left) * offset,
                    dy = (node->bottom - node->top) * offset,
                    dz = (node->front - node->back) * offset;

                oldBody->set_position(
                    Vector3(
                        node->left + dx,
                        node->top + dy,
                        node->back + dz
                    )
                );
                retriesCount -= 1;
                // Make sure we don't bump it out of the box. If we do, next iteration should fix it
            } while (retriesCount > 0 && pos.sameAs(body->get_position()));

            if (retriesCount == 0 && pos.sameAs(body->get_position())) {
                // This is very bad, we ran out of precision.
                // We cannot proceed under current root's constraints, so let's
                // throw - this will cause parent to give bigger space for the root
                // node, and hopefully we can fit on the subsequent iteration.
                return false;
            }
        }
        // Next iteration should subdivide node further.
        result = insert(oldBody, node);
        if (result)
            result = insert(body, node);
    }

    return result;
}

void COctree::insertBodies(std::vector<CBody> &bodies) {
    treeNodes.reset(bodies.size() * 8);

    root = createRootNode(bodies);

    if (bodies.size() > 0) {
        root->body = &bodies[0];
    }

    for (size_t i = 1; i < bodies.size(); ++i) {
        if (!insert(&(bodies[i]), root))
            return;
    }
};

void COctree::updateBodyForce(CBody& sourceBody) 
{
    int shiftIndex = 0;
    size_t pushIndex = 0;
    double v, dx, dy, dz, r;
    double fx = 0, fy = 0, fz = 0;

    int queueLength = 1;
    std::vector<COctreeNode *> queue;
    queue.reserve(9);
    queue.emplace_back(root);

    const auto& src_pos = sourceBody.get_position();

    while (queueLength--) 
    {
        COctreeNode& node = (*queue[shiftIndex++]);
        CBody *body = node.body;

        bool differentBody = (body != &sourceBody);
        if (body != NULL && differentBody) 
        {
            // If the current node is a leaf node (and it is not source body),
            // calculate the force exerted by the current node on body, and add this
            // amount to body's net force.
            {
                const auto& body_pos = body->get_position();
                dx = body_pos.x - src_pos.x;
                dy = body_pos.y - src_pos.y;
                dz = body_pos.z - src_pos.z;
            }

            r = sqrt(dx * dx + dy * dy + dz * dz);

            if (r == 0) {
                // Poor man's protection against zero distance.
                dx = (random.nextDouble() - 0.5) / 50;
                dy = (random.nextDouble() - 0.5) / 50;
                dz = (random.nextDouble() - 0.5) / 50;
                r = sqrt(dx * dx + dy * dy + dz * dz);
            }

            // This is standard gravitation force calculation but we divide
            // by r^3 to save two operations when normalizing force vector.

            
            v = LayoutSettings::gravity * body->get_mass() * sourceBody.get_mass() / (r * r * r);
            fx += v * dx;
            fy += v * dy;
            fz += v * dz;
        }
        else if (differentBody) 
        {
            // Otherwise, calculate the ratio s / r,  where s is the width of the region
            // represented by the internal node, and r is the distance between the body
            // and the node's center-of-mass

            dx = node.massVector.x / node.mass - src_pos.x;
            dy = node.massVector.y / node.mass - src_pos.y;
            dz = node.massVector.z / node.mass - src_pos.z;

            r = sqrt(dx * dx + dy * dy + dz * dz);

            if (r == 0) {
                // Sorry about code duplication. I don't want to create many functions
                // right away. Just want to see performance first.
                dx = (random.nextDouble() - 0.5) / 50;
                dy = (random.nextDouble() - 0.5) / 50;
                dz = (random.nextDouble() - 0.5) / 50;
                r = sqrt(dx * dx + dy * dy + dz * dz);
            }
            // If s / r < θ, treat this internal node as a single body, and calculate the
            // force it exerts on sourceBody, and add this amount to sourceBody's net force.
            if ((node.right - node.left) / r < LayoutSettings::theta) {
                // in the if statement above we consider node's width only
                // because the region was squarified during tree creation.
                // Thus there is no difference between using width or height.
                v = LayoutSettings::gravity * node.mass * sourceBody.get_mass() / (r * r * r);
                fx += v * dx;
                fy += v * dy;
                fz += v * dz;
            }
            else {
                // Otherwise, run the procedure recursively on each of the current node's children.
                for (int i = 0; i < 8; ++i) {
                    if (node.quads[i]) {
                        if (queue.size() < pushIndex) {
                            queue[pushIndex] = node.quads[i];
                        }
                        else {
                            queue.emplace_back(node.quads[i]);
                        }
                        queueLength++;
                        pushIndex++;
                    }
                }
            }
        }
    }

    {
        const auto& force = sourceBody.get_force();
        sourceBody.set_force(
            Vector3(
                force.x + fx,
                force.y + fy,
                force.z + fz
            )
        );
    }
}

