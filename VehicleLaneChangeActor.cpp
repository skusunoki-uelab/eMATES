/* ******************************************************************
 * Copyright 2014-2024 ADVENTURE Project
 * All Rights Reserved
 * ****************************************************************** */
/**
 * @file VehicleLaneChangeActor.cpp
 */
#include "VehicleLaneChangeActor.hpp"
#include "AppMates.hpp"
#include "GVManager.hpp"
#include "Lane.hpp"
#include "Section.hpp"
#include "TimeManager.hpp"
#include "Vehicle.hpp"
#include "VehicleActor.hpp"
#include "VehicleBehavior.hpp"
#include "VehicleBodyProperty.hpp"
#include "VehicleDecision.hpp"
#include "VehicleLocation.hpp"
#include "VehicleScene.hpp"
#include <cassert>
#include <cmath>

using namespace std;
using namespace amu::geometry;
using LP = LanePosition;

//======================================================================
void VehicleLaneChangeActor::setVehicle(
    Vehicle* vehicle, VehicleBodyProperty* body, VehicleBehavior* behavior,
    VehicleDecision* decision, VehicleLocation* location, VehicleScene* scene)
{
    _vehicle  = vehicle;
    _body     = body;
    _behavior = behavior;
    _decision = decision;
    _location = location;
    _scene    = scene;
}

//======================================================================
void VehicleLaneChangeActor::act()
{
    if (_decision->isLaneChangeExecutable()
        && !(_decision->isLaneChangeActive()))
    {
        // 車線変更を開始
        // Start lane-change
        _startLaneChange();
    }
    if (_decision->isLaneChangeActive())
    {
        // 車線変更中
        // During changing lanes
        _updateError();
    }
}

//======================================================================
void VehicleLaneChangeActor::_startLaneChange()
{
    if (!(_behavior->laneTo()))
    {
        return;
    }

    _decision->setIsLaneChangeActive(true);
    if (_vehicle->localRoute()->targetDirection() == LP::Left)
    {
        _errorRemaining = -(_vehicle->location()->section()->laneWidth());
    }
    else
    {
        _errorRemaining = _vehicle->location()->section()->laneWidth();
    }

    /*
     * 車線の更新
     *   この後の処理でnextLaneが必要であるためひとまず登録する．
     *   実際には Vehicle::proact() でローカル経路を再探索する
     *
     * Update lane
     *   Since nextLane is necessary for the processing after this,
     *   it is registered tentatively. Actually, local-reroute
     *   in Vehicle::proact() .
     */
    const Lane* nextLane = _behavior->laneTo();
    _location->setLane(nextLane);
    _location->setNextLane(nextLane->nextStraightLane());
    _location->setDistance(_behavior->distanceTo());

    // 車線変更に使用した変数のクリア
    // Clear variables used for lane-change
    _scene->setDesiredLaneTo(nullptr);
    _scene->setDesiredDistanceTo(0);
    _behavior->setLaneTo(nullptr);
    _behavior->setDistanceTo(0);
    _decision->setIsLaneChangeRequired(false);
    _decision->setIsLaneChangeExecutable(false);
    // VehicleDecision::_isLaneChangeActive は _finishLaneChange() でクリア
    // Clear VehicleDecision::_isLaneChangeActive in _finishLaneChange() .

    _scene->setAdjLeader(nullptr);
    _scene->setAdjLeaderDiff(0);
    _scene->setAdjFollower(nullptr);
    _scene->setAdjFollowerDiff(0);
}

//======================================================================
void VehicleLaneChangeActor::_updateError()
{
    _errorRemaining
        += _behavior->errorVelocity() * AppMates::getTimeManager().unit();

    if ((_vehicle->localRoute()->targetDirection() == LP::Right
         && _errorRemaining <= 0)
        || (_vehicle->localRoute()->targetDirection() == LP::Left
            && _errorRemaining >= 0))
    {
        _finishLaneChange();
    }
    else
    {
        _location->setError(_errorRemaining);
    }
}

//======================================================================
void VehicleLaneChangeActor::_finishLaneChange()
{
    _errorRemaining = 0;
    _location->setError(0);

    // フラグのクリア
    // Clear variable
    _decision->setIsLaneChangeActive(false);

    VehicleActor* actor = const_cast<VehicleActor*>(_vehicle->actor());
    actor->unnotify();
    actor->setRequiresLocalReroute(true);
}

//======================================================================
void VehicleLaneChangeActor::abortLaneChange()
{
    _finishLaneChange();
}
