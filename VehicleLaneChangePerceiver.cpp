/* ******************************************************************
 * Copyright 2014-2024 ADVENTURE Project
 * All Rights Reserved
 * ****************************************************************** */
/**
 * @file VehicleLaneChangePerceiver.cpp
 */
#include "VehicleLaneChangePerceiver.hpp"
#include "AppMates.hpp"
#include "GVManager.hpp"
#include "LocalLaneRouter.hpp"
#include "TimeManager.hpp"
#include "VirtualLeader.hpp"
#include "Vehicle.hpp"
#include "VehicleBehavior.hpp"
#include "VehicleBodyProperty.hpp"
#include "VehicleDecision.hpp"
#include "VehicleLocalRoute.hpp"
#include "VehicleLocation.hpp"
#include "VehicleScene.hpp"
#include <iostream>
#include <iomanip>

using namespace std;
using LP = LanePosition;

//======================================================================
void VehicleLaneChangePerceiver::setVehicle(
    Vehicle* vehicle, VehicleBehavior* behavior, VehicleBodyProperty* body,
    VehicleDecision* decision, VehicleLocalRoute* localRoute,
    VehicleLocation* location, VehicleScene* scene,
    LocalLaneRouter* localRouter)
{
    _vehicle     = vehicle;
    _behavior    = behavior;
    _body        = body;
    _decision    = decision;
    _localRoute  = localRoute;
    _location    = location;
    _scene       = scene;
    _localRouter = localRouter;
}

//======================================================================
void VehicleLaneChangePerceiver::preperceive()
{
    // 交差点走行中は何もしない
    // Do nothing at intersection
    if (_location->intersection())
    {
        return;
    }

    // 車線変更実行中 (error更新中）は何もしない
    // Do nothing during changing lanes (updating error)
    if (_decision->isLaneChangeActive())
    {
        return;
    }

    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    if (!(_decision->isLaneChangeRequired()))
    {
        if (_localRoute->targetDirection() == LP::Left
            || _localRoute->targetDirection() == LP::Right)
        {
            /*
             * 車線変更が必要
             *   VehicleDecision::_isLaneChangeRequired をtrueにし，
             *   自分が所属する単路部に通知する
             *
             * Require Lane-change
             *   Set VehicleDecision::_isLaneChangeRequired to true and
             *   notify the section the subject car belongs
             *   
             */
            _decision->setIsLaneChangeRequired(true);
            const_cast<VehicleActor*>(_vehicle->actor())->notify();
        }
        else
        {
            // 車線変更は必要なし
            // Not require lane-change
            if (_behavior->isNotifying())
            {
                const_cast<VehicleActor*>(_vehicle->actor())->unnotify();
            }
        }
    }
}

//======================================================================
void VehicleLaneChangePerceiver::perceive()
{
    // 交差点走行中は何もしない
    // Do nothing at intersection
    if (_location->intersection())
    {
        return;
    }

    // 車線変更実行中 (error更新中）は何もしない
    // Do nothing during changing lanes (updating error)
    if (_decision->isLaneChangeActive())
    {
        return;
    }

    // 車線変更先の車線の先行車と後続車を探索する
    // Search for preceding and following cars on the lane to change to
    if (_decision->isLaneChangeRequired())
    {
        _searchAdjLeaderAndFollower();
    }

    // 以下の処理は単路部を走行する全車両で必要
    // The following processing is required for all cars in sections.
    _searchInterruptingLeader();
    _renewDesiredHeadwayForLaneChange();
    _setVirtualLeadersForLaneChange();
}

