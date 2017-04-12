//
//  layout.cpp
//  layout++
//
//  Created by Andrei Kashcha on 5/21/15.
//  Copyright (c) 2015 Andrei Kashcha. All rights reserved.
//

#include "layout.h"
#include <iostream>
#include <cmath>
#include <map>

Layout::Layout() :tree(settings) {}

void Layout::init(int* links, long size) {
  random = Random(42);
  initBodies(links, size);

  // Now the graph is initialized. Let's make sure we get
  // good initial positions:
  setDefaultBodiesPositions();
}

void Layout::init(int *links, long linksSize, int *initialPositions, size_t posSize) {
  initBodies(links, linksSize);
  if (bodies.size() * 3 != posSize) {
    cout << "There are " << bodies.size() << " nodes in the graph and " << endl
    << posSize << " positions. It is expected that each body has exactly" << endl
    << "3 Int32 records in the positions file (x, y, z). However this is not the case" << endl
    << "here. Are you sure you are loading correct positions for this graph file?";
    throw "Positions file mismatch";
  }

  loadPositionsFromArray(initialPositions);
}

void Layout::loadPositionsFromArray(int *initialPositions) {
  for (size_t i = 0; i < bodies.size(); ++i) {
    Vector3 initialPos(initialPositions[i * 3 + 0], //+ Random::nextDouble(),
                       initialPositions[i * 3 + 1], //+ Random::nextDouble(),
                       initialPositions[i * 3 + 2] //+ Random::nextDouble()
                       );
    bodies[i].setPos(initialPos);
  }
}

void Layout::setDefaultBodiesPositions() {
  size_t maxBodyId = bodies.size();
  for (size_t i = 0; i < maxBodyId; ++i) {
    Body *body = &(bodies[i]);
    if (!body->positionInitialized()) {
      Vector3 initialPos(random.nextDouble() * log(maxBodyId) * 100,
                         random.nextDouble() * log(maxBodyId) * 100,
                         random.nextDouble() * log(maxBodyId) * 100);
      bodies[i].setPos(initialPos);
    }
    Vector3 *sourcePos = &(body->pos);
    // init neighbours position:
    for (size_t j = 0; j < body->springs.size(); ++j) {
      if (!bodies[body->springs[j]].positionInitialized()) {
        Vector3 neighbourPosition(
                                  sourcePos->x + random.next(settings.springLength) - settings.springLength/2,
                                  sourcePos->y + random.next(settings.springLength) - settings.springLength/2,
                                  sourcePos->z + random.next(settings.springLength) - settings.springLength/2
                                  );
        bodies[j].setPos(neighbourPosition);
      }
    }
  }
}

void Layout::initBodies(int* links, long size) {
  // FIXME: If there are no links in a graph, it will fail
  int from = 0;
  int maxBodyId = 0;

  // since we can have holes in the original list - let's
  // figure out max node id, and then initialize bodies
  for (int i = 0; i < size; i++) {
    int index = *(links + i);

    if (index < 0) {
      index = -index;
      from = index - 1;
      if (from > maxBodyId) maxBodyId = from;
    } else {
      int to = index - 1;
      if (to > maxBodyId) maxBodyId = to;
    }
  }

  bodies.reserve(maxBodyId + 1);
  for (int i = 0; i < maxBodyId + 1; ++i) {
    bodies.push_back(Body());
  }

  // Now that we have bodies, let's add links:
  Body *fromBody = nullptr;
  for (int i = 0; i < size; i++) {
    int index = *(links + i);
    if (index < 0) {
      index = -index;
      from = index - 1;
      fromBody = &(bodies[from]);
    } else {
      int to = index - 1;
      fromBody->springs.push_back(to);
      bodies[to].incomingCount += 1;
    }
  }

  // Finally, update body mas based on total number of neighbours:
  for (size_t i = 0; i < bodies.size(); i++) {
    Body *body = &(bodies[i]);
    body->mass = 1 + (body->springs.size() + body->incomingCount)/3.0;
  }
}

