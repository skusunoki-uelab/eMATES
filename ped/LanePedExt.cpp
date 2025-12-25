/* ******************************************************************
 * Copyright 2014-2024 ADVENTURE Project
 * All Rights Reserved
 * ****************************************************************** */
/**
 * @file LanePedExt.cpp
 */
#ifdef INCLUDE_PEDESTRIANS
#include "LanePedExt.hpp"
#include "VehiclePedExt.hpp"
#include "../Lane.hpp"
#include "../SubLaneBundle.hpp"
#include "../Vehicle.hpp"
#include <algorithm>
#include <iostream>

using namespace std;

//======================================================================
LanePedExt::LanePedExt(Lane* lane) : _lane(lane)
{
    _subsec                   = nullptr;
    _hasApproachingPedestrian = false;
    _approachingVehicles.clear();
    _tmpApproachingVehicles.clear();

#ifdef _OPENMP
    omp_init_lock(&_lock);
#endif //_OPENMP
}

//======================================================================
LanePedExt::~LanePedExt()
{
#ifdef _OPENMP
    omp_destroy_lock(&_lock);
#endif //_OPENMP
}

//======================================================================
void LanePedExt::renewApproachingVehicleOrder()
{
    // 2つのvectorを入れ替えて _approachingVehicles をソートする
    // Swap 2 vectors and sort _approachingVehicles
    _approachingVehicles.swap(_tmpApproachingVehicles);

    /*
     * ソートは必要ないかもしれない
     * Sorting may not be necessary
     */
    /*
    if (!(_approachingVehicles.empty()))
    {
        sort(_approachingVehicles.begin(), _approachingVehicles.end(),
             [](const Vehicle* rl, const Vehicle* rr)
             {
                 return (rl->pedExt()->timeToEnterLane()
                         < rr->pedExt()->timeToEnterLane());
             });
    }
    */

    // 一時的コンテナをクリアする
    // Clear temporary containers
    _tmpApproachingVehicles.clear();
}

//======================================================================
void LanePedExt::putApproachingVehicle(Vehicle* vehicle)
{
#ifdef _OPENMP
    omp_set_lock(&_lock);
#endif //_OPENMP

    if (find(
            _tmpApproachingVehicles.begin(), _tmpApproachingVehicles.end(),
            vehicle)
        != _tmpApproachingVehicles.end())
    {
        cerr << "ERROR: vehicle[" << vehicle->id()
             << "] is already registered to add to _approachingVehicles"
             << " in Lane[" << _lane->id() << "(@" << _lane->parent()->id()
             << ")]" << endl;
        exit(EXIT_FAILURE);
    }
    _tmpApproachingVehicles.emplace_back(vehicle);

#ifdef _OPENMP
    omp_unset_lock(&_lock);
#endif //_OPENMP
}

#endif //INCLUDE_PEDESTRIANS
