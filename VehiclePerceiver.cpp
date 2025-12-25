/* ******************************************************************
 * Copyright 2014-2024 ADVENTURE Project
 * All Rights Reserved
 * ****************************************************************** */
/**
 * @file VehiclePerceiver.cpp
 */
#include "VehiclePerceiver.hpp"
#include "AppMates.hpp"
#include "ClockerManager.hpp"
#include "CSNodeFast.hpp" // [eMATES]
#include "CSNodeNormal.hpp" // [eMATES]
#include "GVManager.hpp"
#include "Intersection.hpp"
#include "Lane.hpp"
#include "ODNode.hpp"
#include "RandomNumberGenerator.hpp"
#include "RelativeDirection.hpp"
#include "Vehicle.hpp"
#include "VehicleBehavior.hpp"
#include "VehicleBodyProperty.hpp"
#include "VehicleDecision.hpp"
#include "VehicleLocation.hpp"
#include "VehicleScene.hpp"
#include "VirtualLeader.hpp"
#include "Section.hpp"
#include "Signal.hpp"
#ifdef INCLUDE_PEDESTRIANS
#include "ped/Pedestrian.hpp"
#include "ped/VehiclePedExt.hpp"
#include "ped/Zebra.hpp"
#endif //INCLUDE_PEDESTRIANS
#include <iostream>
#include <typeinfo>
#include <cassert>
#include <AmuConverter.hpp>
#include <AmuPoint.hpp>
#include <AmuVector.hpp>

using namespace std;
using namespace amu::converter;
using namespace amu::geometry;
using namespace amu::math;
using LP = LanePosition;

//#define VEHICLE_RECOG_MEASURE_TIME

//======================================================================
VehiclePerceiver::VehiclePerceiver()
{
    _vehicle     = nullptr;
    _behavior    = nullptr;
    _body        = nullptr;
    _decision    = nullptr;
    _globalRoute = nullptr;
    _localRoute  = nullptr;
    _location    = nullptr;
    _scene       = nullptr;

    _checksCollisionStrictly
        = AppMates::getGVManager().getFlag("STRICT_COLLISION_CHECK");

#ifdef INCLUDE_PEDESTRIANS
    _pedExt = nullptr;
#endif //INCLUDE_PEDESTRIANS
}

//======================================================================
void VehiclePerceiver::setVehicle(
    Vehicle* vehicle, VehicleBehavior* behavior, VehicleBodyProperty* body,
    VehicleDecision* decision, VehicleGlobalRoute* globalRoute,
    VehicleLocalRoute* localRoute, VehicleLocation* location,
    VehicleScene* scene)
{
    _vehicle     = vehicle;
    _behavior    = behavior;
    _body        = body;
    _decision    = decision;
    _globalRoute = globalRoute;
    _localRoute  = localRoute;
    _location    = location;
    _scene       = scene;
}

//======================================================================
void VehiclePerceiver::preperceive()
{
    // 前ステップの仮想先行車をクリアする
    // Clear the virtual leaders in the previous step
    _scene->clear();

#ifdef INCLUDE_PEDESTRIANS
    _vehicle->pedExt()->clearStatus();
#endif //INCLUDE_PEDESTRIANS
}

//======================================================================
void VehiclePerceiver::perceive()
{
    // [eMATES] 充電中もしくは充電待ちならskip
    VehicleEV* ev = dynamic_cast<VehicleEV*>(_vehicle);
    if (ev && (ev->isChargingInCS() || ev->isWaiting()))
    {
        return;
    }

    /*
     * 単路中で交差点との境界に近く，レーンの先頭で，速度ゼロの場合，
     * 一旦停止フラグを立てる
     *
     * If the car is near the boundary with the intersection on a
     * section, at the beginning of the lane, and the velocity is zero,
     * enable a pause flag.
     */
    if (_location->section()
        && _location->section()->distanceToNext(
               _location->lane(), _location->distance())
            < _body->bodyLength()
        && !(_location->lane()->frontAgent(_vehicle))
        && abs(_behavior->velocity()) < 1.0e-6 && _behavior->numPausing() == 0)
    {
        _behavior->incrementNumPausing();
    }

    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    // 仮想先行者の探索全般で用いる変数の決定
    // Determine variables used in general virtual leader searches
    const Intersection* nextInter = nullptr;
    const Intersection* prevInter = nullptr;
    if (_location->section())
    {
        nextInter = _location->section()->intersection(
            _location->section()->isUp(_location->lane()));
        prevInter = _location->section()->intersection(
            !(_location->section()->isUp(_location->lane())));
    }
    else
    {
        if (_localRoute->targetDirection() == LP::Center)
        {
            // 予定経路が守られている場合
            // In the case the planned route being kept
            if (_location->prevIntersection())
            {
                nextInter = const_cast<Intersection*>(_globalRoute->next(
                    _location->prevIntersection(), _location->intersection()));
            }
            else
            {
                nextInter = const_cast<Intersection*>(
                    _globalRoute->next(_location->intersection()));
            }
        }
        else
        {
            /*
             * 予定経路が守られていない場合
             *   通常，LocalRouteの最後から2つめには交差点内の最後の
             *   レーンが入っている
             *
             * In the case the planned route not being kept
             *   Usually the second to last lane in the LocalRoute is
             *   the last lane in the intersection.
             */
            const Lane* exitLane = _vehicle->localRoute()->previous(
                _vehicle->localRoute()->lastLane());
            if (exitLane && _location->intersection()->containsLane(exitLane))
            {
                nextInter = _location->intersection()->next(
                    _location->intersection()->direction(
                        exitLane->endConnector()));
            }
            else
            {
                nextInter = _location->intersection()->next(1);
            }
        }
        prevInter = _location->intersection();
    }

    // 次の交差点に進入できるか
    // Whether the car can enter the next intersection
    bool canEnterNextInter = true;

    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    // 希望走行速度の決定
    // Determine desired velocity
    _determineDesiredVelocity();

    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    // 仮想先行車の取得
    // Get virtual leaders
    /**
     * @todo
     * ひとまず，エージェントは毎ステップ必要な全ての情報を
     * 正しく認知できることを前提とする．
     * 見落としなどを実装するためにはこの関数を修正する必要があろう．
     */

    //------------------------------------------------------------------
    // 先行エージェント
    // Preceding agent
    _searchFrontAgent(50);

    //------------------------------------------------------------------
    /*
     * 前方単路の制限速度
     *   単路部の終端より30m以上手前であれば考慮しない
     *
     * Speed limit for the section ahead
     *   Not considered if the car is 30 m or more before the end of
     *   the section
     */
    if (_location->intersection()
        || (_location->section()->distanceToNext(
                _location->lane(), _location->distance())
            < 30))
    {
        _searchFrontSpeedLimit(30);
    }

    //------------------------------------------------------------------
    // 信号
    // Traffic light
    canEnterNextInter = !(_shouldStopBySignal(nextInter, prevInter, 50));

    //------------------------------------------------------------------
    /*
     * 単路内先頭車両の処理
     *   次に進入する交差点がODノードの場合は無視する.
     *
     * Processing for the front car on a section
     *   Ignore if the next intersection is an OD node.
     */
    if (_location->section()                                              //
        && canEnterNextInter                                              //
        && _location->section()->isHeadAgent(_vehicle, _location->lane()) //
        && typeid(*nextInter) != typeid(ODNode)
        && typeid(*nextInter) != typeid(CSNodeFast) // [eMATES]
        && typeid(*nextInter) != typeid(CSNodeNormal)) // [eMATES]
    {
        //--------------------------------------------------------------
        // 交錯レーン探索の準備
        // Preparing for collision lane search

        // 交差点内交錯レーン
        // Collision lanes in intersection
        vector<const Lane*> clInter;
        // 交差点内交錯レーンの上流の単路内レーン
        // Lanes within a section upstream of collision lanes
        vector<const Lane*> clSection;

        nextInter->getCollisionLanes(
            _localRoute->lanesInIntersection(), clInter, clSection);

        //--------------------------------------------------------------
        // 交差点内交錯レーン
        // Collision lanes in intersection
        canEnterNextInter
            = !(_shouldStopByCollisionInIntersection(nextInter, clInter));

        //--------------------------------------------------------------
        // 交差点内交錯レーンの上流の単路内レーン
        // Lanes within a section upstream of collision lanes
        if (canEnterNextInter)
        {
            canEnterNextInter
                = !(_shouldStopByCollisionInSection(nextInter, clSection));
        }

        //--------------------------------------------------------------
        // 現在は無効化
        // Disabled now

        // 右左折時は交差点内に1台ずつしか進入できない
        // Only one car can enter the intersection when turning
        /*
          if (canEnterNextInter && !(clInter.empty()))
          {
          canEnterNextInter = !(_shouldStopByLaneInInter(nextInter));
          }
        */

        //--------------------------------------------------------------
        // 交差点通過後の単路部の空きスペース
        // Space on the section after passing the intersection
        if (canEnterNextInter)
        {
            canEnterNextInter = !(_shouldStopByShortSpace());
        }

        //--------------------------------------------------------------
        // 現在は無効化
        // Disabled now

        // 最小ヘッドウェイ
        // Minimum headway
        /*
          if (canEnterNextInter)
          {
          canEnterNextInter = !(_shouldStopByMinHeadway(nextInter));
          }
        */

        //--------------------------------------------------------------
        if (canEnterNextInter)
        {
            // 転回時事前減速
            // Deceleration before turning
            _determineTurningVelocity();

#ifdef INCLUDE_PEDESTRIANS
            // 歩行者
            // Pedestrian
            _pedExt->searchPedestrian();
#endif //INCLUDE_PEDESTRIANS
        }
    }

    //------------------------------------------------------------------
    // 単路部内全車両の処理
    // Processing for all cars on a section
    if (_location->section())
    {
    }

    //------------------------------------------------------------------
    // 交差点内全車両の処理
    // Processing for all cars on a intersection
    else if (_location->intersection())
    {
        //--------------------------------------------------------------
        // 優先エージェント
        // Preferred agents
        _searchPreferentialAgentInIntersection();

#ifdef INCLUDE_PEDESTRIANS
        //--------------------------------------------------------------
        // 歩行者
        // Pedestrian
        _pedExt->searchPedestrian();
#endif //INCLUDE_PEDESTRIANS
    }
}

