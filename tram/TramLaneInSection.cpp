/* ******************************************************************
 * Copyright 2014-2024 ADVENTURE Project
 * All Rights Reserved
 * ****************************************************************** */
/**
 * @file TramLaneInSection.cpp
 */
#ifdef INCLUDE_TRAMS
#include "TramLaneInSection.hpp"
#include "../Connector.hpp"
#include "../LaneBundle.hpp"

using namespace std;
using namespace amu::geometry;

//======================================================================
TramLaneInSection::TramLaneInSection(
    const string& id, const Connector* ptBegin, const Connector* ptEnd,
    AmuLineSegment* lineSegment, LaneBundle* parent)
    : LaneInSection(id, ptBegin, ptEnd, lineSegment, parent)
{
}

//======================================================================
TramLaneInSection::~TramLaneInSection() {}

#endif //INCLUDE_TRAMS
