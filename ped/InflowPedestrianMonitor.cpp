/* ******************************************************************
 * Copyright 2014-2024 ADVENTURE Project
 * All Rights Reserved
 * ****************************************************************** */
/**
 * @file InflowPedestrianMonitor.cpp
 */
#ifdef INCLUDE_PEDESTRIANS
#include "InflowPedestrianMonitor.hpp"
#include "Pedestrian.hpp"
#include "Zebra.hpp"
#include "ZebraODEdge.hpp"
#include "../CustomMessage.hpp"
#include "../LaneBundle.hpp"
#include "../SubIntersection.hpp"
#include "../SubLaneBundle.hpp"
#include <sstream>

using namespace std;

//======================================================================
void InflowPedestrianMonitor::recordInflowPedestrian(
    const Pedestrian* pedestrian, ulint headway, ulint genTime,
    ulint genInterval)
{
    InflowPedestrianMonitor::Record* record
        = new InflowPedestrianMonitor::Record(
            pedestrian->id(), headway, genTime, genInterval);
    _records.emplace_back(record);
}

//======================================================================
const LaneBundle* InflowPedestrianMonitor::laneBundle() const
{
    return _edge->zebra()->parent();
}

//======================================================================
const Zebra* InflowPedestrianMonitor::zebra() const
{
    return _edge->zebra();
}

//======================================================================
void InflowPedestrianMonitor::print(ostream& out) const
{
    ostringstream ss;
    ss << _id << ": lane_bundle[" << laneBundle()->id() << "], zebra["
       << zebra()->id() << "], dir(" << zebra()->directionInIntersection()
       << "), cross_dir(" << _edge->crossingDirection() << ")";
    amu::msg::message(out, ss.str());
}

#endif //INCLUDE_PEDESTRIANS
