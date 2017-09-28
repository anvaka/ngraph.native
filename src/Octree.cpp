//
//  quadTree.cpp
//  layout++
//
//  Created by Andrei Kashcha on 5/21/15.
//  Copyright (c) 2015 Andrei Kashcha. All rights reserved.
//
//  NOTE: This code was copied as is from https://github.com/anvaka/ngraph.quadtreebh3d/blob/master/index.js
//  Some of the coments are no longer relvant to C++.
//

#include "quadTree.h"
#include <cmath>

NotEnoughQuadSpaceException  _NotEnoughQuadSpaceException;

QuadTreeNode *QuadTree::createRootNode(std::vector<Body> &bodies) {
  double x1 = INT32_MAX, x2 = INT32_MIN,
  y1 = INT32_MAX, y2 = INT32_MIN,
  z1 = INT32_MAX, z2 = INT32_MIN;

  for (std::vector<Body>::iterator body = bodies.begin() ; body != bodies.end(); ++body) {
    double x = body->pos.x;
    double y = body->pos.y;
    double z = body->pos.z;
    if (x < x1) x1 = x;
    if (x > x2) x2 = x;
    if (y < y1) y1 = y;
    if (y > y2) y2 = y;
    if (z < z1) z1 = z;
    if (z > z2) z2 = z;
  }

  // squarify bounds:
  double maxSide = std::max(x2 - x1, std::max(y2 - y1, z2 - z1));

  if (maxSide == 0) {
    maxSide = bodies.size() * 500;
    x1 -=  maxSide;
    y1 -=  maxSide;
    z1 -=  maxSide;
    x2 += maxSide;
    y2 += maxSide;
    z2 += maxSide;
  } else {
    x2 = x1 + maxSide;
    y2 = y1 + maxSide;
    z2 = z1 + maxSide;
  }


  QuadTreeNode *root = treeNodes.get();
  root->left = x1;
  root->right = x2;
  root->top = y1;
  root->bottom = y2;
  root->back = z1;
  root->front = z2;

  return root;
}

void QuadTree::insert(Body *body, QuadTreeNode *node) {
  if (!node->body) {
    // This is internal node. Update the total mass of the node and center-of-mass.
    double x = body->pos.x;
    double y = body->pos.y;
    double z = body->pos.z;
    node->mass += body->mass;
    node->massVector.x += body->mass * x;
    node->massVector.y += body->mass * y;
    node->massVector.z += body->mass * z;
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

    QuadTreeNode *child = node->quads[quadIdx];
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
    } else {
      // continue searching in this quadrant.
      insert(body, child);
    }
  } else {
    // We are trying to add to the leaf node.
    // We have to convert current leaf into internal node
    // and continue adding two nodes.
    Body *oldBody = node->body;
    node->body = NULL; // internal nodes do not carry bodies
    if (oldBody->pos.sameAs(body->pos)) {
      int retriesCount = 3;
      do {
        double offset = random.nextDouble(),
        dx = (node->right - node->left) * offset,
        dy = (node->bottom - node->top) * offset,
        dz = (node->front - node->back) * offset;

        oldBody->pos.x = node->left + dx;
        oldBody->pos.y = node->top + dy;
        oldBody->pos.z = node->back + dz;
        retriesCount -= 1;
        // Make sure we don't bump it out of the box. If we do, next iteration should fix it
      } while (retriesCount > 0 && oldBody->pos.sameAs(body->pos));

      if (retriesCount == 0 && oldBody->pos.sameAs(body->pos)) {
        // This is very bad, we ran out of precision.
        // We cannot proceed under current root's constraints, so let's
        // throw - this will cause parent to give bigger space for the root
        // node, and hopefully we can fit on the subsequent iteration.
        throw _NotEnoughQuadSpaceException;
      }
    }
    // Next iteration should subdivide node further.
    insert(oldBody, node);
    insert(body, node);
  }
}

void QuadTree::insertBodies(std::vector<Body> &bodies) {
  try {
    treeNodes.reset();
    root = createRootNode(bodies);
    if (bodies.size() > 0) {
      root->body = &bodies[0];
    }

    for (size_t i = 1; i < bodies.size(); ++i) {
      insert(&(bodies[i]), root);
    }
    return;
  } catch(NotEnoughQuadSpaceException &e) {
    // well we tried, but some bodies ended up on the same
    // spot, cannot do anything, but hope that next iteration will fix it
  }
};

void QuadTree::updateBodyForce(Body *sourceBody) {
  std::vector<QuadTreeNode *> queue;
  int queueLength = 1;
  int shiftIndex = 0;
  size_t pushIndex = 0;
  double v, dx, dy, dz, r;
  double fx = 0, fy = 0, fz = 0;
  queue.push_back(root);
  while (queueLength) {
    QuadTreeNode *node = queue[shiftIndex];
    Body *body = node->body;
    queueLength -= 1;
    shiftIndex += 1;
    bool differentBody = (body != sourceBody);
    if (body != NULL && differentBody) {
      // If the current node is a leaf node (and it is not source body),
      // calculate the force exerted by the current node on body, and add this
      // amount to body's net force.
      dx = body->pos.x - sourceBody->pos.x;
      dy = body->pos.y - sourceBody->pos.y;
      dz = body->pos.z - sourceBody->pos.z;
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

      v = layoutSettings->gravity * body->mass * sourceBody->mass / (r * r * r);
      fx += v * dx;
      fy += v * dy;
      fz += v * dz;
    } else if (differentBody) {
      // Otherwise, calculate the ratio s / r,  where s is the width of the region
      // represented by the internal node, and r is the distance between the body
      // and the node's center-of-mass

      dx = node->massVector.x / node->mass - sourceBody->pos.x;
      dy = node->massVector.y / node->mass - sourceBody->pos.y;
      dz = node->massVector.z / node->mass - sourceBody->pos.z;

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
      if ((node->right - node->left) / r < layoutSettings->theta) {
        // in the if statement above we consider node's width only
        // because the region was squarified during tree creation.
        // Thus there is no difference between using width or height.
        v = layoutSettings->gravity * node->mass * sourceBody->mass / (r * r * r);
        fx += v * dx;
        fy += v * dy;
        fz += v * dz;
      } else {
        // Otherwise, run the procedure recursively on each of the current node's children.
        for (int i = 0; i < 8; ++i) {
          if (node->quads[i]) {
            if (queue.size() < pushIndex) {
              queue[pushIndex] = node->quads[i];
            } else {
              queue.push_back(node->quads[i]);
            }
            queueLength += 1;
            pushIndex += 1;
          }
        }
      }
    }

  }

  sourceBody->force.x += fx;
  sourceBody->force.y += fy;
  sourceBody->force.z += fz;
}