//======================================================================
void VehiclePerceiver::_determineDesiredVelocity()
{
    if (_location->section())
    {
        // 単路部の場合は制限速度
        // Speed limit for a section
        _scene->setVMax(
            _location->lane()->speedLimit() / 60.0 / 60.0); // [km/h]->[m/ms]
    }
    else
    {
        // 交差点の場合は抜けた先の単路部の制限速度
        // Speed limit of the section ahead in an intersection
        double vMax  = _localRoute->lastLane()->speedLimit() / 60.0 / 60.0;
        double vPref = vMax;

        Signal::Permission permission = _location->intersection()->permission(
            _location->intersection()->direction(
                _location->prevIntersection()));
        /**
         * @todo Intersection::permission に代替を検討
         */
        if (permission == Signal::Permission::CREEPING
            || permission == Signal::Permission::PAUSING)
        {
            // 信号が黄点滅や赤点滅の場合は徐行
            // Slow down in the traffic light is flashing yellow or red
            vPref = AppMates::getGVManager().getNumeric( //
                        "VELOCITY_CREEP")
                / 60.0 / 60.0;
        }
        if (_localRoute->turning() == RD::RIGHT)
        {
            // 右折時の走行速度
            // Velocity for turning right
            vPref = AppMates::getGVManager().getNumeric(
                        "VELOCITY_AT_TURNING_RIGHT")
                / 60.0 / 60.0;
        }
        else if (_localRoute->turning() == RD::LEFT)
        {
            // 左折時の走行速度
            // Velocity for turning left
            vPref = AppMates::getGVManager().getNumeric(
                        "VELOCITY_AT_TURNING_LEFT")
                / 60.0 / 60.0;
        }
        if (vPref < vMax)
        {
            _scene->setVMax(vPref);
        }
        else
        {
            _scene->setVMax(vMax);
        }
    }
}

//======================================================================
void VehiclePerceiver::_searchFrontAgent(double threshold)
{
#ifdef VEHICLE_RECOG_MEASURE_TIME
    ClockerManager::instance().startClock("VEHICLE_FRONTAGENT");
#endif //VEHICLE_RECOG_MEASURE_TIME

    const ObjectInLane* front = _location->lane()->frontAgent(_vehicle);
    if (front)
    {
        const VirtualLeader* leader = new VirtualLeader(
            front->distance() - _location->distance()
                - (front->bodyLength() + _body->bodyLength()) / 2,
            front->velocity(), VirtualLeader::VEHICLE_FRONT,
            "Vehicle ID:" + front->id());
        _scene->addLeader(leader);
    }
    /*
     * 同一車線に先行車がいない場合は前方の車線を探索
     *
     * If there is no preceding car in the same lane, search it in the
     * lanes ahead.
     */
    else
    {
        double totalDistance
            = _location->lane()->length() - _location->distance();
        const Lane*       lookupLane   = _location->lane();
        const LaneBundle* lookupBundle = lookupLane->parent();

        while (totalDistance < threshold)
        {
            /**
             * @todo 関数にまとめられないか
             */
            // 探索がレーン束を越える場合
            // When searching beyond laneBundle object
            if (!(lookupBundle->containsNextLane(lookupLane)))
            {
                lookupBundle = lookupBundle->nextBundle(lookupLane);

                // ODノードに到達したら探索を打ち切る
                // Stop searching when reached OD node
                if (typeid(*lookupBundle) == typeid(ODNode)
                    || typeid(*lookupBundle) == typeid(CSNodeFast) // [eMATES]
                    || typeid(*lookupBundle) == typeid(CSNodeNormal)) // [eMATES]
                {
                    break;
                }
            }

            /*
             * 次の探索対象車線を決定する
             *   LocalRouteに含まれる車線を検索するが，それがない場合は
             *   lookupLaneの直進車線とする．
             *
             * Find the lane to search next
             *   Search for the lane included in LocalRoute, and if not
             *   search for the straight ahead lane of the lookupLane.
             */
            const Lane* nextLane = _localRoute->next(lookupLane);
            if (!nextLane)
            {
                nextLane = lookupLane->nextStraightLane();
            }
            lookupLane = nextLane;
            if (!lookupLane)
            {
                break;
            }

            front = lookupLane->tailAgent();
            if (front)
            {
                const VirtualLeader* leader = new VirtualLeader(
                    totalDistance + front->distance()
                        - (front->bodyLength() + _body->bodyLength()) / 2,
                    front->velocity(), VirtualLeader::VEHICLE_FRONT,
                    "Vehicle ID:" + front->id());
                _scene->addLeader(leader);
                break;
            }
            totalDistance += lookupLane->length();
        }
    }

#ifdef VEHICLE_RECOG_MEASURE_TIME
    ClockerManager::instance().stopClock("VEHICLE_FRONTAGENT");
#endif //VEHICLE_RECOG_MEASURE_TIME
}

