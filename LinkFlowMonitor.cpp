/* **************************************************************************
 * Copyright 2014-2024 ADVENTURE Project
 * All Rights Reserved
 * ************************************************************************** */
/**
 * @file LinkFlowMonitor.cpp
 */
#include "LinkFlowMonitor.hpp"
#include "AppMates.hpp"
#include "CustomMessage.hpp"
#include "Vehicle.hpp"
#include <sstream>

using namespace std;

//==============================================================================
void LinkFlowMonitor::recordPassedVehicle(Vehicle* vehicle)
{
    assert(vehicle);

    PassRecord* record = new PassRecord(
        vehicle->location()->lane()->id(),            //
        vehicle->id(),                                //
        *(vehicle->body()->type()),                   //
        vehicle->globalRoute()->start()->id(),        //
        vehicle->globalRoute()->goal()->id(),         //
        (AppMates::getTimeManager().time()            //
         - vehicle->location()->sectionInflowTime()), //
        vehicle->behavior()->numPausing(),            //
        vehicle->behavior()->pauseDuration());
    _passRecords.emplace_back(record);
}

//==============================================================================
void LinkFlowMonitor::print(ostream& out) const
{
    ostringstream oss;
    oss << _id << ": section[" << _section->id() << "], is_up("
        << (_isUp ? "1" : "0") << "), intersections["
        << _section->intersection(!_isUp)->id() << "->"
        << _section->intersection(_isUp)->id() << "]";
    amu::msg::message(out, oss.str());
}
