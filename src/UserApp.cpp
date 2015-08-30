/*
 *
 * Copyright (c) 2013, Wieden+Kennedy
 *
 * Stephen Schieberl
 * Michael Latzoni
 *
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or
 * without modification, are permitted provided that the following
 * conditions are met:
 *
 * Redistributions of source code must retain the above copyright
 * notice, this list of conditions and the following disclaimer.
 * Redistributions in binary form must reproduce the above copyright
 * notice, this list of conditions and the following disclaimer in
 * the documentation and/or other materials provided with the
 * distribution.
 *
 * Neither the name of the Ban the Rewind nor the names of its
 * contributors may be used to endorse or promote products
 * derived from this software without specific prior written
 * permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 * FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
 * COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
 * BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
 * STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
 * ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 */

#include "cinder/app/AppBasic.h"
#include "cinder/Camera.h"
#include "cinder/Channel.h"
#include "cinder/gl/Texture.h"
#include "Cinder-OpenNI.h"
#include "ShapeDetection.h"
#include "cinder/params/Params.h"

/*
 * This application demonstrates how to display NiTE users.
 * Kat and Sergio
 */
class UserApp : public ci::app::AppBasic
{
    
public:
    void						draw();
    void						keyDown( ci::app::KeyEvent event );
    void						prepareSettings( ci::app::AppBasic::Settings* settings );
    void						setup();
    
    ShapeDetection    mShapeDetection;
    nite::Point3f mRightKnee;
    nite::Point3f mLeftKnee;
    nite::Point3f mTorso;
    
private:
    struct Bone
    {
        Bone( nite::JointType a, nite::JointType b )
        : mJointA( a ), mJointB( b )
        {
        }
        nite::JointType			mJointA;
        nite::JointType			mJointB;
    };
    
    // used for the GUI params
    params::InterfaceGl mParams;
    bool mUseBalance = false;
    bool mShowNegativeSpace = false;
    bool mShowDistanceLines = false;
    int mJointParam = 0;
    std::list<Vec3f> mJointTrail;
    
    ci::CameraPersp				mCamera;
    
    std::vector<Bone>			mBones;
    ci::Channel16u				mChannel;
    OpenNI::DeviceManagerRef	mDeviceManager;
    OpenNI::DeviceRef			mDevice;
    ci::gl::TextureRef			mTexture;
    std::vector<nite::UserData>	mUsers;
    void						onUser( nite::UserTrackerFrameRef, const OpenNI::DeviceOptions& deviceOptions );
    
    void						screenShot();
};

#include "cinder/ImageIo.h"
#include "cinder/Utilities.h"

using namespace ci;
using namespace ci::app;
using namespace std;