//======================================================================
void VehiclePerceiver::_searchFrontSpeedLimit(double threshold)
{
#ifdef VEHICLE_RECOG_MEASURE_TIME
    ClockerManager::instance().startClock("VEHICLE_FRONTSPEEDLIMIT");
#endif //VEHICLE_RECOG_MEASURE_TIME

    // 現在のレーンの制限速度はすでに希望速度に反映されている
    // Speed limit of current lane is already obtained as desired speed
    double totalDistance = _location->lane()->length() - _location->distance();
    const Lane*       lookupLane   = _location->lane();
    const LaneBundle* lookupBundle = lookupLane->parent();

    while (totalDistance < threshold)
    {
        // 探索がレーン束を越える場合
        // When searching beyond laneBundle object
        if (!(lookupBundle->containsNextLane(lookupLane)))
        {
            lookupBundle = lookupBundle->nextBundle(lookupLane);
            // ODノードに到達したら探索を打ち切る
            // Stop searching when reached OD node
            if (typeid(*lookupBundle) == typeid(ODNode)
                || typeid(*lookupBundle) == typeid(CSNodeFast) // [eMATES]
                || typeid(*lookupBundle) == typeid(CSNodeNormal)) // [eMATES]
            {
                break;
            }
        }

        /*
         * 次の探索対象車線を決定する
         *   LocalRouteに含まれる車線を検索するが，それがない場合は
         *   lookupLaneの直進車線とする．
         *
         * Find the lane to search next
         *   Search for the lane included in LocalRoute, and if not
         *   search for the straight ahead lane of the lookupLane.
         */
        const Lane* nextLane = _localRoute->next(lookupLane);
        if (!nextLane)
        {
            nextLane = lookupLane->nextStraightLane();
        }
        lookupLane = nextLane;
        if (!lookupLane)
        {
            break;
        }

        /*
         * 対象車線制限速度が現在の速度よりも小さければ事前に減速
         *
         * If speed limit of the target lane s lower than that of
         * current lane, slow down in advance
         */
        if (lookupLane->speedLimit() / 3600.0
            < _behavior->velocity()) // [km/h]->[m/ms]
        {
            const VirtualLeader* leader = new VirtualLeader(
                totalDistance - _body->bodyLength() / 2,
                lookupLane->speedLimit() / 3600.0, VirtualLeader::SPEED_LIMIT,
                "Lane ID:" + lookupLane->id());
            _scene->addLeader(leader);
            break;
        }
        totalDistance += lookupLane->length();
    }

#ifdef VEHICLE_RECOG_MEASURE_TIME
    ClockerManager::instance().stopClock("VEHICLE_FRONTSPEEDLIMIT");
#endif //VEHICLE_RECOG_MEASURE_TIME
}

//======================================================================
bool VehiclePerceiver::_shouldStopBySignal(
    const Intersection* nextInter, const Intersection* prevInter,
    double threshold)
{
    bool isStopped          = false;
    bool isSearchContinuing = true;
    if (!nextInter)
    {
        return isStopped;
    }

#ifdef VEHICLE_RECOG_MEASURE_TIME
    ClockerManager::instance().startClock("VEHICLE_SIGNAL");
#endif //VEHICLE_RECOG_MEASURE_TIME

    // 次の交差点の信号
    // Traffic light at the next intersection
    int fromDir = nextInter->direction(prevInter);

    const RelativeDirection& rd = _localRoute->turning();
    double                   distanceToSignal;
    if (_location->section())
    {
        distanceToSignal = _location->section()->distanceToNext(
            _location->lane(), _location->distance());
    }
    else
    {
        distanceToSignal = _location->intersection()->distanceToNext(
                               _location->lane(), _location->distance())
            + _location->intersection()->nextSection(nextInter)->length();
    }

    const Intersection* lookupFrom = prevInter;
    const Intersection* lookupTo   = nextInter;
    while (distanceToSignal < threshold)
    {
        const Signal* signal = lookupTo->signal();
        if (signal)
        {
            switch (lookupTo->permission(fromDir, rd, _vehicle))
            {

            case Signal::Permission::PROHIBITION:
            {
                //++++++++++++++++++++++++++++++++++++++++++++++++++++++
                // 進入許可なし（赤信号）
                // No entry permitted (red)
                const VirtualLeader* leader = new VirtualLeader(
                    distanceToSignal - _body->bodyLength() / 2, 0,
                    VirtualLeader::SIGNAL_RED,
                    "Intersection ID:" + lookupTo->id());
                _scene->addLeader(leader);

                /*
                 * 目前の交差点で停止する場合はフラグを立てる
                 *
                 * Enable a flag if stop at the intersection in front
                 * of the car
                 */
                if (lookupTo == nextInter)
                {
                    isStopped = true;
                }
                break;
            }

            case Signal::Permission::PERMISSION:
                //++++++++++++++++++++++++++++++++++++++++++++++++++++++
                // 進入許可あり（青信号）
                // Entry permitted (blue)
                break;

            case Signal::Permission::CREEPING:
            {
                //++++++++++++++++++++++++++++++++++++++++++++++++++++++
                // 徐行制限（黄点滅）
                // Slow down (flashing yellow)
                const VirtualLeader* leader = new VirtualLeader(
                    distanceToSignal + _body->bodyLength(),
                    AppMates::getGVManager().getNumeric("VELOCITY_CREEP") / 60
                        / 60,
                    VirtualLeader::SIGNAL_YELLOWBLINK,
                    "Intersection ID:" + lookupTo->id());
                _scene->addLeader(leader);
                isSearchContinuing = false;
                break;
            }
            case Signal::Permission::PAUSING:
            {
                //++++++++++++++++++++++++++++++++++++++++++++++++++++++
                // 一旦停止（赤点滅）
                // Pause (flashing red)
                if (_behavior->numPausing() > 0)
                {
                    // 一旦停止後は徐行
                    // Go slowly after paused
                    const VirtualLeader* leader = new VirtualLeader(
                        distanceToSignal + _body->bodyLength(),
                        AppMates::getGVManager().getNumeric("VELOCITY_CREEP")
                            / 60 / 60,
                        VirtualLeader::SIGNAL_REDBLINK_AFT,
                        "Intersection ID:" + lookupTo->id());
                    _scene->addLeader(leader);
                    isSearchContinuing = false;
                    break;
                }
                else
                {
                    // 一旦停止前は停止
                    // Stop before paused
                    const VirtualLeader* leader = new VirtualLeader(
                        distanceToSignal - _body->bodyLength() / 2, 0,
                        VirtualLeader::SIGNAL_REDBLINK_BEF,
                        "Intersection ID:" + lookupTo->id());
                    _scene->addLeader(leader);

                    /*
                     * 目前の交差点で停止する場合はフラグを立てる
                     *
                     * Enable a flag if stop at the intersection
                     * in front of the car
                     */
                    if (lookupTo == nextInter)
                    {
                        isStopped = true;
                    }
                    isSearchContinuing = false;
                    break;
                }
            }
            default:
                break;
            }
        }
        if (!isSearchContinuing)
        {
            break;
        }

        //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
        // 更に前方を見るための処理
        // Processing to search further
        const Intersection* lookupTmp = lookupTo;
        lookupTo   = _globalRoute->next(lookupFrom, lookupTo);
        lookupFrom = lookupTmp;
        if (!lookupTo)
        {
            break;
        }

        distanceToSignal += lookupFrom->center().distance(lookupTo->center());
        if (distanceToSignal > threshold)
        {
            break;
        }
        fromDir = lookupTo->direction(lookupFrom);

        if (fromDir == -1)
        {
            cout << "vehicle: " << _vehicle->id();
            _globalRoute->print(cout);
        }
    }

#ifdef VEHICLE_RECOG_MEASURE_TIME
    ClockerManager::instance().stopClock("VEHICLE_SIGNAL");
#endif //VEHICLE_RECOG_MEASURE_TIME

    return isStopped;
}

