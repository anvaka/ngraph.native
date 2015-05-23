//
//  Random.h
//  layout++
//
//  Created by Andrei Kashcha on 5/21/15.
//  Copyright (c) 2015 Andrei Kashcha. All rights reserved.
//

#ifndef layout___Random_h
#define layout___Random_h

#include <cstdlib>


class Random {
  
public:
  static double nextDouble() {
    return static_cast <double> (rand()) / static_cast <double> (RAND_MAX);
  }
  
  static double next(double max) {
    return Random::nextDouble() * max;
  }
  
};
#endif
