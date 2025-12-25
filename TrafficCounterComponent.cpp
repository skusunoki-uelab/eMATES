/* ******************************************************************
 * Copyright 2014-2024 ADVENTURE Project
 * All Rights Reserved
 * ****************************************************************** */
/**
 * @file TrafficCounterComponent.cpp
 */
#include "TrafficCounterComponent.hpp"
#include "ObjectInLane.hpp"
#include "Vehicle.hpp"
#include "VehicleLocation.hpp"
#include <AmuVector.hpp>
#include <cassert>

using namespace std;
using namespace amu::geometry;
using namespace amu::math;

//======================================================================
void TrafficCounterComponent::recordPassedVehicle(Vehicle* vehicle)
{
    assert(vehicle);
    TrafficCounterComponent::Record* record
        = new TrafficCounterComponent::Record(
            _lane->id(), vehicle->id(), *(vehicle->body()->type()),
            vehicle->globalRoute()->start()->id(),
            vehicle->globalRoute()->goal()->id(),
            vehicle->behavior()->velocity());
    _records.emplace_back(record);
}

//======================================================================
AmuPoint TrafficCounterComponent::position()
{
    amu::math::AmuVector vec = _lane->directionVector();
    vec.normalize();
    return (_lane->beginConnector()->point() + vec * _distance);
}
