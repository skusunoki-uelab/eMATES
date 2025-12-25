/* **************************************************************************
 * Copyright 2014-2024 ADVENTURE Project
 * All Rights Reserved
 * ************************************************************************** */
/**
 * @file LaneInIntersection.cpp
 */
#include "LaneInIntersection.hpp"
#include "Config.hpp"
#include "CustomMessage.hpp"
#include "Intersection.hpp"
#include "Section.hpp"
#include <iostream>
#include <cassert>

using namespace std;
using namespace amu::geometry;

//==============================================================================
LaneInIntersection::LaneInIntersection(
    const std::string& id, const Connector* ptBegin, const Connector* ptEnd,
    AmuLineSegment* ptLineSegment, LaneBundle* parent)
    : Lane(id, ptBegin, ptEnd, ptLineSegment, parent)
{
}

//==============================================================================
void LaneInIntersection::print(ostream& out) const
{
    Lane::print(out);

    ostringstream oss;
    if (!_collisionLanesInIntersection.empty())
    {
        oss << "CollisionLanesInIntersection:" << endl;
        for (auto itr : _collisionLanesInIntersection)
        {
            oss << "  ID: " << itr->id() << endl;
        }
    }
    else
    {
        oss << "CollisionLanesInIntersection: none" << endl;
    }

    if (!_collisionLanesInSection.empty())
    {
        oss << "CollisionLanesInSection:" << endl;
        for (auto itr : _collisionLanesInSection)
        {
            oss << "  ID: " << itr->id() << "(@" << itr->parent()->id() << ")"
                << endl;
        }
    }
    else
    {
        oss << "CollisionLanesInSection: none" << endl;
    }
    amu::msg::message(out, oss.str());
}