//======================================================================
bool VehiclePerceiver::_shouldStopByCollisionInIntersection(
    const Intersection* nextInter, const vector<const Lane*>& clInter)
{
#ifdef VEHICLE_RECOG_MEASURE_TIME
    ClockerManager::instance().startClock("VEHICLE_CLINTER");
#endif //VEHICLE_RECOG_MEASURE_TIME

    bool isStopped = false;

    for (auto itr_c : clInter)
    {
        const ObjectInLane* clTail = itr_c->tailAgent();
        for (const ObjectInLane* clAgent = clTail; //
             clAgent != nullptr;                   //
             clAgent = itr_c->frontAgent(clAgent))
        {
            // 簡易検索の場合は最後尾エージェントのみ
            // For simple search, search only the last agent
            if (!_checksCollisionStrictly && clAgent != clTail)
            {
                break;
            }
            // 自分自身は無視する
            // Ignore the subject car itself
            if (clAgent == _vehicle)
            {
                continue;
            }

            //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
            // 交錯する地点までの距離を計算する
            // Find the distance to a collision point
            bool                       hasPassedCollisionPoint = false;
            const vector<const Lane*>& liInter
                = _localRoute->lanesInIntersection();

            for (auto itr_l : liInter)
            {
                // 交錯地点
                // Collision point
                AmuPoint cp;
                if (itr_l->createIntersectionPoint(itr_c->lineSegment(), &cp))
                {
                    // 交錯地点までの距離
                    // Distance to the collision point
                    double ltc
                        = itr_c->lineSegment()->pointBegin().distance(cp);

                    // clAgentがすでに交錯地点を通過している場合
                    // If clAgent has already passed the collision point
                    if (ltc < clAgent->distance() - clAgent->bodyLength() / 2)
                    {
                        hasPassedCollisionPoint = true;
                    }
                    /*
                     * 交錯地点が1つでも見つかればそれ以上探索しない
                     *
                     * If even one collision point is found, not search
                     * any further
                     */
                    break;
                }
            }

            if (hasPassedCollisionPoint)
            {
                continue;
            }

            //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
            /*
             * まだ交錯地点を通過していない車両に対する処理
             *
             * Processing for var that have not yet passed the collision
             * point
             */

            // 自分が交差点に到達するまでの時間 (Time To Intersection)
            // Time to reach the intersection (Time To Intersection)
            double thisTti;

            /*
             * 交差点に接近している場合は速度に関係なくゼロ
             *
             * Zero if approaching the intersection, regardless of
             * velocity
             */
            if (_location->section()->distanceToNext(
                    _location->lane(), _location->distance())
                < 20)
            {
                thisTti = 0.0;
            }

            // 速度がゼロでなければ実際にかかる時間
            // Actual time if the velocity is non-zero
            else if (_behavior->velocity() > 1.0e-6)
            {
                thisTti = _location->section()->distanceToNext(
                              _location->lane(), _location->distance())
                    / _behavior->velocity();
            }
            // 停車中ならひとまず100 s
            // 100 s if the car is pausing
            else
            {
                thisTti = 100.0 * 1000;
            }

            // 相手が交差点を通過するまでの時間 (Time To Pass)
            // Time until clAgent passes the intersection (Time To Pass)
            double thatTtp;

            // 停車中ならひとまず100 s
            // 100 s if clAgent is pausing
            if (clAgent->velocity() < 1.0e-6)
            {
                thatTtp = 100.0 * 1000;
            }

            // 速度がゼロでなければ実際にかかる時間
            // Actual time if the velocity is non-zero
            else
            {
                thatTtp = nextInter->distanceToNext(itr_c, clAgent->distance())
                    / clAgent->velocity();
            }

            // thisTtiがthatTtpより小さければであれば交錯の可能性がある
            // Possible collision if thisTti is less than thatTtp
            if (thisTti < thatTtp)
            {
                const VirtualLeader* leader = new VirtualLeader(
                    _location->section()->distanceToNext(
                        _location->lane(), _location->distance())
                        - _body->bodyLength() / 2,
                    0, VirtualLeader::VEHICLE_CLINT,
                    "Vehicle ID:" + clAgent->id());
                _scene->addLeader(leader);
                isStopped = true;
                break;
            }
        }
    }

#ifdef VEHICLE_RECOG_MEASURE_TIME
    ClockerManager::instance().stopClock("VEHICLE_CLINTER");
#endif //VEHICLE_RECOG_MEASURE_TIME

    return isStopped;
}

//======================================================================
bool VehiclePerceiver::_shouldStopByCollisionInSection(
    const Intersection* nextInter, const vector<const Lane*>& clSection)
