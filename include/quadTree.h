//
//  quadTree.h
//  layout++
//
//  Created by Andrei Kashcha on 5/21/15.
//  Copyright (c) 2015 Andrei Kashcha. All rights reserved.
//

#ifndef __layout____quadTree__
#define __layout____quadTree__

#include <vector>
#include "primitives.h"
#include "Random.h"

struct QuadTreeNode {
  QuadTreeNode *quads[8];
  Body *body;
  double mass;
  Vector3 massVector;
  double left;
  double right;
  double top;
  double bottom;
  double front;
  double back;
 
  void reset() {
    quads[0] = quads[1] = quads[2] = quads[3] = quads[4] = quads[5] = quads[6] = quads[7] = NULL;
    body = NULL;
    massVector.reset();
    mass = 0;
    left = right = top = bottom = front = back = 0;
  }
};

class NodePool {
  size_t currentAvailable = 0;
  std::vector<QuadTreeNode *> pool;
public:
  void reset() {
    currentAvailable = 0;
  }

  QuadTreeNode* get() {
    QuadTreeNode *result;
    if (currentAvailable < pool.size()) {
      result = pool[currentAvailable];
      result->reset();
    } else {
      result = new QuadTreeNode();
      pool.push_back(result);
    }
    currentAvailable += 1;
    return result;
  }
};

class QuadTree {
  Random random;
  const LayoutSettings *layoutSettings;
  NodePool treeNodes;
  QuadTreeNode *root;
  QuadTreeNode *createRootNode(std::vector<Body> &bodies);
  void insert(Body *body, QuadTreeNode *node);
public:
  QuadTree(const LayoutSettings& _settings) {
    layoutSettings = &_settings;
    random = Random(1984);
  }
  void insertBodies(std::vector<Body> &bodies);
  void updateBodyForce(Body *sourceBody);
};

#endif /* defined(__layout____quadTree__) */
