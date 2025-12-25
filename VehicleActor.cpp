/* ******************************************************************
 * Copyright 2014-2024 ADVENTURE Project
 * All Rights Reserved
 * ****************************************************************** */
/**
 * @file VehicleActor.cpp
 */
#include "VehicleActor.hpp"
#include "AppMates.hpp"
#include "Blinker.hpp"
#include "CustomMessage.hpp"
#include "GVManager.hpp"
#include "Intersection.hpp"
#include "Lane.hpp"
#include "LinkFlowMonitor.hpp"
#include "LinkFlowRecord.hpp"
#include "LocalLaneRouter.hpp"
#include "RandomNumberGenerator.hpp"
#include "RoadMap.hpp"
#include "Section.hpp"
#include "TimeManager.hpp"
#include "TrafficCounterComponent.hpp"
#include "Vehicle.hpp"
#include "VehicleBehavior.hpp"
#include "VehicleBodyProperty.hpp"
#include "VehicleDecision.hpp"
#include "VehicleGlobalRoute.hpp"
#include "VehicleLocalRoute.hpp"
#include "VehicleLocation.hpp"
#include "VehicleScene.hpp"
#include "io/LinkFlowMonitorWriter.hpp"
#ifdef INCLUDE_PEDESTRIANS
#include "ped/VehiclePedExt.hpp"
#endif //INCLUDE_PEDESTRIANS
#include <cassert>

using namespace std;

#define PAUSE_VELOCITY_THRESHOLD 1.0e-4
#define PAUSE_DURATION_THRESHOLD 1000

//======================================================================
void VehicleActor::setVehicle(
    Vehicle* vehicle, VehicleBehavior* behavior, VehicleBodyProperty* body,
    VehicleDecision* decision, VehicleGlobalRoute* globalRoute,
    VehicleLocalRoute* localRoute, VehicleLocation* location,
    VehicleScene* scene, LocalLaneRouter* localRouter, Blinker* blinker)
{
    _vehicle     = vehicle;
    _behavior    = behavior;
    _body        = body;
    _decision    = decision;
    _globalRoute = globalRoute;
    _localRoute  = localRoute;
    _location    = location;
    _scene       = scene;
    _localRouter = localRouter;
    _blinker     = blinker;

    _requiresReroute      = false;
    _requiresLocalReroute = false;
}

