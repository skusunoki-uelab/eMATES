/* ******************************************************************
 * Copyright 2014-2024 ADVENTURE Project
 * All Rights Reserved
 * ****************************************************************** */
/**
 * @file PedestrianScene.cpp
 */
#ifdef INCLUDE_PEDESTRIANS
#include "PedestrianScene.hpp"
#include "Pedestrian.hpp"
#include "../Lane.hpp"
#include "../LaneBundle.hpp"
#include <sstream>

using namespace std;

//======================================================================
void PedestrianScene::print(ostream& out) const
{
    stringstream ss;

    ss << "Nearest Pedestrian: ";
    if (!_nearestPedestrian)
    {
        ss << "none" << endl;
    }
    else
    {
        ss << _nearestPedestrian->id()
           << ", distance: " << _distanceToNearestPedestrian << "("
           << _frontDistanceToNearestPedestrian << ", "
           << _sideDistanceToNearestPedestrian << ")" << endl;
    }
    ss << "Existing Oncoming Pedestrian: "
       << (_existsOncomingPedestrian ? "true" : "false") << endl;
    ss << "Lane to Stop: ";
    if (!_laneToStop)
    {
        ss << "none" << endl;
    }
    else
    {
        ss << _laneToStop->id() << "(@" << _laneToStop->parent()->id()
           << ")" << endl;
    }

#pragma omp critical(out_critical)
    out << ss.str();
}

#endif //INCLUDE_PEDESTRIANS
