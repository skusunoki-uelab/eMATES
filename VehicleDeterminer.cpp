/* ******************************************************************
 * Copyright 2014-2024 ADVENTURE Project
 * All Rights Reserved
 * ****************************************************************** */
/**
 * @file VehicleDeterminer.cpp
 */
#include "VehicleDeterminer.hpp"
#include "AppMates.hpp"
#include "LoggerManager.hpp"
#include "Vehicle.hpp"
#include "VehicleBehavior.hpp"
#include "VehicleBodyProperty.hpp"
#include "VehicleDecision.hpp"
#include "VehicleLocation.hpp"
#include "VehicleScene.hpp"
#include "VirtualLeader.hpp"
#include "io/Logger.hpp"
#include <vector>
#include <algorithm>
#include <cmath>
#include <cassert>
#include <sstream>

using namespace std;

//======================================================================
void VehicleDeterminer::setVehicle(
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
void VehicleDeterminer::determine()
{
    _determineAcceleration();
}

//======================================================================
void VehicleDeterminer::_determineAcceleration()
{
    if (_behavior->sleepDuration() > 0)
    {
        return;
    }

    // 加速度候補
    // Acceleration candidates
    vector<double> accelCandidates;

    // 自由走行（前方の状況を考慮しない）
    // Flee flow (Not consider conditions ahead)
    double aMax = _body->maxAcceleration()
        * (1 - (_vehicle->velocity() / _scene->vMax()));
    accelCandidates.push_back(aMax);

    // 前方の状況に応じた最適な加速度の算出
    // Calculate optimal acceleration according to conditions ahead
    const vector<const VirtualLeader*>& leaders = _scene->leaders();
    for (auto itr : leaders)
    {
        accelCandidates.push_back(calcAcceleration(
            itr->distance(), _vehicle->velocity() - itr->velocity(),
            _scene->desiredHeadway()));
    }

    // 最も遅い候補を選ぶ
    // Pick the lowest candidate
    assert(!accelCandidates.empty());
    double accel = *min_element(accelCandidates.begin(), accelCandidates.end());

    /*
     * 最大加速度，最大減速度を越えた場合の処理
     *
     * Processing when maximum acceleration and maximum deceleration
     * are exceeded
     */
    if (accel > _body->maxAcceleration())
    {
        accel = _body->maxAcceleration();
    }
    else if (accel < _body->maxDeceleration())
    {
        accel = _body->maxDeceleration();
    }

    _decision->setDecidedAccel(accel);

    return;
}

//======================================================================
double VehicleDeterminer::calcAcceleration(
    double distance, double relVelocity, double desiredHeadway) const
{
    double sDis
        = calcSafetyDistance(_vehicle->velocity(), relVelocity, desiredHeadway);
    return _body->maxAcceleration()
        * min((1 - pow(_vehicle->velocity() / _scene->vMax(), 4.0)),
              (1 - pow(sDis / distance, 2.0)));
}

//======================================================================
double VehicleDeterminer::calcAcceleration(
    double distance, double safetyDistance) const
{
    return _body->maxAcceleration()
        * (1 - pow(_vehicle->velocity() / _scene->vMax(), 4.0)
           - pow(safetyDistance / distance, 2.0));
}

//======================================================================
double VehicleDeterminer::calcSafetyDistance(
    double velocity, double relVelocity, double desiredHeadway) const
{
    double sDis = _body->jamDistance() + velocity * desiredHeadway
        + velocity * relVelocity
            / (2
               * sqrt(_body->maxAcceleration() * (-_body->maxDeceleration())));
    return sDis;
}