//======================================================================
void VehicleActor::act()
{
    _movesToNextLane = false;

    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    // 加速度および速度の更新
    // Update acceleration and velocity
    _behavior->setAccel(_decision->decidedAccel());
    _renewVelocity();

    // 速度履歴の保存
    // Record velocity history
    if (AppMates::getGVManager().getFlag("VEHICLE_VELOCITY_HISTORY_RECORD")
        && (AppMates::getTimeManager().step() - _location->startingStep())
                % (int)AppMates::getGVManager().getNumeric(
                    "VEHICLE_VELOCITY_HISTORY_INTERVAL")
            == 0)
    {
        _renewVelocityHistory();
    }

    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    // 休止状態タイマのカウントダウン
    // Countdown timer for inactive state
    if (_behavior->sleepDuration() > 0)
    {
        _behavior->addSleepDuration(-AppMates::getTimeManager().unit());
    }
    // 休止状態の開始
    // Start inactive state
    else if (_decision->shouldSleep())
    {
        _behavior->setSleepDuration(1500);
        _decision->setShouldSleep(false);
    }

    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    // 距離の更新
    // Update distance
    /**
     * @todo 2次精度にする
     */
    double gain = _vehicle->velocity() * AppMates::getTimeManager().unit();
    _location->setOldDistance(_vehicle->distance());
    _location->addDistance(gain);
    _location->addDistanceFromInflowBorder(gain);
    _location->addTripLength(gain);

    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    // レーンへの登録
    // Registration to lane
    if (_location->distance() < _location->lane()->length())
    {
        //--------------------------------------------------------------
        /*
         * 車両がレーンの終点に達していない場合は同一レーンに登録する
         *
         * Register in the same lane if the car has not reached the end
         * of the lane.
         */
        const_cast<Lane*>(_location->lane())->putAgent(_vehicle);

        /*
         * 加えて，車両が単路部にいる場合で，次の交差点まで50m以内で，
         * 次に右左折するときはウィンカーを点ける
         *
         * Additionally, turn on the blinker if the car is on a section,
         * is within 50m to the next intersection, and will turn left or
         * right.
         */
        if (_location->section())
        {
            if (_location->section()->distanceToNext(
                    _location->lane(), _location->distance())
                    <= 50
                && (_localRoute->turning().value() != _blinker->direction()))
            {
                if (_localRoute->turning() == RD::LEFT)
                {
                    _blinker->setLeft();
                }
                else if (_localRoute->turning() == RD::RIGHT)
                {
                    _blinker->setRight();
                }
                else
                {
                    _blinker->setNone();
                }
            }
        }

        // 単路部内では5秒おきに車線変更を判断する
        // Judge lane-change in every 5 seconds in a section
        if (_location->section()
            && (!_decision->isLaneChangeExecutable()
                && !_decision->isLaneChangeActive()))
        {
            if (((AppMates::getTimeManager().time() - _location->startingTime())
                     % 5000
                 == 0)
                && (AppMates::getTimeManager().time()
                    != _location->startingTime()))
            {
                _requiresLocalReroute = true;
            }
        }
    }
    else
    {
        //--------------------------------------------------------------
        // 次の車線に移る
        // Move to next lane
        _movesToNextLane = true;

        // 次の車線を基準にするので，oldDistanceはマイナス
        // oldDistance is negative because it is based on the next lane
        _location->addOldDistance(-(_location->lane()->length()));
        _location->addDistance(-(_location->lane()->length()));

        // VehicleLocationの_lane, _intersection, _sectionの更新
        // Update _lane, _intersection, _section of VehicleLocation
        if (!_location->section())
        {
            assert(_location->intersection());

            // 単路部内の停止回数をクリア
            // Clear the number of pausing in section
            _behavior->setNumPausing(0);

            if (_location->intersection()->containsLane(_location->nextLane()))
            {
                // 交差点内の次の車線へ
                // To the next lane in the intersection
                _runIntersection2Intersection();
            }
            else
            {
                // 交差点から単路部へ
                // From intersection to section
                _location->setDistanceFromInflowBorder(0);
                _runIntersection2Section();
                if (_blinker->direction() != Blinker::NONE)
                {
                    _blinker->setNone();
                }
            }
        }
        else
        {
            if (_location->section()->containsLane(_location->nextLane()))
            {
                // 単路部内の次の車線へ
                // To the next lane in the section
                _runSection2Section();
            }
            else
            {
                // 単路部から交差点へ
                // From section to intersection
                _location->setDistanceFromInflowBorder(0);
                _runSection2Intersection();
            }
        }

        const_cast<Lane*>(_location->lane())->registerAgentToAdd(_vehicle);
        const_cast<Lane*>(_location->lane())
            ->setLastArrivalTime(AppMates::getTimeManager().time());
    }

#ifdef INCLUDE_PEDESTRIANS
    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    /*
     * 上流で停止できず進入してしまう予定の横断歩道レーンへの登録
     *
     * Registration to the crosswalk lane that this vehicle will enter
     * without being able to stop upstream.
     */
    if (_vehicle->pedExt()->requiresNotification())
    {
        _vehicle->pedExt()->notifyCrosswalkLane();
    }
#endif //INCLUDE_PEDESTRIANS

    /**
     * @~japanese @todo もう少し賢くしたい
     */
    if (_location->section() && !_requiresLocalReroute
        && !(_decision->isLaneChangeExecutable())
        && !(_decision->isLaneChangeActive())
        && AppMates::getTimeManager().time() % 10000 == 0)
    {
        _requiresLocalReroute = true;
    }
}

