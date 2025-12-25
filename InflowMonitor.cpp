/* **************************************************************************
 * Copyright 2014-2024 ADVENTURE Project
 * All Rights Reserved
 * ************************************************************************** */
/**
 * @file InflowMonitor.cpp
 */
#include "InflowMonitor.hpp"
#include "CustomMessage.hpp"
#include "Lane.hpp"
#include "ODNode.hpp"
#include "Section.hpp"
#include "Vehicle.hpp"
#include "VehicleBodyProperty.hpp"
#include <sstream>

using namespace std;

//==============================================================================
void InflowMonitor::recordInflowVehicle(
    const Lane* lane, const Vehicle* vehicle, ulint headway, ulint genTime,
    ulint genInterval)
{
    InflowMonitor::Record* record = new InflowMonitor::Record(
        _odNode->id(), lane->id(), vehicle->id(), *(vehicle->body()->type()),
        headway, genTime, genInterval, vehicle->globalRoute()->start()->id(),
        vehicle->globalRoute()->goal()->id());
    _records.emplace_back(record);
}

//==============================================================================
void InflowMonitor::print(ostream& out) const
{
    ostringstream ss;
    ss << _id << ": intersection[" << _odNode->id() << "] | section["
       << _section->id() << "]";
    amu::msg::message(out, ss.str());
}
