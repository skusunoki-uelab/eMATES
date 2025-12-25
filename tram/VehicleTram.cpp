/* ******************************************************************
 * Copyright 2014-2024 ADVENTURE Project
 * All Rights Reserved
 * ****************************************************************** */
/**
 * @file VehicleTram.cpp
 */
#ifdef INCLUDE_TRAMS
#include "VehicleTram.hpp"
#include "TramRouteManager.hpp"
#include "TramRoute.hpp"
#include "../AppMates.hpp"
#include "../Intersection.hpp"
#include "../VehicleGlobalRoute.hpp"
#include <vector>
#include <iostream>

using namespace std;

//======================================================================
VehicleTram::~VehicleTram() {}

//======================================================================
void VehicleTram::setRoute()
{
    // 出発地・目的地を含む通過すべき交差点
    // Intersections to pass including origin and destination
    const vector<const Intersection*>& gates = _globalRoute.gates();

    // 条件に合致する路面電車の路線を取得
    // Acquire tram route that match the condition
    TramRoute* tramRoute
        = AppMates::getTramRouteManager().tramRouteWithDesignatedGates(
            gates);
    if (!tramRoute)
    {
        cerr << "ERROR: tram route not found:" << endl;
        _globalRoute.print(cerr);
        return;
    }

    // 通過交差点の完全なリストを登録
    // Register a complete list of intersections to pass
    vector<const Intersection*> tramRouteInters;
    tramRoute->getIntersections(tramRouteInters);
    for (auto itr : tramRouteInters)
    {
        const_cast<Route&>(_globalRoute.route()).addIntersection(itr);
    }
}

//======================================================================
void VehicleTram::preperceive()
{
    _perceiver.preperceive();
}

//======================================================================
void VehicleTram::perceive()
{
    // スリープ中は何もしない
    // Do nothing during inactive
    if (_behavior.sleepDuration() > 0)
    {
        return;
    }
    _perceiver.perceive();
}

//======================================================================
void VehicleTram::determine()
{
    _determiner.determine();
}

//======================================================================
void VehicleTram::act()
{
    _actor.act();
}

//======================================================================
void VehicleTram::postact()
{
    _actor.postact();
}

#endif //INCLUDE_TRAMS
