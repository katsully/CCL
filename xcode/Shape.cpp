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
ID(-1),
lastFrameSeen(-1),
matchFound(false),
mOffBalance(false),
moving(false),
stillness(0),
motion(0.0f)
{
}