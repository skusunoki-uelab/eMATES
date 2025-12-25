/* **************************************************************************
 * Copyright 2014-2024 ADVENTURE Project
 * All Rights Reserved
 * ************************************************************************** */
/**
 * @file SpeedLimitItem.cpp
 */
#include "SpeedLimitItem.hpp"
#include "Intersection.hpp"
#include "Lane.hpp"
#include "LaneInSection.hpp"
#include "Section.hpp"
#include <sstream>
#include <typeinfo>

using namespace std;

//==============================================================================
SpeedLimitItem::SpeedLimitItem(
    ulint time, Section* section, bool isUp, double speedLimit)
    : _time(time), _section(section), _isUp(isUp), _speedLimit(speedLimit)
{
    _className = typeid(this).name();

    ostringstream oss;
    string        up = (isUp ? "U" : "D");
    oss << "section[" << section->id() + "](" << up + "), " << _speedLimit
        << "[km/h]";
    _contents = oss.str();
}

//==============================================================================
void SpeedLimitItem::activate()
{
    // 単路部の属性を更新する
    // Update section property
    _section->setSpeedLimit(_isUp, _speedLimit);

    // 接続する交差点の巨視的諸量を更新する
    // Update macroscopic quantities of connecting intersection
    _section->intersection(_isUp)->observeLinkFlow();
}
