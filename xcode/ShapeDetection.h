//
//  ShapeDetection.h
//  UserApp
//
//  Created by Kat Sullivan on 8/28/15.
//
//

#include "cinder/app/AppNative.h"
#include "cinder/gl/Texture.h"
#include "cinder/gl/gl.h"
#include "Cinder-OpenNI.h"
#include "CinderOpenCV.h"
#include "cinder/Rand.h"
#include "Shape.h"
//#include "json/json.h"
#include "cinder/gl/Fbo.h"

using namespace ci;
using namespace ci::app;
using namespace std;

class ShapeDetection {
public:
    ShapeDetection();
    void draw( bool useBalance, bool showNegativeSpace );
    void onDepth( openni::VideoFrameRef frame, const OpenNI::DeviceOptions& deviceOptions );
    void onBalance( nite::Point3f leftKnee, nite::Point3f rightKnee, nite::Point3f torso );
    
    cv::Mat mInput;
    
    vector<Shape> mTrackedShapes;   // store tracked shapes
    
    // all pixels below near limit and above far limit are set to far limit depth
    short mNearLimit;
    short mFarLimit;
    
    // threshold for the camera
    double mThresh;
    double mMaxVal;
    
    bool mDrawShapes;   // boolean for whether you draw shapes or points
    
    gl::TextureRef mTexture;
    gl::TextureRef mTextureDepth;
    ci::Surface8u mSurfaceSubtract;

    
  //  bool mCameraPresent;    // bool for whether camera was started without error
    
private:
   // OpenNI::DeviceRef mDevice;
   // OpenNI::DeviceManagerRef mDeviceManager;
   // gl::TextureRef mTexture;
    //gl::TextureRef mTextureDepth;
    
    ci::Surface8u mSurface;
    ci::Surface8u mSurfaceDepth;

    ci::Surface8u mSurfaceBlur;
        
    typedef vector< vector<cv::Point > > ContourVector;
    ContourVector mContours;
    ContourVector mApproxContours;
    int shapeUID;
    
    cv::vector<cv::Vec4i> mHierarchy;
    vector<Shape> mShapes;
    
    //void onColor( openni::VideoFrameRef frame, const OpenNI::DeviceOptions& deviceOptions );
    vector< Shape > getEvaluationSet( ContourVector rawContours, int minimalArea, int maxArea );
    Shape* findNearestMatch( Shape trackedShape, vector< Shape > &shapes, float maximumDistance );
    cv::Mat removeBlack( cv::Mat input, short nearLimit, short farLimit );
    //void screenShot();
};
