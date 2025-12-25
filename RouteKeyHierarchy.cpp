/* ******************************************************************
 * Copyright 2014-2024 ADVENTURE Project
 * All Rights Reserved
 * ****************************************************************** */
/**
 * @file RouteKeyHierarchy.cpp
 */
#include "RouteKeyHierarchy.hpp"
#include "Intersection.hpp"
#include <iostream>

using namespace std;

//======================================================================
RouteKeyHierarchy::RouteKeyHierarchy(
    const VehicleType type, const unsigned int prefRank,
    const double weights[], const Intersection* before,
    const Intersection* start, const Intersection* goal)
    : RouteKeyBase(type, weights, before, start, goal),
      _prefRank(prefRank)
{
}

//======================================================================
bool RouteKeyHierarchy::equals(RouteKeyBase* another) const
{
    if (!RouteKeyBase::equals(another))
    {
        return false;
    }

    RouteKeyHierarchy* castedAnotherKey
        = dynamic_cast<RouteKeyHierarchy*>(another);
    if (!castedAnotherKey || castedAnotherKey->prefRank() != _prefRank)
    {
        return false;
    }

    return true;
}

//======================================================================
void RouteKeyHierarchy::print(ostream& out) const
{
    out << _vehicleType << "," << _prefRank;
    for (unsigned int i = 0; i < VEHICLE_ROUTING_PARAMETER_SIZE; i++)
    {
        out << "," << _weights[i];
    }
    out << "," << _before->id() << "," << _start->id() << ","
        << _goal->id();
}