//======================================================================
void VehicleActor::_renewVelocity()
{
    double velocity = _behavior->velocity()
        + _behavior->accel() * AppMates::getTimeManager().unit();
    if (velocity < 1.0e-6)
    {
        velocity = 0.0;
        _behavior->setAccel(0.0);

        /*
         * 交差点内部での停車はデッドロックの要因となるため，確率的に
         * 休止状態にする
         *
         * Stopping inside an intersection can cause deadlock, so the
         * car stochastically becomes an inactive state 
         */
        if (_location->intersection()
            && _vehicle->randomNumberGenerator()->uniform() < 0.1)
        {
            _behavior->setSleepDuration(1500);
        }
    }
    _behavior->setVelocity(velocity);

    // 一旦停止と判定された場合の処理
    // Processing when being judged pausing
    if (velocity < PAUSE_VELOCITY_THRESHOLD)
    {
        if (!_behavior->isPausing())
        {
            _behavior->setIsPausing(true);
            _behavior->setPauseStartTime(AppMates::getTimeManager().time());
        }
        _pause();
    }
    else
    {
        if (_behavior->isPausing())
        {
            _behavior->setIsPausing(false);
            _behavior->clearPauseStartTime();
        }
    }
}

//======================================================================
void VehicleActor::_renewVelocityHistory()
{
    _behavior->addVelocityRateHistory(_behavior->velocity() / _scene->vMax());
}

//======================================================================
void VehicleActor::_runIntersection2Intersection()
{
    _resetPauseState();

    _location->setPrevLane(_location->lane());
    _location->setLane(_location->nextLane());
    _location->setNextLane(_location->lane()->nextStraightLane());
}

//======================================================================
void VehicleActor::_runIntersection2Section()
{
    _resetPauseState();

    _location->setSection(
        _location->intersection()->nextSection(_location->lane()));

    // 交差点に通過時間を通知
    // Notify passing time to intersection
    int from
        = _location->intersection()->direction(_location->prevIntersection());
    int to = _location->intersection()->direction(_location->section());
    assert(0 <= from && from < _location->intersection()->numNexts());
    assert(0 <= to && to < _location->intersection()->numNexts());

    // リンク旅行時間の記録
    // Record link travel time
    _location->intersection()
        ->linkFlowRecord(from)
        ->recordOutflowFromIntersection(_vehicle, to);
    _location->setSectionInflowTime(AppMates::getTimeManager().time());

    // Intersection::_watchedVehiclesのリセット
    // Reset Intersection::_watchedVehicles
    if (_behavior->isNotifying())
    {
        _behavior->setIsNotifying(false);
        _decision->resetLaneChangeFlags();
        const_cast<Intersection*>(_location->intersection())
            ->eraseWatchedVehicle(_vehicle);
        /*
        const_cast<Section*>(_location->section())
        ->addWatchedVehicle(_vehicle);
        */
    }

    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    /*
     * 車線変更に失敗した場合などに予定経路を達成できなくなる．その場合
     * ここで経路を再探索する．
     *
     * The planned route cannot be achieved, if the lane-change fails.
     * In that case, search the route again here.
     */
    if (_location->prevIntersection())
    {
        Intersection* next = _location->intersection()->next(
            _location->intersection()->direction(_location->section()));
        if (dynamic_cast<ODNode*>(next) == nullptr
            && _globalRoute->next(_location->intersection(), next) == nullptr)
        {
            if (AppMates::getGVManager().getFlag("FLAG_VERBOSE"))
            {
                cout << "vehicle: " << _vehicle->id()
                     << ", rerouting after passing intersection: "
                     << _location->intersection()->id() << endl;

                cout << "prev:" << _location->prevIntersection()->id()
                     << ", curr:" << _location->intersection()->id()
                     << ", next:" << next->id() << endl;
            }
            // postact() 内で reroute する
            // Reroute in postact()
            _requiresReroute = true;
        }
    }

    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    _location->setPrevIntersection(_location->intersection());
    _location->setIntersection(nullptr);

    _location->setPrevLane(_location->lane());
    _location->setLane(_location->nextLane());

    // postact() 内で localReroute する
    // localReroute in postact()
    _requiresLocalReroute = true;
}