/*
 * 単路部内を走行中で，交差点内で他車と交錯する可能性のある車両が
 * 交差点手前で停止するかどうかの判定は以下の手順に従う
 *
 * 0. 交差点内交錯レーンの上流にある単路部内のレーンは取得済みである．
 * 1. 取得した単路内レーンについて，それぞれの先頭車両を取得する．
 * 2. 自車と交錯単路内レーンの先頭車両との位置関係および進行方向
 *    （ウィンカーで判別）から道を譲るべきかどうか判定する．
 * 3. 道を譲ると判定され，実際に該当交差点に到達するまでの時間差が
 *    小さい場合に減速し停止する．相手よりもじゅうぶん早く交差点を
 *    通過できる場合は優先関係を無視する．
 *
 * Follow the procedure below to determine whether a car that is
 * traveling on a section and may collide another car at an intersection
 * should stop before the intersection.
 *
 * 0. The lanes in the section upstream of the collision lanes in the
 *    intersection has already been obtained.
 * 1. Obtain the front car for each obtained lanes in the section.
 * 2. Determine whether to give way based on the positional relationship
 *    between the subject car and the front cars on the obtained lanes
 *    and the direction (determined by the blinker).
 * 3. If determined to give way and if the time difference to actually
 *    reach the intersection is small, the subject car decelerates and
 *    stops. Ignore the priority relationship if the subject car can
 *    pass sufficiently earlier than the opponent.
 */
{
    bool isStopped = false;

#ifdef VEHICLE_RECOG_MEASURE_TIME
    ClockerManager::instance().startClock("VEHICLE_CLSECT");
#endif //VEHICLE_RECOG_MEASURE_TIME

    // 自分の進入方向
    // Approach direction of the subject car
    int thisDir = nextInter->direction(_location->section());

    // ギャップアクセプタンス
    // Gap acceptance threshold
    double gap
        = AppMates::getGVManager().getNumeric("GAP_ACCEPTANCE_VEHICLE_CROSS")
        * 1000; // [s]->[ms]

    for (unsigned int i = 0; i < clSection.size(); i++)
    {
        // 相手の進入方向
        // Approach direction of the opponent car
        int thatDir = nextInter->direction(clSection[i]->endConnector());
        Signal::Permission thatPermission = nextInter->permission(thatDir);

        assert(thatDir != -1);

        /*
         * 相手レーンの信号が赤，赤点滅の場合は自分が優先
         *
         * If the opponent's traffic light is red or flashing red,
         * the subject car has priority.
         */
        if (thatPermission == Signal::Permission::PROHIBITION
            || thatPermission == Signal::Permission::PAUSING)
        {
            continue;
        }

        // 交錯する可能性のある相手エージェント
        // An opponent agent that may collide the subject car
        const Vehicle* headVehicle = clSection[i]->headVehicle();
        if (!headVehicle || headVehicle == _vehicle
            /* || !(_canWatch(headVehicle)) */)
        // 見通しを考慮するならアンコメントする
        // Uncomment if considering visibility
        {
            continue;
        }

        if (_yields(nextInter, thisDir, thatDir, headVehicle))
        {
            /*
             * 相手に道を譲る状況であっても，衝突のおそれがなければ
             * 減速する必要はない
             *
             * Even if the situation to give way to the opponent, there
             * is no need to slow down if there is no risk of collision.
             */

            //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
            // 自分が交差点に到達するまでの時間
            // Time to reach the intersection
            double thisTti;

            // 交差点に近ければひとまず3 s
            // 3 s if approaching the intersection
            if (_location->section()->distanceToNext(
                    _location->lane(), _location->distance())
                < _body->bodyLength())
            {
                thisTti = 3.0 * 1000;
            }
            // 速度がゼロでなければ実際にかかる時間
            // Actual time if the velocity is non-zero
            else if (_behavior->velocity() > 1.0e-6)
            {
                thisTti = _location->section()->distanceToNext(
                              _location->lane(), _location->distance())
                    / _behavior->velocity();
            }
            // 停車中ならひとまず100 s
            // 100 s if the car is pausing
            else
            {
                thisTti = 100.0 * 1000;
            }

            //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
            // 相手エージェントが走行中の単路
            // A Section on which the opponent agent is traveling
            const Section* collisionSection = nextInter->nextSection(thatDir);

            // 相手が交差点に到達するまでの時間
            // Time until the opponent reaches the intersection
            double thatTti;

            /*
             * 相手が優先車両で，交差点直前で加速中の場合は別処理
             *   動き出したばかりで低速だとthatTtiが過大になるため
             *
             * Another process is needed when the opponent is a priority
             * car and is accelerating just before the intersection
             *   Because thatTti bocomes exessive if it is just starting
             *   to move and the velocity is low.
             */
            if (collisionSection->distanceToNext(
                    clSection[i], headVehicle->distance())
                < headVehicle->bodyLength())
            //&& headVehicle->velocity()>0)
            {
                thatTti = 0.0;
            }
            // 速度がゼロでなければ実際にかかる時間
            // Actual time if the velocity is non-zero
            else if (headVehicle->velocity() > 1.0e-6)
            {
                thatTti = collisionSection->distanceToNext(
                              clSection[i], headVehicle->distance())
                    / headVehicle->velocity();
            }
            // 停車中ならひとまず50 s
            // 50 s if the car is pausing
            else
            {
                thatTti = 50.0 * 1000;
            }

            /*
             * thatTtiとthisTtiの差がgap未満なら道を譲る
             *
             * Give way if the difference between thatTti and thisTti
             * is less than gap.
             */
            if (thatTti - thisTti < gap)
            {
                const VirtualLeader* leader = new VirtualLeader(
                    _location->section()->distanceToNext(
                        _location->lane(), _location->distance())
                        - _body->bodyLength() / 2,
                    0, VirtualLeader::VEHICLE_CLSEC,
                    "Vehicle ID:" + headVehicle->id());
                _scene->addLeader(leader);
                isStopped = true;
                break;
            }
        }
    }

    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    // 停車中は確率的に休止状態になり，デッドロックを回避する
    // Inactivate stochastically while stopping to avoid deadlock
    if (isStopped && _behavior->velocity() < 1.0e-6
        && _vehicle->randomNumberGenerator()->uniform() < 0.05)
    {
        _decision->setShouldSleep(true);
    }

#ifdef VEHICLE_RECOG_MEASURE_TIME
    ClockerManager::instance().stopClock("VEHICLE_CLSECT");
#endif //VEHICLE_RECOG_MEASURE_TIME

    return isStopped;
}

//======================================================================
bool VehiclePerceiver::_shouldStopByLaneInInter(const Intersection* inter)
{
    // 直進は常にfalse（交差点進入可）
    // False for going straight (The car can enter the intersection)
    if (_localRoute->turning() == RD::STRAIGHT)
    {
        return false;
    }

#ifdef VEHICLE_RECOG_MEASURE_TIME
    ClockerManager::instance().startClock("VEHICLE_LANEININTER");
#endif //VEHICLE_RECOG_MEASURE_TIME

    bool result = false;

    const Lane* targetLane = _location->nextLane();

    // 交差点の車線に辿り着くまでlocalRouteを読み飛ばす
    // Skip localRoute until reaching a lane in the intersection
    while (!(inter->containsLane(targetLane)))
    {
        targetLane = _localRoute->next(targetLane);
    }

    // 交差点のレーンを処理
    // Processing for lanes in the intersection
    while (inter->containsLane(targetLane))
    {
        // 先行車がいたら進入しない
        // Not enter if there is a preceding car
        if (targetLane->tailAgent())
        {
            const VirtualLeader* leader = new VirtualLeader(
                _location->section()->distanceToNext(
                    _location->lane(), _location->distance())
                    - _body->bodyLength() / 2,
                0, VirtualLeader::LANE_INT, "Lane ID:" + targetLane->id());
            _scene->addLeader(leader);
            result = true;
            break;
        }
        targetLane = _localRoute->next(targetLane);
    }

#ifdef VEHICLE_RECOG_MEASURE_TIME
    ClockerManager::instance().stopClock("VEHICLE_LANEININTER");
#endif //VEHICLE_RECOG_MEASURE_TIME

    return result;
}

