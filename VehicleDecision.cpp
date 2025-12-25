/* ******************************************************************
 * Copyright 2014-2024 ADVENTURE Project
 * All Rights Reserved
 * ****************************************************************** */
/**
 * @file VehicleDecision.cpp
 */
#include "VehicleDecision.hpp"
#include "AppMates.hpp"
#include "GVManager.hpp"
#include "Vehicle.hpp"
#include "VehicleLaneChangePerceiver.hpp"
#include "VehicleLocation.hpp"
#include <AmuVector.hpp>

using namespace std;
using namespace amu::math;

//======================================================================
VehicleDecision::VehicleDecision()
{
    _vehicle      = nullptr;
    _decidedAccel = 0;
    _shouldSleep  = false;

    _isLaneChangeRequired   = false;
    _isLaneChangeExecutable = false;
    _isLaneChangeActive     = false;
}

//======================================================================
void VehicleDecision::print(ostream& out) const
{
    stringstream ss;

    ss << "Should sleep: " << (_shouldSleep ? "true" : "false") << endl;

#pragma omp critical(out_critical)
    out << ss.str();
}

//======================================================================
void VehicleDecision::printLaneChangeFlags(ostream& out) const
{
    stringstream ss;

    ss << "Lane change required: " << _isLaneChangeRequired
       << ", executable: " << _isLaneChangeExecutable
       << ", active: " << _isLaneChangeActive << endl;

#pragma omp critical(out_critical)
    out << ss.str();
}
