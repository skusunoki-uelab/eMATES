/* ******************************************************************
 * Copyright 2014-2024 ADVENTURE Project
 * All Rights Reserved
 * ****************************************************************** */
/**
 * @file VehicleLaneChangeDeterminer.cpp
 */
#include "VehicleLaneChangeDeterminer.hpp"
#include "AppMates.hpp"
#include "GVManager.hpp"
#include "LoggerManager.hpp"
#include "Section.hpp"
#include "Vehicle.hpp"
#include "VehicleBehavior.hpp"
#include "VehicleBodyProperty.hpp"
#include "VehicleDecision.hpp"
#include "VehicleScene.hpp"
#include "io/Logger.hpp"
#include <cassert>

using namespace std;
using LP = LanePosition;

// static Logger* logger;

//======================================================================
void VehicleLaneChangeDeterminer::setVehicle(
    Vehicle* vehicle, VehicleBodyProperty* body, VehicleBehavior* behavior,
    VehicleDecision* decision, VehicleLocation* location, VehicleScene* scene)
{
    _vehicle  = vehicle;
    _body     = body;
    _behavior = behavior;
    _decision = decision;
    _location = location;
    _scene    = scene;

    // logger = AppMates::getLoggerManager().logger("vehicle");
}

//======================================================================
void VehicleLaneChangeDeterminer::determine()
{
    if (_decision->isLaneChangeRequired() || _decision->isLaneChangeExecutable()
        || _decision->isLaneChangeActive())
    {
        _behavior->setLaneTo(_scene->desiredLaneTo());
        _behavior->setDistanceTo(_scene->desiredDistanceTo());
    }

    // 車線変更中
    // During changing lanes
    if (_decision->isLaneChangeActive())
    {
        _renewErrorVelocity();
        return;
    }

    // 車線変更が必要であるが実行可能でない場合
    // When a lane-change is required but not executable
    else if (
        _decision->isLaneChangeRequired()
        && !(_decision->isLaneChangeExecutable())
        && !(_decision->isLaneChangeActive()))
    {
        _judgeGapAcceptance();
    }
}

//======================================================================
void VehicleLaneChangeDeterminer::_judgeGapAcceptance()
{
    // 車線変更が実行可能と判定されたのちは何もしない
    // Do nothing after lane-change is judged to be executable
    if (!(_decision->isLaneChangeRequired())
        || _decision->isLaneChangeExecutable()
        || _decision->isLaneChangeActive())
    {
        return;
    }

    // 車線変更は単路部内のみ可能
    // Lane-change is allowed only on section
    assert(_location->section());

    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    // 先行車と自車による判定
    // Judgment by the preceding car and the subject car
    if (_scene->adjLeader()
        && !isGapAcceptable(
            _scene->adjLeaderDiff(), _scene->adjLeader(), _vehicle))
    {
        return;
    }

    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    // 後続車と自車による判定
    // Judgment by the subject car and the following car
    if (_scene->adjFollower()
        && !(/*_scene->adjFollower()
               ->laneChangeDeterminer()
               ->*/
             isGapAcceptable(
                 _scene->adjFollowerDiff(), _vehicle, _scene->adjFollower())))
    {
        return;
    }

    _decision->setIsLaneChangeExecutable(true);
}

//======================================================================
bool VehicleLaneChangeDeterminer::isGapAcceptable(
    double gap, const Vehicle* leader, const Vehicle* follower) const
{
    ASSERT_MSG((leader != _vehicle) || (follower != _vehicle));
    if (gap < 0)
    {
        return false;
    }

    // 安全距離を算出
    // Calculate safety distance
    /**
     * @todo 他者のdeterminerやsceneを見えてよいか
     */
    double relVelocity = follower->velocity() - leader->velocity();
    double sDis        = follower->determiner()->calcSafetyDistance(
        follower->velocity(), relVelocity, follower->scene()->desiredHeadway());
    if (sDis < 0)
    {
        return false;
    }

    // 後続車が停車して譲ってくれている状況
    // Situation where the following car stops and give way
    else if (follower != _vehicle && follower->velocity() < 1.0e-6)
    {
        return true;
    }

    // 加速度を算出
    // Calculate acceleration
    double accel     = follower->determiner()->calcAcceleration(gap, sDis);
    double threshold = follower->body()->maxDeceleration()
        * _vehicle->localRoute()->targetUtility();
    if (accel < threshold)
    {
        return false;
    }

    return true;
}

//======================================================================
void VehicleLaneChangeDeterminer::_renewErrorVelocity()
{
    if (_decision->isLaneChangeActive())
    {
        if (_vehicle->localRoute()->targetDirection() == LP::Left)
        {
            _behavior->setErrorVelocity(
                AppMates::getGVManager().getNumeric("ERROR_VELOCITY") / 3600.0);
        }
        else
        {
            _behavior->setErrorVelocity(
                -AppMates::getGVManager().getNumeric("ERROR_VELOCITY")
                / 3600.0);
        }
    }
}