//======================================================================
void VehicleActor::_runSection2Section()
{
    _resetPauseState();
    _location->setPrevLane(_location->lane());
    _location->setLane(_location->nextLane());

    const Lane* nextLaneCandidate
        = _localRouter->nextLaneWithoutLaneChange(_location->lane());
    if (nextLaneCandidate)
    {
        _location->setNextLane(nextLaneCandidate);
    }
    else
    {
        // postact() 内でlocalRerouteおよびdecideNextLaneする
        // localReroute and decideNextLane in postact()
        _requiresLocalReroute = true;
    }
}

//======================================================================
void VehicleActor::_runSection2Intersection()
{
    _location->setIntersection(
        _location->section()->nextIntersection(_location->lane()));

    /*
     * 車線変更を行いながら交差点に進入する場合は強制終了
     *
     * In the case of entering an intersection while lane-changing,
     * forced to terminate it.
     */
    if (_decision->isLaneChangeActive())
    {
        if (AppMates::getGVManager().getFlag("FLAG_VERBOSE"))
        {
            cerr << "vehicle:" << _vehicle->id()
                 << " enters intersection while lane changing"
                 << " from section:" << _location->section()->id() << endl;
        }
        const_cast<VehicleLaneChangeActor*>(_vehicle->laneChangeActor())
            ->abortLaneChange();
    }

    // リンク旅行時間の記録
    // Record link travel time
    bool isUp = _location->section()->isUp(_location->lane());
    _location->section()->linkFlowRecord(isUp)->recordOutflowFromSection(
        _vehicle);

    if (_location->section()->linkFlowMonitor(isUp))
    {
        _location->section()->linkFlowMonitor(isUp)->recordPassedVehicle(
            _vehicle);
    }

    _resetPauseState();

    // 最後に通過した交差点を登録
    // Register the last passed intersection
    if (!(_location->prevIntersection()))
    {
        _globalRoute->setLastPassedIntersectionIndex(_location->intersection());
    }
    else
    {
        _globalRoute->setLastPassedIntersectionIndex(
            _location->prevIntersection(), _location->intersection());
    }

    // Section::_watchedVehicles のリセット
    // Reset Section::_watchedVehicles
    if (_behavior->isNotifying())
    {
        const_cast<Section*>(_location->section())
            ->eraseWatchedVehicle(_vehicle);
        _behavior->setIsNotifying(false);
        _decision->resetLaneChangeFlags();
    }

    _location->setSection(nullptr);

    _location->setPrevLane(_location->lane());
    _location->setLane(_location->nextLane());
    _requiresLocalReroute = true;

    // ODノードでなければ次のレーンを設定
    // Set next lane if the intersection is not an ODNode
    if (!(dynamic_cast<ODNode*>(
            const_cast<Intersection*>(_location->intersection()))))
    {
        const Lane* nextLaneCandidate
            = _localRouter->nextLaneWithoutLaneChange(_location->lane());
        if (nextLaneCandidate)
        {
            _location->setNextLane(nextLaneCandidate);
        }
        else
        {
            _location->setNextLane(_location->lane()->nextStraightLane());
        }
    }
}