//======================================================================
void VehicleLaneChangePerceiver::_searchAdjLeaderAndFollower()
{
    const Vehicle* adjLeader   = nullptr;
    const Vehicle* adjFollower = nullptr;

    const Lane* laneTo = nullptr;
    double      distanceTo;
    _location->lane()->getSideLaneDistance(
        _location->distance(), _localRoute->targetDirection(), &laneTo,
        &distanceTo);

    if (!laneTo)
    {
        if (_decision->isLaneChangeActive())
        {
            cerr << "WARNING:" << "adjacent lane not found while lane-changing."
                 << endl
                 << "vehicle:" << _vehicle->id()
                 << " in section:" << _location->section()->id() << endl;
            const_cast<VehicleLaneChangeActor*>(_vehicle->laneChangeActor())
                ->abortLaneChange();
        }
        _decision->setIsLaneChangeRequired(false);
        return;
    }
    _scene->setDesiredLaneTo(laneTo);
    _scene->setDesiredDistanceTo(distanceTo);

    // 車体の後端を起点に探索する
    // Search from the rear end of the vehicle
    const ObjectInLane* adjLeaderAgent;
    double              adjLeaderDiff;
    laneTo->getFrontAgentFar(
        distanceTo - _body->bodyLength() / 2, 100, &adjLeaderAgent,
        &adjLeaderDiff);
    adjLeader
        = dynamic_cast<Vehicle*>(const_cast<ObjectInLane*>(adjLeaderAgent));
    if (!adjLeader)
    {
        _scene->setAdjLeader(nullptr, 0.0);
    }
    else
    {
        _scene->setAdjLeader(
            adjLeader,
            adjLeaderDiff
                - (_body->bodyLength() + adjLeader->body()->bodyLength() / 2));
    }

    const ObjectInLane* adjFollowerAgent;
    double              adjFollowerDiff;
    laneTo->getFollowingAgentFar(
        distanceTo - _body->bodyLength() / 2, 100, &adjFollowerAgent,
        &adjFollowerDiff);
    adjFollower
        = dynamic_cast<Vehicle*>(const_cast<ObjectInLane*>(adjFollowerAgent));
    if (!adjFollower)
    {
        _scene->setAdjFollower(nullptr, 0.0);
    }
    else
    {
        _scene->setAdjFollower(
            adjFollower,
            adjFollowerDiff - adjFollower->body()->bodyLength() / 2);
    }
}

//======================================================================
void VehicleLaneChangePerceiver::_searchInterruptingLeader()
{
    const Vehicle* interruptingLeader = nullptr;
    double         interruptingLeaderDiff;
    const Vehicle* interruptingFollower = nullptr;
    double         interruptingFollowerDiff;

    if (!(_location->section()))
    {
        return;
    }

    // 車線変更中の車両を取得する
    // Get the cars that are changing lanes
    const list<const Vehicle*>& watchedVehicles
        = _location->section()->watchedVehicles();

    // もっとも近い車両を選ぶ
    // Pick the closest car
    for (auto itr : watchedVehicles)
    {
        const VehicleBehavior* anotherBehavior = itr->behavior();
        const Lane*            anotherLaneTo   = anotherBehavior->laneTo();

        if (!anotherLaneTo)
        {
            continue;
        }

        if (itr->velocity()
            < AppMates::getGVManager().getNumeric("VELOCITY_CREEP") / 3600)
        {
            continue;
        }

        /**
         * @todo 他者のdecisionは見えてよいか．BlinkerかisNotifyingでは．
         */
        if (itr->decision()->isLaneChangeRequired()
            && anotherLaneTo == _location->lane())
        {
            /*
             * 相手のレーンに投影したときの位置の差
             *   相手が前方であれば正，後方であれば負
             * 
             * Difference in position projected onto the opponent's lane
             *   Positive if the opponent is ahead, negative if behind. 
             */
            double distanceFrom = itr->location()->distance()
                - _location->lane()->lengthOnSideLane(
                    _location->distance(), itr->location()->lane());

            // _interruptingLeader の判定
            // Judgment of _interruptingLeader
            if (distanceFrom > 0)
            {
                if (!interruptingLeader
                    || distanceFrom
                        < interruptingLeader->behavior()->distanceTo())
                {
                    interruptingLeader     = itr;
                    interruptingLeaderDiff = distanceFrom
                        - (_body->bodyLength() + itr->body()->bodyLength()) / 2;
                }
            }
            // _interruptingFollower の判定
            // Judgment of _interruptingFollower
            else
            {
                if (!interruptingFollower
                    || distanceFrom
                        > interruptingFollower->behavior()->distanceTo())
                {
                    interruptingFollower     = itr;
                    interruptingFollowerDiff = -distanceFrom
                        - (_body->bodyLength() + itr->body()->bodyLength()) / 2;
                }
            }
        }
    }

    if (!interruptingLeader)
    {
        _scene->setInterruptingLeader(nullptr, 0);
    }
    else
    {
        _scene->setInterruptingLeader(
            interruptingLeader, interruptingLeaderDiff);
    }

    if (!interruptingFollower)
    {
        _scene->setInterruptingFollower(nullptr, 0);
    }
    else
    {
        _scene->setInterruptingFollower(
            interruptingFollower, interruptingFollowerDiff);
    }
}

