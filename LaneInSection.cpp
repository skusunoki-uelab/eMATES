/* **************************************************************************
 * Copyright 2014-2024 ADVENTURE Project
 * All Rights Reserved
 * ************************************************************************** */
/**
 * @file LaneInSection.cpp
 */
#include "LaneInSection.hpp"
#include "Config.hpp"
#include "Section.hpp"
#include <cassert>
#include <iostream>

using namespace std;
using namespace amu::geometry;
using namespace amu::math;
using LP = LanePosition;

//==============================================================================
LaneInSection::LaneInSection(
    const std::string& id, const Connector* ptBegin, const Connector* ptEnd,
    AmuLineSegment* ptLineSegment, LaneBundle* parent)
    : Lane(id, ptBegin, ptEnd, ptLineSegment, parent)
{
}

//==============================================================================
bool LaneInSection::isSideLaneFound(const Lane* lane, LP::Type direction) const
{
    if (direction == LP::Left)
    {
        for (auto itr : _leftLanes)
        {
            if (itr.second == lane)
            {
                return true;
            }
        }
        return false;
    }
    else if (direction == LP::Right)
    {
        for (auto itr : _rightLanes)
        {
            if (itr.second == lane)
            {
                return true;
            }
        }
        return false;
    }
    else if (direction == LP::Center)
    {
        return false;
    }
    else
    {
        ostringstream sse;
        sse << "LaneInSection[" << _id << "(@" << _parent->id()
            << ")]::isSideLaneFound - bad direction specified." << endl;
        amu::msg::error(sse.str());
        exit(EXIT_FAILURE);
    }
}

//==============================================================================
void LaneInSection::getSideLaneDistance(
    double originDistance, LP::Type direction, const Lane** result_sideLane,
    double* result_distance) const
{
    if (direction == LP::Center || originDistance > this->length())
    {
        return;
    }

    // 探索線分の作成
    // Create search line
    AmuVector searchVector = directionVector();
    searchVector.normalize();

    AmuPoint beginPoint
        = _beginConnector->point() + searchVector * originDistance;
    if (direction == LP::Left)
    {
        searchVector.revoltXY(M_PI_2);
    }
    else if (direction == LP::Right)
    {
        searchVector.revoltXY(-M_PI_2);
    }
    else
    {
        ostringstream sse;
        sse << "LaneInSection[" << _id << "(@" << _parent->id()
            << ")]::getSideLaneDistance - bad direction specified." << endl;
        amu::msg::error(sse.str());
        exit(EXIT_FAILURE);
    }
    AmuLineSegment searchLine
        = AmuLineSegment(
              beginPoint,
              beginPoint + searchVector * SEARCH_SIDE_LANE_LINE_LENGTH)
              .z0();

    // 横レーンの取得
    // Get side lane
    AmuPoint crsPoint;
    if (direction == LP::Left)
    {
        for (auto itr : _leftLanes)
        {
            if (itr.first.includes(originDistance)
                && itr.second->lineSegment()->z0().createIntersectionPoint(
                    &searchLine, &crsPoint))
            {
                *result_sideLane = itr.second;
                *result_distance
                    = itr.second->beginConnector()->point().distance(crsPoint);
                return;
            }
        }
    }
    else
    {
        for (auto itr : _rightLanes)
        {
            if (itr.first.includes(originDistance)
                && itr.second->lineSegment()->z0().createIntersectionPoint(
                    &searchLine, &crsPoint))
            {
                *result_sideLane = itr.second;
                *result_distance
                    = itr.second->beginConnector()->point().distance(crsPoint);
                return;
            }
        }
    }
}

//==============================================================================
double LaneInSection::lengthOnSideLane(
    double originDistance, const Lane* sideLane) const
{
    ASSERT_MSG(originDistance <= this->length());

    // 探索線分の作成
    // Create search line
    AmuVector searchVector = directionVector();
    searchVector.normalize();

    AmuPoint beginPoint
        = _beginConnector->point() + searchVector * originDistance;
    searchVector.revoltXY(M_PI_2);

    AmuLineSegment searchLine
        = AmuLineSegment(
              beginPoint - searchVector * SEARCH_SIDE_LANE_LINE_LENGTH,
              beginPoint + searchVector * SEARCH_SIDE_LANE_LINE_LENGTH)
              .z0();

    // 横レーンの取得
    // Get side lane
    AmuPoint crsPoint;
    if (sideLane->lineSegment()->z0().createIntersectionPoint(
            &searchLine, &crsPoint))
    {
        return sideLane->beginConnector()->point().distance(crsPoint);
    }
    else
    {
        return originDistance;
    }
}

//==============================================================================
const Lane* LaneInSection::rightLane(double distance) const
{
    for (auto itr : _rightLanes)
    {
        if (itr.first.includes(distance))
        {
            return itr.second;
        }
    }
    return nullptr;
}

//==============================================================================
const Lane* LaneInSection::leftLane(double distance) const
{
    for (auto itr : _leftLanes)
    {
        if (itr.first.includes(distance))
        {
            return itr.second;
        }
    }
    return nullptr;
}

//==============================================================================
const Lane* LaneInSection::sideLane(LP::Type direction, double distance) const
{
    if (direction == LP::Left)
    {
        return leftLane(distance);
    }
    else if (direction == LP::Right)
    {
        return rightLane(distance);
    }
    else if (direction == LP::Center)
    {
        return nullptr;
    }
    else
    {
        ostringstream sse;
        sse << "LaneInSection[" << _id << "(@" << _parent->id()
            << ")]::sideLane - bad direction specified." << endl;
        exit(EXIT_FAILURE);
    }
}

//==============================================================================
void LaneInSection::print(ostream& out) const
{
    Lane::print(out);

    ostringstream oss;
    if (!_leftLanes.empty())
    {
        oss << "LeftLanes:" << endl;
        for (auto itr : _leftLanes)
        {
            oss << "  [" << itr.first.lower() << ", " << itr.first.upper()
                << "] - ID: " << itr.second->id() << endl;
        }
    }
    else
    {
        oss << "LeftLanes: none" << endl;
    }
    if (!_rightLanes.empty())
    {
        oss << "RightLanes:" << endl;
        for (auto itr : _rightLanes)
        {
            oss << "  [" << itr.first.lower() << ", " << itr.first.upper()
                << "] - ID: " << itr.second->id() << endl;
        }
    }
    else
    {
        oss << "RightLanes: none" << endl;
    }

    amu::msg::message(out, oss.str());
}
