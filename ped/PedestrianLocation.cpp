/* ******************************************************************
 * Copyright 2014-2024 ADVENTURE Project
 * All Rights Reserved
 * ****************************************************************** */
/**
 * @file PedestrianLocation.cpp
 */
#ifdef INCLUDE_PEDESTRIANS
#include "PedestrianLocation.hpp"
#include "../AppMates.hpp"
#include "../TimeManager.hpp"

using namespace std;

//======================================================================
PedestrianLocation::PedestrianLocation()
    : _pedestrian(nullptr),
      _intersection(nullptr),
      _zebra(nullptr),
      _isOnZebra(false)
{
    _generationTime = AppMates::getTimeManager().time();
    _startingTime   = 0;
}

//======================================================================
void PedestrianLocation::print(ostream& out) const
{
    stringstream ss;

    if (_intersection)
    {
        ss << "Intersection ID: " << _intersection->id() << ", ";
    }
    if (_zebra)
    {
        ss << "Zebra ID: " << _zebra->id() << endl;
    }
    ss << "Position(x y z): " << _position << endl;

#pragma omp critical(out_critical)
    out << ss.str();
}

#endif //INCLUDE_PEDESTRIANS
