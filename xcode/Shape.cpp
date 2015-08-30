//
//  Shape.cpp
//  UserApp
//
//  Created by Kathleen Sullivan on 8/28/15.
//
//

#include "Shape.h"

Shape::Shape() :
centroid( cv::Point() ),
mAcceleration( Vec2f( 0.0, 0.0) ),
mVelocity( Vec2f( 0.0, -2.0) ),
mMaxSpeed( 40.0 ),
mMaxForce( 100.0 ),
ID(-1),
lastFrameSeen(-1),
matchFound(false),
mOffBalance(false),
moving(false),
stillness(0),
motion(0.0f)
{
}

// TRAIL THE CENTROID
void Shape::arrive(Vec2f centroidLocation)
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

void Shape::applyForce( Vec2f force )
{
    // potentially add mass here A = F / M
    mAcceleration += force;
}

void Shape::updateTrail(  ) {

    // Update velocity
    mVelocity += mAcceleration;
    // Limit speed
    mVelocity.limit( mMaxSpeed );
    mLocation += mVelocity;
    // Reset acceleration to 0 each cycle
    mAcceleration = Vec2f::zero();
    
    mCenterTrail.push_back(mLocation);
    if ( mCenterTrail.size() > 50 ) {
        mCenterTrail.pop_front();
    }
}