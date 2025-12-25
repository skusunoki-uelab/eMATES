/* ******************************************************************
 * Copyright 2014-2024 ADVENTURE Project
 * All Rights Reserved
 * ****************************************************************** */
/**
 * @file VehicleBehavior.cpp
 */
#include "VehicleBehavior.hpp"
#include "AppMates.hpp"
#include "GVManager.hpp"
#include "Vehicle.hpp"
#include "VehicleLaneChangePerceiver.hpp"
#include "VehicleLocation.hpp"
#include <AmuVector.hpp>
#include <cstdint>

using namespace std;
using namespace amu::math;

//======================================================================
VehicleBehavior::VehicleBehavior()
{
    _vehicle = nullptr;

    _velocity      = 0;
    _errorVelocity = 0;
    _accel         = 0;
    _velocityRateHistory.clear();

    _laneTo = nullptr;

    _isNotifying = false;

    _isPausing  = false;
    _numPausing = 0;
    clearPauseStartTime(); //_pauseStartTime = UINT32_MAX;
    _pauseDuration = 0;
    _sleepDuration = 0;
}

//======================================================================
double VehicleBehavior::aveVelocityInSection() const
{
    return (
        _vehicle->location()->distanceFromInflowBorder()
        / (AppMates::getTimeManager().time()
           - _vehicle->location()->sectionInflowTime()));
}

//======================================================================
void VehicleBehavior::addVelocityRateHistory(double rate)
{
    _velocityRateHistory.push_back(rate);
    if (_velocityRateHistory.size()
        > AppMates::getGVManager().getNumeric(
            "VEHICLE_VELOCITY_HISTORY_SIZE"))
    {
        _velocityRateHistory.pop_front();
    }
}

//======================================================================
double VehicleBehavior::aveVelocityRate() const
{
    if (_velocityRateHistory.empty())
    {
        return 1.0;
    }

    double sum = accumulate(
        _velocityRateHistory.begin(), _velocityRateHistory.end(), 0.0);
    return sum / _velocityRateHistory.size();
}

//======================================================================
void VehicleBehavior::print(ostream& out) const
{
    stringstream ss;

    ss << "Velocity, ErrorVelocity: (" << _velocity << ", "
       << _errorVelocity << ")" << endl;

    AmuVector dvec = _vehicle->location()->directionVector();
    dvec.normalize();
    AmuVector vvec = dvec * _velocity;
    ss << "VelocityVector: (" << vvec.x() << ", " << vvec.y() << ", "
       << vvec.z() << ")" << endl;

    ss << "Acceleration: " << _accel << endl;
    ss << "Sleep Duration: " << _sleepDuration << endl;

#pragma omp critical(out_critical)
    out << ss.str();
}