//======================================================================
bool VehiclePerceiver::_shouldStopByShortSpace()
{
#ifdef VEHICLE_RECOG_MEASURE_TIME
    ClockerManager::instance().startClock("VEHICLE_SHORTSPACE");
#endif //VEHICLE_RECOG_MEASURE_TIME

    bool result = false;

    // 停車するのに十分な距離
    // Enough distance for the car to stop
    double enoughDistance = _body->bodyLength() + _body->jamDistance();

    /*
     * _localRouteの最後には交差点通過後の最初のレーンが入っている
     *
     * The last element of _localRoute is the first lane after passing
     * the intersection
     */
    const ObjectInLane* tail = _localRoute->lastLane()->tailAgent();
    if (!tail)
    {
        // result = false;
    }
    else if (
        tail->distance() - tail->bodyLength() / 2 < enoughDistance
        && (tail->velocity() < 1.0e-6 || tail->accel() < 0))
    {
        const VirtualLeader* leader = new VirtualLeader(
            _location->section()->distanceToNext(
                _location->lane(), _location->distance())
                - _body->bodyLength() / 2,
            0, VirtualLeader::VEHICLE_TAIL, "Vehicle ID:" + tail->id());
        _scene->addLeader(leader);

        // 他の車両の進行を妨げないように休止状態になる
        // Inactivate not to disturb the other cars move
        if (_behavior->velocity() < 1.0e-6)
        {
            _decision->setShouldSleep(true);
        }
        result = true;
    }

#ifdef VEHICLE_RECOG_MEASURE_TIME
    ClockerManager::instance().stopClock("VEHICLE_SHORTSPACE");
#endif //VEHICLE_RECOG_MEASURE_TIME

    return result;
}

//======================================================================
bool VehiclePerceiver::_shouldStopByMinHeadway(const Intersection* nextInter)
{
#ifdef VEHICLE_RECOG_MEASURE_TIME
    ClockerManager::instance().startClock("VEHICLE_MIN_HEADWAY");
#endif //VEHICLE_RECOG_MEASURE_TIME

    assert(_location->section());

    bool result = false;
    if ((_localRoute->turning() == RD::LEFT
         || _localRoute->turning() == RD::RIGHT)
        && _behavior->velocity() > 1.0e-6
        && AppMates::getTimeManager().time()
                + _location->section()->distanceToNext(
                      _location->lane(),
                      _location->distance())
                    / _behavior->velocity()
                - _location->nextLane()->lastArrivalTime()
            < AppMates::getGVManager().getNumeric("MIN_HEADWAY_AT_TURNING")
                * 1000) // [s]->[ms]
    {
        const VirtualLeader* leader = new VirtualLeader(
            _location->section()->distanceToNext(
                _location->lane(), _location->distance())
                - _body->bodyLength() / 2,
            0, VirtualLeader::TURNING_MIN_HEADWAY,
            "Intersection ID:" + nextInter->id());
        _scene->addLeader(leader);
        result = true;
    }

#ifdef VEHICLE_RECOG_MEASURE_TIME
    ClockerManager::instance().stopClock("VEHICLE_MIN_HEADWAY");
#endif //VEHICLE_RECOG_MEASURE_TIME

    return result;
}

//======================================================================
void VehiclePerceiver::_determineTurningVelocity()
{
#ifdef VEHICLE_RECOG_MEASURE_TIME
    ClockerManager::instance().startClock("VEHICLE_BEFORE_TURN");
#endif //VEHICLE_RECOG_MEASURE_TIME

    // 直進の場合は何もしない
    // Do nothing if going straight
    if (_localRoute->turning() == RD::STRAIGHT)
    {
    }

    // 右折前減速
    // Deceleration before turning right
    else if (_localRoute->turning() == RD::RIGHT)
    {
        const VirtualLeader* leader = new VirtualLeader(
            _location->section()->distanceToNext(
                _location->lane(), _location->distance())
                + _body->bodyLength() * 2,
            AppMates::getGVManager().getNumeric("VELOCITY_AT_TURNING_RIGHT")
                / 60.0 / 60.0,
            VirtualLeader::TURNING_RIGHT,
            "Section ID:" + _location->section()->id());
        _scene->addLeader(leader);
    }

    // 左折前減速
    // Deceleration before turning left
    else if (_localRoute->turning() == RD::LEFT)
    {
        const VirtualLeader* leader = new VirtualLeader(
            _location->section()->distanceToNext(
                _location->lane(), _location->distance())
                + _body->bodyLength() * 2,
            AppMates::getGVManager().getNumeric("VELOCITY_AT_TURNING_LEFT")
                / 60.0 / 60.0,
            VirtualLeader::TURNING_LEFT,
            "Section ID:" + _location->section()->id());
        _scene->addLeader(leader);
    }

#ifdef VEHICLE_RECOG_MEASURE_TIME
    ClockerManager::instance().stopClock("VEHICLE_BEFORE_TURN");
#endif //VEHICLE_RECOG_MEASURE_TIME
}

