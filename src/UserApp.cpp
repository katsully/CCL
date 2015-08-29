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
    float mRightKneeX;
    float mLeftKneeX;
    cv::Point mTorso;
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
    bool mUseBalance;
    bool mShowNegativeSpace;
    
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
    if ( mChannel ) {
        if ( mTexture ) {
            mTexture->update( Channel32f( mChannel ) );
        } else {
            mTexture = gl::Texture::create( Channel32f( mChannel ) );
        }
        gl::draw( mTexture, mTexture->getBounds(), getWindowBounds() );
    }
    
    gl::setMatrices( mCamera );
    
    
    gl::color( Colorf( 1.0f, 0.0f, 0.0f ) );
    for ( std::vector<nite::UserData>::const_iterator iter = mUsers.begin(); iter != mUsers.end(); ++iter )
    {
        const nite::Skeleton& skeleton = iter->getSkeleton();
        
        if ( skeleton.getState() == nite::SKELETON_TRACKED ) {
            
            // DRAW POINTS
            gl::enableAlphaBlending();
            gl::color( ColorA(1.0f, 1.0f, 1.0f, 0.3f) );
            glPointSize(10.0f);
            gl::disableAlphaBlending();
            
            gl::begin( GL_POINTS );
            
            // this draws the first 15 points (so no joints get drawn twice)
            for ( int i=0; i<15; i++ ) {
                const nite::SkeletonJoint& joint0 = skeleton.getJoint( mBones[i].mJointA );

                if (joint0.getType() == nite::JOINT_LEFT_KNEE) {
                    mLeftKneeX = -joint0.getPosition().x;
                } else if ( joint0.getType() == nite::JOINT_RIGHT_KNEE ) {
                    mRightKneeX = -joint0.getPosition().x;
                } else if ( joint0.getType() == nite::JOINT_TORSO ) {
                    mTorso = cv::Point( -joint0.getPosition().x, joint0.getPosition().y );
                }

				const nite::SkeletonJoint& joint1 = skeleton.getJoint( mBones[i].mJointB );

				Vec3f v0 = OpenNI::toVec3f( joint0.getPosition() );
				Vec3f v1 = OpenNI::toVec3f( joint1.getPosition() );
				v0.x = -v0.x;
				v1.x = -v1.x;
                
                // PRINT VALUES
                if (mBones[i].mJointA == mBones[i].mJointA){
//                    console() << iter->mJointA << " X:" << v0.x << " Y:" << v0.y << endl;
                }
                
                gl::vertex( v0 );
                gl::vertex( v1 );
            }
            gl::end();
            
            // draw negative space around the dancer
            if (mShowNegativeSpace) {
                // DRAW DISTANCE LINES
                gl::begin( GL_TRIANGLE_FAN );
                
                gl::enableAlphaBlending();
                gl::color( ColorA(1.0f, 0.25f, 1.0f, 0.3f) );
                gl::lineWidth(5.0f);
                
                //  this draws the distance lines (and not the inidivual joints)
                for ( int i = 15; i < mBones.size(); i++ ) {
                    gl::color( ColorA(1.0f, 0.25f, 1.0f, 0.3f) );
                    const nite::SkeletonJoint& joint0 = skeleton.getJoint( mBones[i].mJointA );
                    const nite::SkeletonJoint& joint1 = skeleton.getJoint( mBones[i].mJointB );
                    
                    Vec3f v0 = OpenNI::toVec3f( joint0.getPosition() );
                    Vec3f v1 = OpenNI::toVec3f( joint1.getPosition() );
                    v0.x = -v0.x;
                    v1.x = -v1.x;
                    
                    // PRINT DISTANCES
                    //  console() << iter->mJointA << " X: " << v0.x << " Y: " << v0.y << endl;
//                    if (mBones[i].mJointA == 15){
                        // i wanna prrint the distance instead of X & Y
                        //                    console() << iter->mJointA << " X:" << v0.x << " Y:" << v0.y << endl;
//                    }
                    Vec3f distPoint = v0 - v1;
                    float dist = sqrt( distPoint.x * distPoint.x + distPoint.y * distPoint.y );
                    cout << "the distance between " << mBones[i].mJointA << " and " << mBones[i].mJointB << " is " << dist << endl;
                    
                    gl::vertex( v0 );
                    gl::vertex( v1 );
                    
                }
                gl::disableAlphaBlending();
                gl::end();
            }
        }
    }

    // show if dancer is on or off balance
    if ( mUseBalance ) {
        mShapeDetection.onBalance( mLeftKneeX, mRightKneeX, mTorso );
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
//    mBones.push_back( Bone( nite::JOINT_LEFT_HAND,	nite::JOINT_RIGHT_HAND ) );
    //limbs to center
   // mBones.push_back( Bone( nite::JOINT_LEFT_HAND,	nite::JOINT_TORSO ) );
   // mBones.push_back( Bone( nite::JOINT_RIGHT_HAND,	nite::JOINT_TORSO ) );
//    mBones.push_back( Bone( nite::JOINT_LEFT_FOOT,	nite::JOINT_TORSO ) );
//    mBones.push_back( Bone( nite::JOINT_RIGHT_FOOT,	nite::JOINT_TORSO ) );
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
        console() << "exception with openni" << ex.what() << endl;
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
}

CINDER_APP_BASIC( UserApp, RendererGl )
