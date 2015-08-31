//
//  TrailPoint.cpp
//  CCL
//
//  Created by Kat Sullivan on 8/30/15.
//
//

#include "TrailPoint.h"

TrailPoint::TrailPoint() :
mAcceleration( Vec2f::zero() ),
mVelocity( Vec2f( 0.0, -2.0 ) ),
mLocation( Vec2f( 800, 200) ),
mMaxSpeed( 40.0 ),
mMaxForce( 100.0 )
{
}

// TRAIL THE CENTROID
void TrailPoint::arrive( Vec3f centroidLocation, bool fromUser )
{
//    cout << "location " << mLocation << endl;
    Vec3f desired = centroidLocation - mLocation;
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
    Vec3f steer = desired - mVelocity;
    steer.limit(mMaxForce);
    
    applyForce(steer);
}

void TrailPoint::applyForce( Vec3f force )
{
    // potentially add mass here A = F / M
    mAcceleration += force;
}

void TrailPoint::updateTrail() {
    
    // Update velocity
    mVelocity += mAcceleration;
    // Limit speed
    mVelocity.limit( mMaxSpeed );
    mLocation += mVelocity;
    // Reset acceleration to 0 each cycle
    mAcceleration = Vec3f::zero();
    
    mTrail.push_back(mLocation);
    if ( mTrail.size() > 100 ) {
        mTrail.pop_front();
    }
}