void Layout::setBodiesWeight(int *weights) {
    // FIXME: Verify that size of the weights matches size of the bodies.
    // Unfortunately current graph format does not properly store nodes without
    // edges.
    for (size_t i = 0; i < bodies.size(); i++) {
        Body *body = &(bodies[i]);
        body->mass = weights[i];
    }
}

size_t Layout::getBodiesCount() {
  return bodies.size();
}

bool Layout::step() {
  accumulate();
  double totalMovement = integrate();
  cout << totalMovement << " move" << endl;
  return totalMovement < settings.stableThreshold;
}

void Layout::accumulate() {
  tree.insertBodies(bodies);

  #pragma omp parallel for
  for (size_t i = 0; i < bodies.size(); i++) {
    Body* body = &bodies[i];
    body->force.reset();

    tree.updateBodyForce(&(*body));
    updateDragForce(&(*body));
  }

  #pragma omp parallel for
  for (size_t i = 0; i < bodies.size(); i++) {
    Body* body = &bodies[i];
    updateSpringForce(&(*body));
  }
}

double Layout::integrate() {
  double dx = 0, tx = 0,
  dy = 0, ty = 0,
  dz = 0, tz = 0,
  timeStep = settings.timeStep;
 
//dx should be private or defined inside loop
 //tx need to be reduction variable, or its value will be unpredictable.
  #pragma omp parallel for reduction(+:tx,ty,tz) private(dx,dy,dz)
  for (size_t i = 0; i < bodies.size(); i++) {
    Body* body = &bodies[i];
    double coeff = timeStep / body->mass;

    body->velocity.x += coeff * body->force.x;
    body->velocity.y += coeff * body->force.y;
    body->velocity.z += coeff * body->force.z;

    double vx = body->velocity.x,
    vy = body->velocity.y,
    vz = body->velocity.z,
    v = sqrt(vx * vx + vy * vy + vz * vz);

    if (v > 1) {
      body->velocity.x = vx / v;
      body->velocity.y = vy / v;
      body->velocity.z = vz / v;
    }

    dx = timeStep * body->velocity.x;
    dy = timeStep * body->velocity.y;
    dz = timeStep * body->velocity.z;

    body->pos.x += dx;
    body->pos.y += dy;
    body->pos.z += dz;

    tx += abs(dx); ty += abs(dy); tz += abs(dz);
  }

  return (tx * tx + ty * ty + tz * tz)/bodies.size();
}

void Layout::updateDragForce(Body *body) {
  body->force.x -= settings.dragCoeff * body->velocity.x;
  body->force.y -= settings.dragCoeff * body->velocity.y;
  body->force.z -= settings.dragCoeff * body->velocity.z;
}

void Layout::updateSpringForce(Body *source) {

  Body *body1 = source;
  for (size_t i = 0; i < source->springs.size(); ++i){
    Body *body2 = &(bodies[source->springs[i]]);

    double dx = body2->pos.x - body1->pos.x;
    double dy = body2->pos.y - body1->pos.y;
    double dz = body2->pos.z - body1->pos.z;
    double r = sqrt(dx * dx + dy * dy + dz * dz);

    if (r == 0) {
      dx = (random.nextDouble() - 0.5) / 50;
      dy = (random.nextDouble() - 0.5) / 50;
      dz = (random.nextDouble() - 0.5) / 50;
      r = sqrt(dx * dx + dy * dy + dz * dz);
    }

    double d = r - settings.springLength;
    double coeff = settings.springCoeff * d / r;

    body1->force.x += coeff * dx;
    body1->force.y += coeff * dy;
    body1->force.z += coeff * dz;

    body2->force.x -= coeff * dx;
    body2->force.y -= coeff * dy;
    body2->force.z -= coeff * dz;
  }
}
