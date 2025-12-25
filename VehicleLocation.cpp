/* ******************************************************************
 * Copyright 2014-2024 ADVENTURE Project
 * All Rights Reserved
 * ****************************************************************** */
/**
 * @file VehicleLocation.cpp
 */
#include "VehicleLocation.hpp"
#include "AppMates.hpp"
#include "Intersection.hpp"
#include "Lane.hpp"
#include "RoadMap.hpp"
#include "Section.hpp"
#include "Vehicle.hpp"
#include <cstdlib>

using namespace std;
using namespace amu::geometry;
using namespace amu::math;

//======================================================================
VehicleLocation::VehicleLocation()
{
    _vehicle = nullptr;

    _roadMap          = nullptr;
    _intersection     = nullptr;
    _prevIntersection = nullptr;
    _section          = nullptr;
    _lane             = nullptr;
    _nextLane         = nullptr;
    _prevLane         = nullptr;

    _distance    = 0;
    _oldDistance = -10;
    _error       = 0;

    _tripLength               = 0;
    _distanceFromInflowBorder = 0;

    _generationTime    = AppMates::getTimeManager().time();
    _startingTime      = 0;
    _sectionInflowTime = 0;
}

//======================================================================
ulint VehicleLocation::startingStep() const
{
    return _startingTime / AppMates::getTimeManager().unit();
}

//======================================================================
AmuPoint VehicleLocation::position() const
{
    // レーンに沿う成分
    // Vector component along the lane
    AmuPoint p = _lane->position(_distance);

    // レーンに垂直な成分
    // Vector component perpendicular to the lane
    AmuVector nv = _lane->lineSegment()->directionVector(_distance);
    nv.normalize();
    nv.revoltXY(M_PI_2);
    p += _error * nv;

    return p;
}

//======================================================================
void VehicleLocation::print(ostream& out) const
{
    stringstream ss;

    if (_section)
    {
        ss << "Section ID: " << _section->id() << ", ";
    }
    else if (_intersection)
    {
        ss << "Intersection ID: " << _intersection->id() << ", ";
    }
    else
    {
        ss << "Section/Intersection is NULL, ";
    }
    if (_lane)
    {
        ss << "Lane ID: " << _lane->id();
    }
    else
    {
        ss << "Lane is NULL";
    }
    if (_nextLane)
    {
        ss << ", NextLane: " << _nextLane->id() << " in "
           << _nextLane->parent()->id();
    }
    ss << endl;
    ss << "Distance, Error: (" << _distance << ", " << _error << ")"
       << endl;
    ss << "Position(x, y, z): (" << x() << ", " << y() << ", " << z()
       << ")" << endl;

#pragma omp critical(out_critical)
    out << ss.str();
}
