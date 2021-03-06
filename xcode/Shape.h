//
//  Shape.h
//  UserApp
//
//  Created by Kathleen Sullivan on 8/28/15.
//
//

#pragma once
#include "CinderOpenCV.h"
#include "TrailPoint.h"

using namespace ci;
using namespace ci::app;
using namespace std;

class Shape {
public:
    Shape();
    
    int ID;
    double area;
    float depth;
    cv::Point centroid; // center point of the shape
    
    Boolean matchFound;
    bool mOffBalance;
    bool moving;
    int stillness;
    float motion;
    cv::vector<cv::Point> hull; // stores point representing the hull of the shape
    Vec2f pos;
    int lastFrameSeen;  // mark the last frame where the blob was seen, used to track when shapes leave the frame
    
    TrailPoint mTrailPoint;
};
