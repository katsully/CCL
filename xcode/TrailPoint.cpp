//
//  TrailPoint.cpp
//  CCL
//
//  Created by Kat Sullivan on 8/30/15.
//
//

#include "TrailPoint.h"

TrailPoint::TrailPoint() :
mAcceleration( Vec2f( 0.0, 0.0) ),
mVelocity( Vec2f( 0.0, -2.0) ),
mMaxSpeed( 40.0 ),
mMaxForce( 100.0 )
{
}

// TRAIL THE CENTROID
void TrailPoint::arrive(Vec2f centroidLocation)
{
    Vec2f desired = centroidLocation - mLocation;
    float d = desired.length();
    desired.normalize();
    
    // Scale with arbitary damping within 100 pixels
    if ( d < 100.0 ) {
        float m = lmap(d, 0.0f, 100.0f, 0.0f, mMaxSpeed);
        desired *= m;
    } else {
        desired *= mMaxSpeed;
    }
    
    // Steering = Desired minus velocity
    Vec2f steer = desired - mVelocity;
    steer.limit(mMaxForce);
    
    applyForce(steer);
}

void TrailPoint::applyForce( Vec2f force )
{
    // potentially add mass here A = F / M
    mAcceleration += force;
}

void TrailPoint::updateTrail( list<Vec2f>& trailPoints ) {
    
    // Update velocity
    mVelocity += mAcceleration;
    // Limit speed
    mVelocity.limit( mMaxSpeed );
    mLocation += mVelocity;
    // Reset acceleration to 0 each cycle
    mAcceleration = Vec2f::zero();
    
    trailPoints.push_back(mLocation);
    if ( trailPoints.size() > 50 ) {
        trailPoints.pop_front();
    }
}