void UserApp::draw()
{
    gl::setViewport( getWindowBounds() );
    gl::clear( Color::black() );
    gl::setMatricesWindow( getWindowSize() );
    
    gl::color( Colorf::white() );
    if ( mShapeDetection.mSurfaceSubtract ) {
        if ( mTexture ) {
            mTexture->update( Channel32f( mShapeDetection.mSurfaceSubtract ) );
        } else {
            mTexture = gl::Texture::create( Channel32f( mShapeDetection.mSurfaceSubtract ) );
        }
        gl::draw( mTexture, mTexture->getBounds(), getWindowBounds() );
    }
    
    gl::setMatrices( mCamera );
    
    
    gl::color( Colorf( 1.0f, 0.0f, 0.0f ) );
    for ( std::vector<nite::UserData>::const_iterator iter = mUsers.begin(); iter != mUsers.end(); ++iter )
    {
        const nite::Skeleton& skeleton = iter->getSkeleton();
        
        if ( skeleton.getState() == nite::SKELETON_TRACKED ) {
            // this draws the first 15 points (so no joints get drawn twice)
            for ( int i=0; i<15; i++ ) {
                gl::enableAlphaBlending();
                gl::color( ColorA(1.0f, 1.0f, 1.0f, 0.3f) );
                glPointSize(10.0f);
                gl::disableAlphaBlending();
                
                gl::begin( GL_POINTS );
                
                const nite::SkeletonJoint& joint0 = skeleton.getJoint( mBones[i].mJointA );
                if ( mJointParam == mBones[i].mJointA ) {
                    Vec3f trailPoint = OpenNI::toVec3f(joint0.getPosition() );
                    trailPoint.x = -trailPoint.x;
                    mJointTrail.push_back(trailPoint);
                    if ( mJointTrail.size() > 200 ) {
                        mJointTrail.pop_front();
                    }
                }
                
                if (joint0.getType() == nite::JOINT_LEFT_KNEE) {
                    mLeftKnee = joint0.getPosition();
                } else if ( joint0.getType() == nite::JOINT_RIGHT_KNEE ) {
                    mRightKnee = joint0.getPosition();
                } else if ( joint0.getType() == nite::JOINT_TORSO ) {
                    mTorso = joint0.getPosition();
                }
                
                const nite::SkeletonJoint& joint1 = skeleton.getJoint( mBones[i].mJointB );
                
                Vec3f v0 = OpenNI::toVec3f( joint0.getPosition() );
                //				Vec3f v1 = OpenNI::toVec3f( joint1.getPosition() );
                v0.x = -v0.x;
                //				v1.x = -v1.x;
                
                gl::vertex( v0 );
                // gl::vertex( v1 );
                gl::end();
            }
            
            
            
            
            
            // draw negative space around the dancer
            
            if (mShowNegativeSpace) {

                gl::begin( GL_POLYGON );
                
                gl::enableAlphaBlending();
                gl::color( ColorA(1.0f, 0.25f, 1.0f, 0.3f) );
                gl::lineWidth(5.0f);
                gl::disableAlphaBlending();
                
                for ( int i = 20; i < mBones.size(); i++ ) {
                    
                    const nite::SkeletonJoint& joint0 = skeleton.getJoint( mBones[i].mJointA );
                    const nite::SkeletonJoint& joint1 = skeleton.getJoint( mBones[i].mJointB );
                    
                    Vec3f v0 = OpenNI::toVec3f( joint0.getPosition() );
                    Vec3f v1 = OpenNI::toVec3f( joint1.getPosition() );
                    v0.x = -v0.x;
                    v1.x = -v1.x;
                
                    // PRINT DISTANCES
                   //  Vec3f distPoint = v0 - v1;
                   //  float dist = sqrt( distPoint.x * distPoint.x + distPoint.y * distPoint.y );
                   //  cout << "the distance between " << mBones[i].mJointA << " and " << mBones[i].mJointB << " is " << dist << endl;
                    
                    gl::vertex( v0 );
                    gl::vertex( v1 );
                    
                }
                gl::end();
                
            }
            
            
            // draw distance lines
            
            if (mShowDistanceLines && !mShowNegativeSpace) {

             gl::begin( GL_LINES );
                gl::enableAlphaBlending();
                gl::color( ColorA(1.0f, 1.0f, 1.0f, 0.3f) );
                gl::lineWidth(5.0f);
                gl::disableAlphaBlending();
                
            for ( int i = 15; i <= 19; i++ ) {
                
                const nite::SkeletonJoint& joint0 = skeleton.getJoint( mBones[i].mJointA );
                const nite::SkeletonJoint& joint1 = skeleton.getJoint( mBones[i].mJointB );
                
                Vec3f v0 = OpenNI::toVec3f( joint0.getPosition() );
                Vec3f v1 = OpenNI::toVec3f( joint1.getPosition() );
                v0.x = -v0.x;
                v1.x = -v1.x;
                
                // PRINT DISTANCES
                Vec3f distPoint = v0 - v1;
                float dist = sqrt( distPoint.x * distPoint.x + distPoint.y * distPoint.y );
                cout << "the distance between " << mBones[i].mJointA << " and " << mBones[i].mJointB << " is " << dist << endl;
                
                gl::vertex( v0 );
                gl::vertex( v1 );
            }
                gl::end();
            }
        }
    }
    gl::setMatrices( mCamera );
    glBegin( GL_LINE_STRIP );
    glLineWidth(20.0f);
    gl::color( Color( 1.0f, 0.08f, 0.58f) );
    for( Vec3f v: mJointTrail ) {
        gl::vertex( v );
    }
    glEnd();
    
    // show if dancer is on or off balance
    if ( mUseBalance ) {
        mShapeDetection.onBalance( mLeftKnee, mRightKnee, mTorso );
    }
    
    // draw contour points
    mShapeDetection.draw( mUseBalance, mShowNegativeSpace );
    
    // draw gui params
    mParams.draw();
}

