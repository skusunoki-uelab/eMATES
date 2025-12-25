/* ******************************************************************
 * Copyright 2014-2024 ADVENTURE Project
 * All Rights Reserved
 * ****************************************************************** */
/**
 * @file VirtualLeader.cpp
 */
#include "VirtualLeader.hpp"

using namespace std;

// clang-format off
//======================================================================
const vector<string> VirtualLeader::vlTypeToString = {
    "SPEED_LIMIT",
    "TURNING_MIN_HEADWAY",
    "TURNING_RIGHT",
    "TURNING_LEFT",
    "VEHICLE_FRONT",
    "VEHICLE_TAIL",
    "VEHICLE_CLINT",
    "VEHICLE_CLINT2",
    "VEHICLE_CLSEC",
    "SIGNAL_RED",
    "SIGNAL_REDBLINK_BEF",
    "SIGNAL_REDBLINK_AFT",
    "SIGNAL_YELLOWBLINK",
    "LANE_INT",
    "LC_ADJ_LEADER",
    "LC_ADJ_SYNC",
    "LC_SHORT_GAP",
    "LC_SHORT_DIST",
    "LC_INTERRUPT",
    "LC_INTERRUPT_GAPGEN",
    "PEDESTRIAN_ON_ZEBRA",
};
// clang-format on

//======================================================================
void VirtualLeader::print(ostream& out) const
{
    out << vlTypeToString[_vlType] << ", " << "dist:" << _distance
        << ", " << "vel:" << _velocity;
    if (!_annotation.empty())
    {
        out << ", " << _annotation;
    }
    out << endl;
}
