//
//  primitives.h
//  layout++
//
//  Created by Andrei Kashcha on 5/21/15.
//  Copyright (c) 2015 Andrei Kashcha. All rights reserved.
//

#ifndef layout___primitives_h
#define layout___primitives_h
#include <cmath>        // std::abs
#include <vector>

using namespace std;

struct LayoutSettings {
  double stableThreshold = 0.009;
  double gravity = -1.2;
  double theta = 1.2;
  double dragCoeff = 0.02;
  double springCoeff = 0.0008;
  double springLength = 30;
  double timeStep = 20;
};

struct Vector3 {
  double x = 0.0;
  double y = 0.0;
  double z = 0.0;

  Vector3(double _x, double _y, double _z) :
  x(_x), y(_y), z(_z) {};

  Vector3() {}

  void reset () {
    x = y = z = 0;
  }

  bool sameAs(const Vector3 &other) {

    double dx = std::abs(x - other.x);
    double dy = std::abs(y - other.y);
    double dz = std::abs(z - other.z);

    return (dx < 1e-8 && dy < 1e-8 && dz < 1e-8);
  }
};

struct Body {
  Vector3 pos;
  Vector3 prevPos;
  Vector3 force;
  Vector3 velocity;
  double mass = 1.0;

  vector<int> springs; // these are outgoing connections.
  // This is just a number of incoming connections for this body,
  // so we can count its mass appropriately.
  int incomingCount = 0;

  Body() { }
  Body(Vector3 _pos): pos(_pos), prevPos(_pos) {}

  void setPos(const Vector3 &_pos) {
    pos = _pos;
    prevPos = _pos;
  }

  bool positionInitialized() {
    return pos.x != 0 || pos.y != 0 || pos.z != 0;
  }
};
class NotEnoughQuadSpaceException: public exception {};

#endif