void UserApp::keyDown( KeyEvent event )
{
    switch ( event.getCode() ) {
        case KeyEvent::KEY_q:
            quit();
            break;
        case KeyEvent::KEY_f:
            setFullScreen( !isFullScreen() );
            break;
        case KeyEvent::KEY_s:
            screenShot();
            break;
    }
}

void UserApp::onUser( nite::UserTrackerFrameRef frame, const OpenNI::DeviceOptions& deviceOptions )
{
    mShapeDetection.onDepth( frame.getDepthFrame(), deviceOptions );
    mChannel	= OpenNI::toChannel16u( frame.getDepthFrame() );
    mUsers		= OpenNI::toVector( frame.getUsers() );
    for ( vector<nite::UserData>::iterator iter = mUsers.begin(); iter != mUsers.end(); ++iter ) {
        if ( iter->isNew() ) {
            mDevice->getUserTracker().startSkeletonTracking( iter->getId() );
        } else if ( iter->isLost() ) {
            mDevice->getUserTracker().stopSkeletonTracking( iter->getId() );
        }
    }
}

void UserApp::prepareSettings( Settings* settings )
{
    settings->setFrameRate( 60.0f );
    settings->setWindowSize( 800, 600 );
}

void UserApp::screenShot()
{
    writeImage( getAppPath() / fs::path( "frame" + toString( getElapsedFrames() ) + ".png" ), copyWindowSurface() );
}

