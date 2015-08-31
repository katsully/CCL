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
    Vec3f mAcceleration;
    Vec3f mVelocity;
    Vec3f mLocation;
    float mMaxSpeed;
    float mMaxForce;
    
    list<Vec3f> mTrail;
    
    void arrive( Vec3f centroidLocation, bool fromUser=false );
    void applyForce (Vec3f force );
    void updateTrail();
};