//======================================================================
void VehiclePerceiver::_searchPreferentialAgentInIntersection()
{
#ifdef VEHICLE_RECOG_MEASURE_TIME
    ClockerManager::instance().startClock("VEHICLE_IN_INTER");
#endif //VEHICLE_RECOG_MEASURE_TIME

    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    // 通過予定の交差点内車線を取得する
    // Get the lanes in the intersection to be passed
    const vector<const Lane*>& lanesInInter
        = _localRoute->lanesInIntersection();
    vector<const Lane*> frontLanes;

    /*
     * 前方のレーンを抽出するため，自分自身が登場するまでは読み飛ばし
     *
     * In order to extract the lane ahead, skip until the subject car
     * appears
     */
    unsigned int pos;
    for (pos = 0; pos < lanesInInter.size(); pos++)
    {
        if (lanesInInter[pos] == _location->lane())
        {
            break;
        }
    }
    for (; pos < lanesInInter.size(); pos++)
    {
        frontLanes.push_back(lanesInInter[pos]);
    }

    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    if (frontLanes.empty())
    {
        // 前方の車線何もなければ何もしない
        // Do nothing if there are no lanes ahead
    }
    else
    {
        // 交錯車線を取得する
        // Get collision lanes
        vector<const Lane*> clInter;
        vector<const Lane*> clSection;
        _location->intersection()->getCollisionLanes(
            frontLanes, clInter, clSection);

        int thisDir = _location->intersection()->direction(
            _location->prevIntersection());

        for (auto itr : clInter)
        {
            const Vehicle* clTail = itr->tailVehicle();
            for (const Vehicle* clVehicle = clTail; //
                 clVehicle != nullptr;              //
                 clVehicle = itr->frontVehicle(clVehicle))
            {
                // 簡易検索の場合は最後尾エージェントのみ
                // For simple search, search only the last agent
                if (!_checksCollisionStrictly && clVehicle != clTail)
                {
                    break;
                }
                // 自分自身は無視する
                // Ignore the subject vehicle itself
                if (clVehicle == _vehicle)
                {
                    continue;
                }
                int thatDir = clVehicle->directionFrom();

                if (_yields(
                        _location->intersection(), thisDir, thatDir, clVehicle))
                {
                    /*
                     * 車線同士の交点の位置を計算
                     *
                     * Calculate the position of intersections point
                     * between lanes
                     */
                    const Lane*                lookupLane = nullptr;
                    const vector<const Lane*>& thatLanes
                        = clVehicle->localRoute()->lanesInIntersection();
                    auto itr_l = find(thatLanes.begin(), thatLanes.end(), itr);
                    if (itr_l == thatLanes.end())
                    {
                        break;
                    }

                    // 交点を持つ自分側の車線
                    // Own lane with intersection point
                    const Lane* thisClLane = nullptr;
                    // 自分側の交点までの距離
                    // Own distance to intersection point
                    double thisClDistance = 0.0;

                    // 交点を持つ相手側の車線
                    // Opponent's lane with intersection point
                    const Lane* thatClLane = nullptr;
                    // 相手側の交点までの距離
                    // Opponent's distance to intersection point
                    double thatClDistance = 0.0;

                    for (; itr_l != thatLanes.end(); itr_l++)
                    {
                        for (lookupLane = _location->lane();
                             _location->intersection()->containsLane(
                                 lookupLane);
                             lookupLane = _localRoute->next(lookupLane))
                        {
                            AmuPoint cp;
                            if (lookupLane->createIntersectionPoint(
                                    (*itr_l)->lineSegment(), &cp))
                            {
                                // 交点が見つかった
                                // Intersection point found
                                thisClLane     = lookupLane;
                                thisClDistance = thisClLane->lineSegment()
                                                     ->pointBegin()
                                                     .distance(cp);

                                thatClLane     = (*itr_l);
                                thatClDistance = thatClLane->lineSegment()
                                                     ->pointBegin()
                                                     .distance(cp);
                            }
                            if (thisClLane)
                            {
                                break;
                            }
                        }
                        if (thatClLane)
                        {
                            break;
                        }
                    }

                    // 交錯車線なし
                    // No collision lanes
                    if (!thisClLane || !thatClLane)
                    {
                        continue;
                    }

                    /*
                     * 自分と相手のどちらかがすでに交錯点を通過
                     *
                     * The subject or the opponent car has already
                     * passed the collision point
                     */
                    if ((thisClLane == _location->lane()
                         && thisClDistance
                             < _location->distance() + _body->bodyLength() / 2)
                        || (thatClLane == clVehicle->location()->lane()
                            && thatClDistance < clVehicle->distance()
                                    + clVehicle->bodyLength() / 2))
                    {
                        continue;
                    }

                    /*
                     * thisClLaneから上流に遡りながら，停止すべき点
                     * までの距離を算出
                     *
                     * Calculate the distance to the point to stop
                     * with going upstream from thisClLane
                     */
                    double distanceToStop = thisClDistance;
                    while (lookupLane != _location->lane())
                    {
                        lookupLane = _localRoute->previous(lookupLane);
                        assert(_location->intersection()->containsLane(
                            lookupLane));
                        distanceToStop += lookupLane->length();
                    }
                    distanceToStop -= _location->distance();

                    const VirtualLeader* leader = new VirtualLeader(
                        distanceToStop - _body->bodyLength() / 2, 0,
                        VirtualLeader::VEHICLE_CLINT2,
                        "Vehicle ID:" + clVehicle->id());
                    _scene->addLeader(leader);
                }
            }
        }
    }

#ifdef VEHICLE_RECOG_MEASURE_TIME
    ClockerManager::instance().stopClock("VEHICLE_IN_INTER");
#endif //VEHICLE_RECOG_MEASURE_TIME
}

//======================================================================
bool VehiclePerceiver::_canWatch(const Vehicle* other) const
{
    assert(other);

    /*
     * 一般の交通流シミュレータは他者を正しく認知できるため，この関数は
     * 常にtrueを返す．必要に応じて書き換える．
     *
     * This function always returns true because cars in general traffic
     * simulators can recognize others correctly. Rewrite if necessary.
     */
    return true;
}