void UserApp::setup()
{
    mShapeDetection = ShapeDetection();
    
    // create vector of joint names
    vector<string> joints = { "Head", "Neck", "Left Shoulder", "Right Shoulder", "Left Elbow", "Right Elbow", "Left Hand", "Right Hand", "Torso", "Left Hip", "Right Hip", "Left Knee", "Right Knee", "Left Foot", "Right Foot" };
    
    //	mBones.push_back( Bone( nite::JOINT_HEAD,			nite::JOINT_NECK ) );
    //	mBones.push_back( Bone( nite::JOINT_LEFT_SHOULDER,	nite::JOINT_LEFT_ELBOW ) );
    //	mBones.push_back( Bone( nite::JOINT_LEFT_ELBOW,		nite::JOINT_LEFT_HAND ) );
    //	mBones.push_back( Bone( nite::JOINT_RIGHT_SHOULDER, nite::JOINT_RIGHT_ELBOW ) );
    //	mBones.push_back( Bone( nite::JOINT_RIGHT_ELBOW,	nite::JOINT_RIGHT_HAND ) );
    //	mBones.push_back( Bone( nite::JOINT_LEFT_SHOULDER,	nite::JOINT_RIGHT_SHOULDER ) );
    //	mBones.push_back( Bone( nite::JOINT_LEFT_SHOULDER,	nite::JOINT_TORSO ) );
    //	mBones.push_back( Bone( nite::JOINT_RIGHT_SHOULDER, nite::JOINT_TORSO ) );
    //	mBones.push_back( Bone( nite::JOINT_TORSO,			nite::JOINT_LEFT_HIP ) );
    //	mBones.push_back( Bone( nite::JOINT_TORSO,			nite::JOINT_RIGHT_HIP ) );
    //	mBones.push_back( Bone( nite::JOINT_LEFT_HIP,		nite::JOINT_RIGHT_HIP ) );
    //	mBones.push_back( Bone( nite::JOINT_LEFT_HIP,		nite::JOINT_LEFT_KNEE ) );
    //	mBones.push_back( Bone( nite::JOINT_LEFT_KNEE,		nite::JOINT_LEFT_FOOT ) );
    //	mBones.push_back( Bone( nite::JOINT_RIGHT_HIP,		nite::JOINT_RIGHT_KNEE ) );
    //	mBones.push_back( Bone( nite::JOINT_RIGHT_KNEE,		nite::JOINT_RIGHT_FOOT ) );
    
    // POINTS
    mBones.push_back( Bone( nite::JOINT_HEAD,			nite::JOINT_HEAD ) );
    mBones.push_back( Bone( nite::JOINT_NECK,			nite::JOINT_NECK ) );
    mBones.push_back( Bone( nite::JOINT_LEFT_SHOULDER,	nite::JOINT_LEFT_SHOULDER ) );
    mBones.push_back( Bone( nite::JOINT_LEFT_ELBOW,     nite::JOINT_LEFT_ELBOW ) );
    mBones.push_back( Bone( nite::JOINT_LEFT_HAND,		nite::JOINT_LEFT_HAND ) );
    mBones.push_back( Bone( nite::JOINT_RIGHT_SHOULDER, nite::JOINT_RIGHT_SHOULDER ) );
    mBones.push_back( Bone( nite::JOINT_RIGHT_ELBOW,    nite::JOINT_RIGHT_ELBOW ) );
    mBones.push_back( Bone( nite::JOINT_RIGHT_HAND,     nite::JOINT_RIGHT_HAND ) );
    mBones.push_back( Bone( nite::JOINT_TORSO,          nite::JOINT_TORSO ) );
    mBones.push_back( Bone( nite::JOINT_LEFT_HIP,		nite::JOINT_LEFT_HIP ) );
    mBones.push_back( Bone( nite::JOINT_LEFT_KNEE,		nite::JOINT_LEFT_KNEE ) );
    mBones.push_back( Bone( nite::JOINT_LEFT_FOOT,		nite::JOINT_LEFT_FOOT ) );
    mBones.push_back( Bone( nite::JOINT_RIGHT_HIP,		nite::JOINT_RIGHT_HIP ) );
    mBones.push_back( Bone( nite::JOINT_RIGHT_KNEE,		nite::JOINT_RIGHT_KNEE ) );
    mBones.push_back( Bone( nite::JOINT_RIGHT_FOOT,		nite::JOINT_RIGHT_FOOT ) );
    
    // DISTANCE LINES
    
    // hand to hand
    mBones.push_back( Bone( nite::JOINT_LEFT_HAND,	nite::JOINT_RIGHT_HAND ) );
    
    // limbs to center
    mBones.push_back( Bone( nite::JOINT_LEFT_HAND,	nite::JOINT_TORSO ) );
    mBones.push_back( Bone( nite::JOINT_RIGHT_HAND,	nite::JOINT_TORSO ) );
    mBones.push_back( Bone( nite::JOINT_LEFT_FOOT,	nite::JOINT_TORSO ) );
    mBones.push_back( Bone( nite::JOINT_RIGHT_FOOT,	nite::JOINT_TORSO ) );
    
    //surrounding body
    mBones.push_back( Bone( nite::JOINT_RIGHT_FOOT,	nite::JOINT_LEFT_FOOT ) );
    mBones.push_back( Bone( nite::JOINT_LEFT_FOOT,	nite::JOINT_LEFT_HAND ) );
    mBones.push_back( Bone( nite::JOINT_LEFT_HAND,	nite::JOINT_HEAD ) );
    mBones.push_back( Bone( nite::JOINT_HEAD,	nite::JOINT_RIGHT_HAND ) );
    mBones.push_back( Bone( nite::JOINT_RIGHT_HAND,	nite::JOINT_RIGHT_FOOT) );
    
    mCamera = CameraPersp( getWindowWidth(), getWindowHeight(), 45.0f, 1.0f, 5000.0f );
    mCamera.lookAt( Vec3f::zero(), Vec3f::zAxis(), Vec3f::yAxis() );
    
    mDeviceManager = OpenNI::DeviceManager::create();
    try {
        mDevice = mDeviceManager->createDevice( OpenNI::DeviceOptions().enableUserTracking() );
    } catch ( OpenNI::ExcDeviceNotAvailable ex ) {
        console() << "exception with openni: " << ex.what() << endl;
        quit();
        return;
    }
    
    mDevice->getUserTracker().setSkeletonSmoothingFactor( 0.5f );
    mDevice->connectUserEventHandler( &UserApp::onUser, this );
    mDevice->start();
    
    // params window
    mParams = params::InterfaceGl( "Parameters", Vec2i( 200, 200 ) );
    mParams.addParam( "On Balance", &mUseBalance );
    mParams.addParam( "Negative Space", &mShowNegativeSpace );
    mParams.addParam( "Distance Lines", &mShowDistanceLines );
    mParams.addParam( "Follow joints", joints, &mJointParam );
}

CINDER_APP_BASIC( UserApp, RendererGl )
