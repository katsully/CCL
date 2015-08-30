//
//  TrailPoint.h
//  CCL
//
//  Created by Kat Sullivan on 8/30/15.
//
//

#pragma once

using namespace ci;
using namespace ci::app;
using namespace std;

class TrailPoint {
public:
    TrailPoint();
    // keep track of centroid trail
    Vec2f mAcceleration;
    Vec2f mVelocity;
    Vec2f mLocation;
    float mMaxSpeed;
    float mMaxForce;
    
    std::list<Vec2f> mTrail;
    
    void arrive( Vec2f centroidLocation );
    void applyForce (Vec2f force );
    void updateTrail();
};