//======================================================================
bool VehiclePerceiver::_yields(
    const Intersection* inter, int thisDir, int thatDir, const Vehicle* other)
{
    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    // 交差点での車両の相互作用を無視するフラグ
    // Flag to ignore car interactions at intersection
    if (AppMates::getGVManager().getFlag("DEBUG_FLAG_IGNORE_YIELDING"))
    {
        return false;
    }

    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    Signal::Permission thisPermission = inter->permission(thisDir);
    Signal::Permission thatPermission = inter->permission(thatDir);

    AmuPoint crossPoint;

    /*
     * 交錯を厳密に評価する場合，交錯点上に車両がいたら他車が道を譲る
     *
     * When evaluating collisions strictly, if a car is on the collision
     * point, other car gives way.
     */
    if (_checksCollisionStrictly
        && _location->lane()->createIntersectionPoint(
            other->location()->lane()->lineSegment(), &crossPoint))
    {
        AmuPoint thisPoint = _location->position();
        AmuPoint thatPoint = other->location()->position();
        double   thisDistance
            = crossPoint.distance(thisPoint) - _body->bodyLength() / 2;
        double thatDistance
            = crossPoint.distance(thatPoint) - other->bodyLength() / 2;
        if (thisDistance < 0 || thatDistance < 0)
        {
            if (thisDistance > thatDistance)
            {
                return true;
            }
            else
            {
                return false;
            }
        }
    }

    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    // スリープ状態の自動車には道を譲らない
    // Not give way to  an inactive car
    if (other->behavior()->sleepDuration() > 0)
    {
        return false;
    }

    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    // 交差点内での進路が交わらない場合は道を譲らない
    // Not give way if the paths in the intersection do not intersect
    bool                       hasCrossPoint = false;
    const vector<const Lane*>& thisLanes = _localRoute->lanesInIntersection();
    const vector<const Lane*>& thatLanes
        = other->localRoute()->lanesInIntersection();
    for (unsigned int i = 0; i < thisLanes.size(); i++)
    {
        for (unsigned int j = 0; j < thatLanes.size(); j++)
        {
            if (thisLanes[i]->createIntersectionPoint(
                    thatLanes[j]->lineSegment(), &crossPoint))
            {
                hasCrossPoint = true;
                break;
            }
        }
        if (hasCrossPoint)
        {
            break;
        }
    }
    if (!hasCrossPoint)
    {
        return false;
    }

    /**
     * @todo これ以降は進路が交わることが前提であるので，単純化できる
     */

    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    // 赤点滅信号の優先順位は最下位
    // Flashing-red lights have the lowest priority

    /*
         * 自分が赤点滅以外で，相手が赤点滅の場合
         *
         * In the case other than flashing-red light for the subject car
         * and flashing-red light for the opponent.
         */
    if (thisPermission != Signal::Permission::PAUSING
        && thatPermission == Signal::Permission::PAUSING)
    {
        return false;
    }
    /*
         * 自分が赤点滅で，相手が赤点滅以外の場合
         *
         * In the case flashing-red light for the subject car
         * and other than flashing-red light for the opponent
         */
    if (thisPermission == Signal::Permission::PAUSING
        && thatPermission != Signal::Permission::PAUSING)
    {
        return true;
    }

    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    /*
     * 自分と相手が同方向から進入しようとする場合は別処理
     *   一般的なケースではないが，道路中央から右左折する路面電車と
     *   直進する自動車の関係などとしてあり得る．
     *
     * Separate processing if the subject and the opponent car try to
     * enter from the same direction
     *   Although it is an unusual case, there may be a relationship
     *   between a tram turning from the center of the roadway and
     *   a car going straight.
     */
    /**
     * @todo 長いので分離
     */
    if (thisDir == thatDir)
    {
        // 直進車優先
        // A car going straight has the highest priority
        if (_localRoute->turning() == RD::STRAIGHT)
        {
            return false;
        }
        // 左折車は第2位
        // A car turning left has the second priority
        else if (_localRoute->turning() == RD::LEFT)
        {
            if (other->blinker()->direction() == Blinker::NONE)
            {
                /*
                 * 自分と相手を結ぶベクトルを作成し，自分の進行方向と
                 * なす角を求める．負なら相手が左にいるため道を譲る．
                 *
                 * Create a vector that connects from the subject car
                 * and the opponent, and find the angle that makes with
                 * the direction of movement of the subject car. If the
                 * negative angle means that the opponent car is on the
                 * left, so the subject car gives way.
                 */
                /**
                 * @todo 要検討．符号正しい？LocalRouteを用いた判定？
                 */
                AmuVector posVector(
                    _location->position(), other->location()->position());
                if (_vehicle->directionVector().calcAngle(posVector) < 0)
                {
                    return true;
                }
            }
            return false;
        }
        // 右折車は第3位
        // A car turning right has the third priority
        else if (_localRoute->turning() == RD::RIGHT)
        {
            if (other->blinker()->direction() == Blinker::NONE
                || other->blinker()->direction() == Blinker::LEFT)
            {
                /*
                 * 自分と相手を結ぶベクトルを作成し，自分の進行方向と
                 * なす角を求める
                 *
                 * Create a vector that connects from the subject car
                 * and the opponent, and find the angle that makes with
                 * the direction of movement of the subject car. If the
                 * positive angle means that the opponent car is on the
                 * right, so the subject car gives way.
                 */
                /**
                 * @todo 要検討．符号正しい？LocalRouteを用いた判定？
                 */
                AmuVector posVector(
                    _location->position(), other->location()->position());
                if (_vehicle->directionVector().calcAngle(posVector) > 0)
                {
                    return true;
                }
            }
            return false;
        }
        // その他？
        // Other cases?
        else
        {
            return true;
        }
    }

    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    // 一般的な優先順位の判定
    // Usual priority determination
    /**
     * @todo 長いので分離
     */
    else
    {
        /*
         * 右折車は直進車・左折車に道を譲る [cf. 道交法第三十七条]
         *
         * A car turning right gives way to cars going straight and
         * turning left
         */
        if (_localRoute->turning() == RD::RIGHT)
        {
            /*
             * 自分：右折，相手：直進
             *
             * The subject car  : turning right,
             * the opponent car : going straight
             */
            if (other->blinker()->direction() == Blinker::NONE)
            {
                return true;
            }
            /*
             * 自分：右折，相手：左折
             *
             * The subject car  : turning right,
             * the opponent car : turning left
             */
            else if (other->blinker()->direction() == Blinker::LEFT)
            {
                if (((inter->relativeDirection(thisDir, thatDir))
                     == (RD::RIGHT | RD::LEFT)))
                {
                    /*
                     * 相手が右あるいは左から交差点に進入する場合は
                     * 交錯は生じない
                     *
                     * If the opponent car is entering the intersection
                     * from the right or left, collisions do not occur.
                     */
                    return false;
                }
                else
                {
                    return true;
                }
            }
            /*
             * 自分：右折，相手：右折
             *
             * The subject car  : turning right,
             * the opponent car : turning right
             */
            else if (other->blinker()->direction() == Blinker::RIGHT)
            {
                /*
                 * 交差点に左方から進入する車両を優先する
                 *
                 * Give priority to the car entering the intersection
                 * from the left.
                 */
                if (inter->relativeDirection(thisDir, thatDir) == RD::LEFT)
                {
                    return true;
                }
                else
                {
                    return false;
                }
            }
            return true;
        }

        /*
         * 道幅の広い道路を走行する車両優先 [cf. 道交法第三十六条２]
         * Give priority to cars which are going on wide roadway
         */
        /**
         * @todo 本来は道路幅を厳密に定義すべき
         */
        else if (
            inter->numIn(thisDir) + inter->numOut(thisDir)
            > inter->numIn(thatDir) + inter->numOut(thatDir) + 4)
        {
            return false;
        }
        else if (
            inter->numIn(thisDir) + inter->numOut(thisDir) + 4
            < inter->numIn(thatDir) + inter->numOut(thatDir))
        {
            return true;
        }

        /*
         * 交差点に左方から進入し直進する車両を優先
         * [cf. 道交法第三十六条１]
         * 注）法的には「左方から進行する車両」を優先するとあるが，
         *     「左方から直進する」と替えることで，左方から左折する
         *     車両より直進する車両を優先させることとした
         *
         * Give priority to cars entering the intersection from
         * the left and going straight.
         */
        else if (inter->relativeDirection(thisDir, thatDir) == RD::LEFT)
        {
            /*
             * 自分：直進 or 左折，相手：左から直進
             *
             * The subject car  : going straight or turning right,
             * the opponent car : going straight from the left
             */
            if (other->blinker()->direction() == Blinker::NONE
                && _localRoute->turning() != RD::LEFT)
            {
                return true;
            }
            /*
             * 自分：直進 or 左折，相手：左から右左折
             *
             * The subject car  : going straight or turning right,
             * the opponent car : turning left or right from the left
             */
            else
            {
                return false;
            }
        }
        /*
         * それ以外の場合は直進車優先
         * Give priority to cars going straight in other cases
         */
        else
        {
            if (other->blinker()->direction() == Blinker::NONE
                && _localRoute->turning() != RD::STRAIGHT)
            {
                return true;
            }
            else
            {
                return false;
            }
        }
    }
}
