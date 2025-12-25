/* **************************************************************************
 * Copyright 2014-2024 ADVENTURE Project
 * All Rights Reserved
 * ************************************************************************** */
/**
 * @file VehiclePedExt.cpp
 */
#ifdef INCLUDE_PEDESTRIANS
#include "VehiclePedExt.hpp"
#include "Pedestrian.hpp"
#include "Zebra.hpp"
#include "../AppMates.hpp"
#include "../GVManager.hpp"
#include "../Intersection.hpp"
#include "../Lane.hpp"
#include "../Section.hpp"
#include "../VehicleBehavior.hpp"
#include "../VehicleBodyProperty.hpp"
#include "../VehicleGlobalRoute.hpp"
#include "../VehicleLocation.hpp"
#include "../VehicleScene.hpp"
#include "../VirtualLeader.hpp"
#include <cassert>
#include <iostream>
#include <vector>

using namespace std;
using namespace amu::geometry;
using namespace amu::math;

//==============================================================================
void VehiclePedExt::setVehicle(
    Vehicle* vehicle, VehicleBehavior* behavior, VehicleBodyProperty* body,
    VehicleGlobalRoute* globalRoute, VehicleLocation* location,
    VehicleScene* scene)
{
    _vehicle     = vehicle;
    _behavior    = behavior;
    _body        = body;
    _globalRoute = globalRoute;
    _location    = location;
    _scene       = scene;
}

//==============================================================================
void VehiclePedExt::searchPedestrian()
{
    // レーン始点までの距離
    // Distance to start point of lane
    double distance = _location->lane()->length() - _location->distance();

    vector<const Lane*> downstreamLanes;
    _vehicle->localRoute()->getNextLanes(
        _vehicle->location()->lane(), downstreamLanes);
    for (auto itr : downstreamLanes)
    {
        if (itr->pedExt()->hasApproachingPedestrian())
        {
            /*
             * レーンに近づく歩行者が存在したらレーン手前で停止する
             *   ただし，以前のステップで停止できそうにないと判断して
             *   いた場合は例外的に歩行者より優先して通行する
             *
             * If a pedestrian is approaching the lane, stop in the
             * front of the lane.
             *   However, if it has been determined in the previous step
             *   that the car is not possible to stop, as an exception,
             *   the car will be given priority over pedestrians.   
             */
            double targetStopDistance
                = distance - _body->bodyLength() / 2.0; // [m]
            if (!(itr->pedExt()->hasApproachingVehicles()))
            {
                VirtualLeader* leader = new VirtualLeader(
                    targetStopDistance, 0.0, VirtualLeader::PEDESTRIAN_ON_ZEBRA,
                    "Lane ID:" + itr->id());
                _scene->addLeader(leader);
            }

            /*
             * レーン手前で停止できそうになければ横断歩道レーンに
             * 通知し，歩行者が衝突を回避しようとする (未実装)
             *   通常はこのような状況にはならないはずだが，歩行者の
             *   歩行開始直後は自動車の認知が間に合わない可能性がある．
             *
             * If the car is not possible to stop in front of the
             * lane, a notification is sent to the crosswalk lane, and
             * pedestrians try to avoid collision (not implemented)
             *   This situation should not occur normally, but there is
             *   a possibility that the car may not be able to recognize
             *   the pedestrian immediately after it starts walking.
             */
            double stoppingDistance = _behavior->velocity()
                * _behavior->velocity() / 2
                * _body->maxDeceleration(); // [m/ms]^2/[m/ms^2]
            if (stoppingDistance > targetStopDistance)
            {
                _calcTimeToEnterCrosswalkLane(itr, targetStopDistance);
            }
            return;
        }
        else
        {
            /*
             * レーンに近づく歩行者が存在せず進入可能な場合は，距離を
             * 加算してひとつ下流のレーンをチェックする
             *
             * If there are no pedestrians approaching the lane and
             * the car is permitted to enter the lane, add the distance
             * and check the lane just downstream.
             */
            distance += itr->length();
        }
    }
}

//==============================================================================
void VehiclePedExt::_calcTimeToEnterCrosswalkLane(
    const Lane* lane, double distance)
{
    static double tmpVelocity = max(
        AppMates::getGVManager().getNumeric("VELOCITY_CREEP")
            / 3600.0, // [km/h]->[m/ms]
        _behavior->velocity());

    if (distance < 0)
    {
        _timeToEnterLane = 0;
    }
    else
    {
        _timeToEnterLane = static_cast<ulint>(ceil(distance / tmpVelocity));
    }

    constexpr static ulint threshold = 3000; //[ms]
    if (_timeToEnterLane < threshold)
    {
        _requiresNotification = true;
        _laneToBeNotified     = const_cast<Lane*>(lane);
    }
}

//==============================================================================
void VehiclePedExt::notifyCrosswalkLane()
{
    assert(_laneToBeNotified);
    _laneToBeNotified->pedExt()->putApproachingVehicle(_vehicle);
}

#endif //INCLUDE_PEDESTRIANS
