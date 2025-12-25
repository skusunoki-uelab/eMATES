/* ******************************************************************
 * Copyright 2014-2024 ADVENTURE Project
 * All Rights Reserved
 * ****************************************************************** */
/**
 * @file VehicleLocalRoute.cpp
 */
#include "VehicleLocalRoute.hpp"
#include "Intersection.hpp"
#include "Lane.hpp"
#include "LaneBundle.hpp"
#include "Section.hpp"
#include "Vehicle.hpp"
#include "VehicleLocation.hpp"
#include <AmuConverter.hpp>
#include <algorithm>
#include <iostream>

using namespace std;
using namespace amu::converter;
using LP = LanePosition;

//======================================================================
void VehicleLocalRoute::getNextLanes(
    const Lane* currentLane, vector<const Lane*>& result_lanes) const
{
    for (auto itr = _localRoute.rbegin(); itr != _localRoute.rend();
         itr++)
    {
        if (*itr == currentLane)
        {
            break;
        }
        result_lanes.emplace_back(*itr);
    }
    reverse(result_lanes.begin(), result_lanes.end());
}

//======================================================================
void VehicleLocalRoute::getPreviousLanes(
    const Lane* currentLane, vector<const Lane*>& result_lanes) const
{
    for (auto itr = _localRoute.begin(); itr != _localRoute.end();
         itr++)
    {
        if (*itr == currentLane)
        {
            break;
        }
        result_lanes.emplace_back(*itr);
    }
}

//======================================================================
void VehicleLocalRoute::print(std::ostream& out) const
{
    stringstream ss;
    ss.str("");

    vector<const Lane*>::const_iterator where;
    ss << "Local Route:" << endl;
    for (where = _localRoute.begin(); where != _localRoute.end();
         where++)
    {
        ss << "\t" << (*where)->id() << " in "
           << (*where)->parent()->id() << endl;
    }
    ss << "Target Lane to Shift: "
       << (_targetLane ? _targetLane->id() : "NULL") << ", "
       << "Target Direction: "
       << (_targetDirection == LP::Center
               ? "current"
               : (_targetDirection == LP::Left ? "left" : "right"))
       << endl;
    ss << "MainLane: ";
    if (_mainLaneInIntersection)
    {
        ss << _mainLaneInIntersection->id() << endl;
    }
    else
    {
        ss << "NULL" << endl;
    }
    ss << "Lanes in intersection:" << endl;
    for (where = _lanesInIntersection.begin();
         where != _lanesInIntersection.end(); where++)
    {
        ss << "\t" << (*where)->id() << endl;
    }
    ss << "Turning: " << _turning << endl;

    // 交錯レーンについて表示する
    // Show about crossing lanes
    const VehicleLocation* location = _vehicle->location();
    if (location->section() != NULL)
    {
        Intersection* nextInter = location->section()->intersection(
            location->section()->isUp(location->lane()));
        if (nextInter)
        {
            vector<const Lane*> cli;
            vector<const Lane*> cls;
            nextInter->getCollisionLanes(
                _lanesInIntersection, cli, cls);
            ss << "Collision Lanes in Intersection:" << endl;
            for (unsigned int i = 0; i < cli.size(); i++)
            {
                ss << "\t" << cli[i]->id() << endl;
            }
            ss << "Collision Lanes in Section:" << endl;
            for (unsigned int i = 0; i < cls.size(); i++)
            {
                ss << "\t" << cls[i]->id() << " of section "
                   << nextInter
                          ->nextSection(nextInter->direction(
                              cls[i]->endConnector()))
                          ->id()
                   << endl;
            }
        }
    }

#pragma omp critical(out_critical)
    out << ss.str();
}