//======================================================================
void VehicleActor::_pause()
{
    /*
     * 発生直後は停止時間および停止回数の計測対象としない
     *
     * Immediately after vehicle's generation, stop time and number of
     * stops are not measured.
     */
    if (!(_vehicle->isAwayFromOriginNode()))
    {
        return;
    }

    _behavior->addPauseDuration(AppMates::getTimeManager().unit());

    ulint pauseDuration
        = AppMates::getTimeManager().time() - _behavior->pauseStartTime();
    if (pauseDuration >= PAUSE_DURATION_THRESHOLD)
    {
        if (pauseDuration - AppMates::getTimeManager().unit()
            < PAUSE_DURATION_THRESHOLD)
        {
            /*
             * 前ステップでは停止と判定されず，今ステップではじめて
             * 停止と判定された場合は停止回数をカウントアップする．
             *
             * If it was not determined to have paused in the previous
             * step and it is determined to have paused for the first
             * time in this step, increment the number of pausing.
             */
            _behavior->incrementNumPausing();
        }
    }
}

//======================================================================
void VehicleActor::_resetPauseState()
{
    _behavior->setNumPausing(0);
    _behavior->setPauseDuration(0);
}

//======================================================================
void VehicleActor::notify()
{
    if (_location->section())
    {
        const_cast<Section*>(_location->section())->addWatchedVehicle(_vehicle);
    }
    else
    {
        const_cast<Intersection*>(_location->intersection())
            ->addWatchedVehicle(_vehicle);
    }
    _behavior->setIsNotifying(true);
}

//======================================================================
void VehicleActor::unnotify()
{
    if (_location->section())
    {
        const_cast<Section*>(_location->section())
            ->eraseWatchedVehicle(_vehicle);
    }
    else
    {
        const_cast<Intersection*>(_location->intersection())
            ->eraseWatchedVehicle(_vehicle);
    }
    _behavior->setIsNotifying(false);
}

//======================================================================
void VehicleActor::postact()
{
    // 感知器への通知
    // Notification to traffic counters
    _notifyTrafficCounter();

    // 経路の再探索
    // Rerouting
    if (_requiresReroute)
    {
        _vehicle->reroute(
            _location->prevIntersection(),
            _location->section()->anotherIntersection(
                _location->prevIntersection()));
        _globalRoute->resetLastPassedIntersectionIndex();
        _globalRoute->setLastPassedIntersectionIndex(
            _location->prevIntersection());

        _requiresReroute = false;
    }

    // ローカル経路の再探索
    // Local rerouting
    if (_requiresLocalReroute)
    {
        // ただしsection内走行中のみ
        // Only while traveling in a section
        if (_location->section())
        {
            _localRouter->clear();
            _localRouter->localReroute(
                _location->section(), _location->lane(), _location->distance());
        }
        const Lane* nextLaneCandidate
            = _localRouter->nextLaneWithoutLaneChange(_location->lane());
        if (nextLaneCandidate)
        {
            _location->setNextLane(nextLaneCandidate);
        }
        else
        {
            _location->setNextLane(_location->lane()->nextStraightLane());
        }
        _requiresLocalReroute = false;
    }

    return;

    // ログ出力
    // Output log
    /*
      if ( SOME_CONDITIONS e.g. _vehicle->id()=="000010")
      {
      Logger* logger = LoggerManager::logger("vehicle");
      stringstream ss;
      _vehicle->print(ss);
      logger->saveLog(_vehicle->id(),ss.str());
      }
    */
}

//======================================================================
void VehicleActor::_notifyTrafficCounter()
{
    vector<TrafficCounterComponent*> counters;
    counters.clear();

    if (_movesToNextLane)
    {
        /*
         * 次のレーンに移った直後は前レーンの感知器もチェックする．
         * このとき，_location::_oldDistanceは負になっているので注意．
         *
         * Check the traffic counter in the previous lane as well,
         * immediately after moving to the next lane. At this time,
         * note that _location::_oldDistance is negative.
         */
        _location->prevLane()->counters(
            &counters,
            _location->oldDistance() + _location->prevLane()->length(),
            _location->distance() + _location->prevLane()->length());
    }
    _location->lane()->counters(
        &counters, _location->oldDistance(), _location->distance());

    if (counters.empty())
    {
        return;
    }

    for (auto itr : counters)
    {
        itr->recordPassedVehicle(_vehicle);
    }
    return;
}
