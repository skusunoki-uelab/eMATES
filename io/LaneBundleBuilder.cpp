/* ******************************************************************
 * Copyright 2014-2024 ADVENTURE Project
 * All Rights Reserved
 * ****************************************************************** */
/**
 * @file LaneBundleBuilder.cpp
 */
#include "LaneBundleBuilder.hpp"
#include "../Intersection.hpp"
#include "../Lane.hpp"
#include "../Section.hpp"
#include "../SubLaneBundle.hpp"
#include <cfloat>

using namespace std;

//======================================================================
bool LaneBundleBuilder::setLaneConnection()
{
    for (auto itr : _bundle->lanes())
    {
        Lane* lane = itr.second;
        if (!(_decideNextLanes(lane)) || !(_decidePrevLanes(lane)))
        {
            return false;
        }
    }
    return true;
}

//======================================================================
bool LaneBundleBuilder::_decideNextLanes(Lane* lane)
{
    const Connector*    endConnector = lane->endConnector();
    vector<const Lane*> nextLanes
        = lane->parent()->lanesFromConnector(endConnector);

    // endConnectorがレーン束の境界にある場合は下流にも尋ねる
    // Also ask downstream if endConnector is on lane bundle boundary
    if (nextLanes.empty())
    {
        Section* parentSection = dynamic_cast<Section*>(lane->parent());
        if (parentSection)
        {
            /*
             * 処理対象が単路部内にあり，下流が交差点である場合
             *
             * The case that the processing target is in a section
             * and the downstream is an intersection
             */
            Intersection* nextIntersection
                = parentSection->intersection(
                    parentSection->isUp(lane));
            if (nextIntersection)
            {
                nextLanes = nextIntersection->lanesFromConnector(
                    endConnector);
            }
        }
        else
        {
            /*
             * 処理対象が交差点にあり，下流が単路部である場合
             *
             * The case that the processing target is in an intersection
             * and the downstream is a section
             */
            Intersection* parentIntersection
                = dynamic_cast<Intersection*>(lane->parent());
            if (parentIntersection->direction(endConnector) != -1)
            {
                Section* nextSection = parentIntersection->nextSection(
                    parentIntersection->direction(endConnector));
                if (nextSection)
                {
                    nextLanes
                        = nextSection->lanesFromConnector(endConnector);
                }
            }
        }
    }

    if (nextLanes.empty())
    {
        // ODNodeではnextLaneが求まらない
        // Cannot find nextLane in ODNode
        return true;
    }

    const Lane* nextStraightLane = nullptr;
    double      minAngle         = DBL_MAX;
    for (auto itr : nextLanes)
    {
        lane->addNextLane(itr);

        // ついでに角度最小のレーンを求める
        // In addition, find the lane with the smallest angle
        double angle
            = fabs(lane->lineSegment()->directionVector().calcAngle(
                itr->directionVector()));
        if (angle < minAngle)
        {
            minAngle         = angle;
            nextStraightLane = itr;
        }
    }

    lane->setNextStraightLane(nextStraightLane);
    return true;
}

//======================================================================
bool LaneBundleBuilder::_decidePrevLanes(Lane* lane)
{
    const Connector*    beginConnector = lane->beginConnector();
    vector<const Lane*> prevLanes
        = lane->parent()->lanesToConnector(beginConnector);

    // beginConnectorがLaneBundleの境界にある場合は上流にも尋ねる
    // Also ask upstream if beginConnector is on lane bundle boundary
    if (prevLanes.empty())
    {
        Section* parentSection = dynamic_cast<Section*>(lane->parent());
        if (parentSection)
        {
            /*
             * 処理対象が単路部内にあり，上流が交差点である場合
             *
             * The case that the processing target is in a section
             * and the upstream is an intersection
             */
            Intersection* prevIntersection
                = parentSection->intersection(
                    !(parentSection->isUp(lane)));
            if (prevIntersection)
            {
                prevLanes = prevIntersection->lanesToConnector(
                    beginConnector);
            }
        }
        else
        {
            /*
             * 処理対象が交差点にあり，上流が単路部である場合
             *
             * The case that the processing target is in an intersection
             * and the upstream is a section
             */
            Intersection* parentIntersection
                = dynamic_cast<Intersection*>(lane->parent());
            if (parentIntersection->direction(beginConnector) != -1)
            {
                Section* prevSection = parentIntersection->nextSection(
                    parentIntersection->direction(beginConnector));
                if (prevSection)
                {
                    prevLanes
                        = prevSection->lanesToConnector(beginConnector);
                }
            }
        }
    }

    if (prevLanes.empty())
    {
        // ODNodeではprevLaneが求まらない
        // Cannot find prevLane in ODNode
        return true;
    }

    const Lane* prevStraightLane = nullptr;
    double      minAngle         = DBL_MAX;
    for (auto itr : prevLanes)
    {
        lane->addPreviousLane(itr);

        // ついでに角度最小のレーンを求める
        // In addition, find the lane with the smallest angle
        double angle
            = fabs(lane->lineSegment()->directionVector().calcAngle(
                itr->directionVector()));
        if (angle < minAngle)
        {
            minAngle         = angle;
            prevStraightLane = itr;
        }
    }

    lane->setPreviousStraightLane(prevStraightLane);
    return true;
}

//======================================================================
bool LaneBundleBuilder::setLaneCollision()
{
    return true;
}

//======================================================================
bool LaneBundleBuilder::createSubnetwork()
{
    for (auto itr : _bundle->subLaneBundles())
    {
        for (int i = 0; i < itr.second->numVertexes(); i++)
        {
            SubLaneBundle* subsec
                = _bundle->pairedSubLaneBundle(itr.second, i);
            if (subsec)
            {
                itr.second->addAdjSubLaneBundle(i, subsec);
            }
        }
    }
    return true;
}

//======================================================================
bool LaneBundleBuilder::assignLanesToSubLaneBundles()
{
    for (auto itr_l : _bundle->lanes())
    {
        bool isIncluded = false;
        for (auto itr_s : _bundle->subLaneBundles())
        {
            if (itr_s.second->includes(itr_l.second))
            {
                itr_s.second->addLane(itr_l.second);
#ifdef INCLUDE_PEDESTRIANS
                itr_l.second->pedExt()->setSubLaneBundle(itr_s.second);
#endif //INCLUDE_PEDESTRIANS
                assert(!isIncluded);
                isIncluded = true;
            }
        }
        if (!isIncluded)
        {
            cout << itr_l.second->id() << "(@"
                 << itr_l.second->parent()->id()
                 << ") is not included in any subsections." << endl;
        }
    }
    return true;
}
