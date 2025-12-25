/* ******************************************************************
 * Copyright 2014-2024 ADVENTURE Project
 * All Rights Reserved
 * ****************************************************************** */
/**
 * @file VehicleScene.cpp
 */
#include "VehicleScene.hpp"
#include "VirtualLeader.hpp"
#include "Vehicle.hpp"
#include <iostream>

using namespace std;

//======================================================================
VehicleScene::VehicleScene()
{
    _vehicle = nullptr;

    _leaders.clear();
    _desiredLaneTo        = nullptr;
    _adjLeader            = nullptr;
    _adjFollower          = nullptr;
    _interruptingLeader   = nullptr;
    _interruptingFollower = nullptr;
    _desiredHeadway       = MAX_DESIRED_HEADWAY * 1000;
}

//======================================================================
void VehicleScene::clear()
{
    for (auto itr : _leaders)
    {
        delete itr;
    }
    _leaders.clear();
    _adjLeader            = nullptr;
    _adjFollower          = nullptr;
    _interruptingLeader   = nullptr;
    _interruptingFollower = nullptr;
}

//======================================================================
void VehicleScene::print(ostream& out) const
{
    stringstream ss;

    ss << "Virtual Leaders:" << endl;

    for (auto itr : _leaders)
    {
        ss << "\t";
        itr->print(ss);
    }
    ss << "DesiredHeadway: " << _desiredHeadway << endl;

    ss << "adjLeader:            ";
    if (_adjLeader)
    {
        ss << _adjLeader->id() << ", " << _adjLeaderDiff << endl;
    }
    else
    {
        ss << "NULL" << endl;
    }
    ss << "adjFollower:          ";
    if (_adjFollower)
    {
        ss << _adjFollower->id() << ", " << _adjFollowerDiff << endl;
    }
    else
    {
        ss << "NULL" << endl;
    }
    ss << "interruptingLeader:   ";
    if (_interruptingLeader)
    {
        ss << _interruptingLeader->id() << ", "
           << _interruptingLeaderDiff << endl;
    }
    else
    {
        ss << "NULL" << endl;
    }
    ss << "interruptingFollower: ";
    if (_interruptingFollower)
    {
        ss << _interruptingFollower->id() << ", "
           << _interruptingFollowerDiff << endl;
    }
    else
    {
        ss << "NULL" << endl;
    }

#pragma omp critical(out_critical)
    out << ss.str();
}