//======================================================================
void VehicleLaneChangePerceiver::_renewDesiredHeadwayForLaneChange()
{
    double headway = _scene->desiredHeadway();
    headway += (MAX_DESIRED_HEADWAY * 1000 - headway)
        * static_cast<double>(AppMates::getTimeManager().unit())
        / (DESIRED_HEADWAY_RELAXATION * 1000);

    // 割り込みされる車両の処理
    // Processing for interrupted car
    if (_scene->interruptingLeader())
    {
        double util
            = _scene->interruptingLeader()->localRoute()->targetUtility();
        headway = min(
            headway,
            util * (MIN_DESIRED_HEADWAY * 1000)
                + (1 - util) * (MAX_DESIRED_HEADWAY * 1000));
    }

    // 車線変更する車両の処理
    // Processing for lane-changing car
    else if (_decision->isLaneChangeRequired())
    {
        double util = _vehicle->localRoute()->targetUtility();
        headway     = min(
            headway,
            util * (MIN_DESIRED_HEADWAY * 1000)
                + (1 - util) * (MAX_DESIRED_HEADWAY * 1000));
    }

    _scene->setDesiredHeadway(headway);
}

//======================================================================
void VehicleLaneChangePerceiver::_setVirtualLeadersForLaneChange()
{
    const Vehicle* adjLeader     = _scene->adjLeader();
    double         adjLeaderDiff = _scene->adjLeaderDiff();

    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    // 車線変更中は _adjLeader に追従
    // Follow _adjLeader during changing lanes
    if (_decision->isLaneChangeActive() && adjLeader)
    {
        const VirtualLeader* leader = new VirtualLeader(
            adjLeaderDiff, adjLeader->velocity(), VirtualLeader::LC_ADJ_LEADER,
            "Vehicle ID:" + adjLeader->id());
        _scene->addLeader(leader);
    }

    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    // synchronization
    else if (
        _decision->isLaneChangeRequired()
        && _localRoute->targetUtility() > DESIRE_THRESHOLD_SYNCHRONIZATION)
    {
        if (adjLeader)
        {
            double relVelocity = _vehicle->velocity() - adjLeader->velocity();
            double sDis        = _vehicle->determiner()->calcSafetyDistance(
                _vehicle->velocity(), relVelocity, _scene->desiredHeadway());

            if (sDis >= 0
                && adjLeader->velocity()
                    > AppMates::getGVManager().getNumeric("VELOCITY_CREEP")
                        / 3600)
            {
                const VirtualLeader* leader = new VirtualLeader(
                    adjLeaderDiff, adjLeader->velocity(),
                    VirtualLeader::LC_ADJ_SYNC,
                    "Vehicle ID:" + adjLeader->id());
                _scene->addLeader(leader);
            }

            // 追加条件
            // Additional condition
            else if (false
                     /*adjLeader->velocity()
                       > AppMates::getGVManager()
                       .getNumeric("VELOCITY_CREEP")
                       / 3600
                       || (adjLeader->velocity() > 1.0e-6
                       && distanceToNextIntersection
                       < _body->bodyLength * 2)*/)
            {
                const VirtualLeader* leader = new VirtualLeader(
                    _body->bodyLength() / 2, 0, VirtualLeader::LC_ADJ_SYNC,
                    "Vehicle ID:" + adjLeader->id());
                _scene->addLeader(leader);
            }
        }
    }

    const Vehicle* interruptingLeader     = _scene->interruptingLeader();
    double         interruptingLeaderDiff = _scene->interruptingLeaderDiff();
    if (!interruptingLeader)
    {
        return;
    }

    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    // 割り込まれている間は _interruptingLeader に追従
    // Follow _interruptingLeader during being interrupted
    /**
     * @todo 他者のdecisionの参照
     */
    if (interruptingLeader->decision()->isLaneChangeActive())
    {
        const VirtualLeader* leader = new VirtualLeader(
            interruptingLeaderDiff, interruptingLeader->velocity(),
            VirtualLeader::LC_INTERRUPT,
            "Vehicle ID:" + interruptingLeader->id());
        _scene->addLeader(leader);
    }

    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    // gap generation (cooperation)
    else if (
        interruptingLeader->localRoute()->targetUtility()
        > DESIRE_THRESHOLD_COOPERATION)
    {
        //double relVelocity
        //= interruptingLeader->velocity() - _vehicle->velocity();
        //double sDis = _vehicle->determiner()->calcSafetyDistance(
        //_vehicle->velocity(), relVelocity, _scene->desiredHeadway());
        //if (interruptingLeaderDiff >= sDis)
        //{
        const VirtualLeader* leader = new VirtualLeader(
            interruptingLeaderDiff, interruptingLeader->velocity(),
            VirtualLeader::LC_INTERRUPT_GAPGEN,
            "Vehicle ID:" + interruptingLeader->id());
        _scene->addLeader(leader);
        //}
    }
}
