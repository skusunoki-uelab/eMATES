/* ******************************************************************
 * Copyright 2014-2024 ADVENTURE Project
 * All Rights Reserved
 * ****************************************************************** */
/**
 * @file TramLaneInIntersection.cpp
 */
#ifdef INCLUDE_TRAMS
#include "TramLaneInIntersection.hpp"
#include "../LaneBundle.hpp"
#include "../Connector.hpp"

using namespace std;
using namespace amu::geometry;

//======================================================================
TramLaneInIntersection::TramLaneInIntersection(
    const string& id, const Connector* ptBegin, const Connector* ptEnd,
    AmuLineSegment* lineSegment, LaneBundle* parent)
    : LaneInIntersection(id, ptBegin, ptEnd, lineSegment, parent)
{
}

//======================================================================
TramLaneInIntersection::~TramLaneInIntersection() {}

#endif //INCLUDE_TRAMS
