/* ******************************************************************
 * Copyright 2014-2024 ADVENTURE Project
 * All Rights Reserved
 * ****************************************************************** */
/**
 * @file RouteKeyBase.cpp
 */
#include "RouteKeyBase.hpp"
#include "Intersection.hpp"
#include <iostream>

using namespace std;

//======================================================================
RouteKeyBase::RouteKeyBase(
    const VehicleType vehicleType, const double weights[],
    const Intersection* before, const Intersection* start,
    const Intersection* goal)
    : _vehicleType(vehicleType),
      _before(before),
      _start(start),
      _goal(goal)
{
    for (unsigned int i = 0; i < VEHICLE_ROUTING_PARAMETER_SIZE; i++)
    {
        _weights[i] = weights[i];
    }
}

//======================================================================
bool RouteKeyBase::equals(RouteKeyBase* another) const
{
    if (another->vehicleType() != _vehicleType
        || another->before() != _before || another->start() != _start
        || another->goal() != _goal)
    {
        return false;
    }
    for (unsigned int i = 0; i < VEHICLE_ROUTING_PARAMETER_SIZE; i++)
    {
        if (another->weight(i) != _weights[i])
        {
            return false;
        }
    }
    return true;
}

//======================================================================
void RouteKeyBase::print(ostream& out) const
{
    out << _vehicleType;
    for (unsigned int i = 0; i < VEHICLE_ROUTING_PARAMETER_SIZE; i++)
    {
        out << ", " << _weights[i];
    }
    out << ", " << _before->id() << ", " << _start->id() << ", "
        << _goal->id();
}
