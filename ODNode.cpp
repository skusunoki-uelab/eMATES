/* **************************************************************************
 * Copyright 2014-2024 ADVENTURE Project
 * All Rights Reserved
 * ************************************************************************** */
/**
 * @file ODNode.cpp
 */
#include "ODNode.hpp"
#include "AppMates.hpp"
#include "CustomMessage.hpp"
#include "GVManager.hpp"
#include "Intersection.hpp"
#include "Lane.hpp"
#include "LaneBundle.hpp"
#include "ObjectManager.hpp"
#include "Section.hpp"
#include "TimeManager.hpp"
#include "Vehicle.hpp"
#include "VehicleBehavior.hpp"
#include "VehicleDecision.hpp"
#include "VehicleGlobalRoute.hpp"
#include "VehicleLocation.hpp"
#include "RoadMap.hpp"
#include "io/InflowMonitorWriter.hpp"
#include "io/VehicleTripWriter.hpp"
#include "io/VehicleTypeWriter.hpp"
#ifdef INCLUDE_TRAMS
#include "tram/ODNodeTramExt.hpp"
#include "tram/TramLaneInIntersection.hpp"
#include "tram/TramLaneInSection.hpp"
#include "tram/VehicleTram.hpp"
#endif //INCLUDE_TRAMS
#include <algorithm>
#include <cassert>
#include <deque>
#include <iostream>
#include <list>
#include <string>
#include <typeinfo>
#include <vector>

using namespace std;

//==============================================================================
ODNode::ODNode(const string& id, const string& type, RoadMap* parent)
    : Intersection(id, type, parent)
{
    _lastGenerationTime = 0;
    _lastIncomeTime     = 0;
    _waitsToPushVehicle = false;
    _inflowMonitor      = nullptr;
    _rng.reset();

#ifdef INCLUDE_TRAMS
    _odNodeTramExt = new ODNodeTramExt(this);
#endif //INCLUDE_TRAMS
}

//==============================================================================
ODNode::~ODNode()
{
    for (auto itr : _waitingVehicles)
    {
        delete itr;
    }
    _waitingVehicles.clear();

#ifdef INCLUDE_TRAMS
    delete _odNodeTramExt;
#endif //INCLUDE_TRAMS
}

//==============================================================================
bool ODNode::checkLaneConnectivity(bool isIntersection) const
{
    bool result = true;

    // 単路部から流入したあとの車線
    // Lanes after inflow from section
    for (auto itr : _borders[0]->inPoints())
    {
        for (auto itr_l : lanesFromConnector(itr))
        {
#ifdef INCLUDE_TRAMS
            // 路面電車レーンはここではチェックしない
            // Tram lanes are not checked here
            if (dynamic_cast<TramLaneInIntersection*>(const_cast<Lane*>(itr_l)))
            {
                continue;
            }
#endif //INCLUDE_TRAMS
            if (itr_l->previousLanes().size() == 0)
            {
                ostringstream ss;
                ss << (isIntersection ? "intersection" : "section") << "["
                   << _id << "]: lane[" << itr_l->id()
                   << "] has no upstream lane.";
                amu::msg::warn(ss.str());
                result = false;
            }
        }
    }

    // 単路部へ流出する前の車線
    // Lanes before outflow to section
    for (auto itr : _borders[0]->outPoints())
    {
        for (auto itr_l : lanesToConnector(itr))
        {
#ifdef INCLUDE_TRAMS
            // 路面電車レーンはここではチェックしない
            // Tram lanes are not checked here
            if (dynamic_cast<TramLaneInIntersection*>(const_cast<Lane*>(itr_l)))
            {
                continue;
            }
#endif //INCLUDE_TRAMS
            if (itr_l->nextLanes().size() == 0)
            {
                ostringstream ss;
                ss << (isIntersection ? "intersection" : "section") << "["
                   << _id << "]: lane[" << itr_l->id()
                   << "] has no downstream lane.";
                amu::msg::warn(ss.str());
                result = false;
            }
        }
    }

    return result;
}

//==============================================================================
void ODNode::deleteAgent()
{
    for (auto itr_l : _lanes)
    {
        // [eMATES] 削除保留エージェントの再登録用
        std::vector<ObjectInLane*> tmpAgents {};
        for (auto itr_a : itr_l.second->agents())
        {
            Vehicle* vehicle = dynamic_cast<Vehicle*>(itr_a);
            if (! vehicle)
            {
                continue;
            }

            // [eMATES] 充電中もしくは充電待ちならEVを消去しない
            VehicleEV* ev = dynamic_cast<VehicleEV*>(vehicle);
            if (ev && (ev->isChargingInCS() || ev->isWaiting()))
            {
                tmpAgents.push_back(itr_a);
                continue;
            }

            if (AppMates::getGVManager().getFlag("FLAG_OUTPUT_TRIP_INFO"))
            {
                // 消去する前にトリップ情報の出力
                // Output trip information before deleting
                VehicleTripWriter writer;
                if (ev)
                {
                    writer.writeVehicleTripEV(ev);
                }
                else
                {
                    writer.writeVehicleTrip(vehicle);
                }
            }
            AppMates::getObjectManager().deleteVehicle(vehicle);
        }
        itr_l.second->clearAgents();
        // [eMATES] 削除保留エージェントの再登録
        for(auto* agent : tmpAgents)
        {
            itr_l.second->agents().push_back(agent);
        }
    }
}

//==============================================================================
void ODNode::pushVehicleToRoadMap(RoadMap* roadMap)
{
    assert(!(_waitingVehicles.empty()));

    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
#ifdef INCLUDE_TRAMS
    // まず路面電車のみ先に処理する
    // Trams are processed first
    deque<Vehicle*> waitingTrams;
    deque<Vehicle*> tmpWaitingVehicles;
    while (!(_waitingVehicles.empty()))
    {
        if (dynamic_cast<VehicleTram*>(_waitingVehicles.front()))
        {
            waitingTrams.push_back(_waitingVehicles.front());
        }
        else
        {
            tmpWaitingVehicles.push_back(_waitingVehicles.front());
        }
        _waitingVehicles.pop_front();
    }
    _waitingVehicles.swap(tmpWaitingVehicles);

    vector<Vehicle*> skippedTrams;
    while (!waitingTrams.empty())
    {
        _odNodeTramExt->pushTramToRoadMap(roadMap, waitingTrams, skippedTrams);
    }

    if (_waitingVehicles.empty())
    {
        return;
    }
#endif //INCLUDE_TRAMS

    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    // 車両を配置可能なレーンを取得する
    // Get lanes where vehicle can be placed
    deque<const Lane*> possibleLanes;
    _extractOutflowLanesWithEnoughSpace(possibleLanes);
    if (possibleLanes.empty())
    {
        return;
    }

    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    // 車両を配置する
    // Place vehicles
    vector<int> order = _rng.getShuffled(possibleLanes.size());
    while (!_waitingVehicles.empty() && !order.empty())
    {
#ifdef INCLUDE_TRAMS
        assert(dynamic_cast<VehicleTram*>(_waitingVehicles.front()) == nullptr);
#endif //INCLUDE_TRAMS

        const Lane* generateLane = possibleLanes[order.back()];
        order.pop_back();
        if (!generateLane)
        {
            continue;
        }

        Vehicle* tmpVehicle = _waitingVehicles.front();
        _waitingVehicles.pop_front();

        _placeVehicleInLane(tmpVehicle, generateLane, roadMap);

        /*
         * 車両属性を出力する
         *   _placeVehicleInLane() から呼び出される ObjectManager::
         *   addVehicleToRoadMap() で識別番号が決まるため，ここで出力する．
         *
         * Output vehicle property
         *   Output it here because the ID number is determined in
         *   ObjectManager::addVehicleToRoadMap() called from
         *   _placeVehicleInLane()
         */
        {
            VehicleTypeWriter writer;
            writer.writeVehicleProperty(tmpVehicle);
        }

        // 流入車両データを記録
        // Record inflow vehicle
        if (AppMates::getGVManager().getFlag("FLAG_OUTPUT_INFLOW_MONITOR")
            && _inflowMonitor)
        {
            ulint headway = AppMates::getTimeManager().time() - _lastIncomeTime;
            ulint genTime = tmpVehicle->location()->generationTime();
            ulint genInterval = genTime - _lastGenerationTime;

            _inflowMonitor->recordInflowVehicle(
                generateLane, tmpVehicle, headway, genTime, genInterval);
        }

        _lastIncomeTime     = AppMates::getTimeManager().time();
        _lastGenerationTime = tmpVehicle->location()->generationTime();
    }

#ifdef INCLUDE_TRAMS
    // 発生できなかった路面電車を_waitingVehiclesに戻す
    // Put trams that could not be generated to _waitingVehicles
    _waitingVehicles.insert(
        _waitingVehicles.begin(), skippedTrams.begin(), skippedTrams.end());
#endif //INCLUDE_TRAMS
}

//==============================================================================
void ODNode::_extractOutflowLanesWithEnoughSpace(
    std::deque<const Lane*>& result_lanes)
{
    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    /*
     * 他車両の車線変更先となっているレーンを取得する．
     *   発生点付近で車線変更が行われているレーンには車両を配置できない．
     *
     * Get the lanes where other vehicles are changing lanes.
     *   A vehicle cannot be placed in the lanes where other vehicles are
     *   changing lanes near the origin point.
     */
    vector<Lane*>               shiftTargetLanes;
    const list<const Vehicle*>& notifyVehicles
        = _incSections[0]->watchedVehicles();

    for (auto itr : notifyVehicles)
    {
        const VehicleBehavior* behavior = itr->behavior();
        if (itr->decision()->isLaneChangeActive()
            && behavior->distanceTo() < 25)
        {
            shiftTargetLanes.push_back(const_cast<Lane*>(behavior->laneTo()));
        }
    }

    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    for (auto itr : _incSections[0]->lanesFrom(this))
    {
        // 車線変更先となっているレーンを除外
        // Exclude lanes that are a target of lane-changing
        if (find(shiftTargetLanes.begin(), shiftTargetLanes.end(), itr)
            != shiftTargetLanes.end())
        {
            continue;
        }

#ifdef INCLUDE_TRAMS
        // 路面電車レーンを除外
        // Exclude tram lane
        if (dynamic_cast<TramLaneInSection*>(const_cast<Lane*>(itr)))
        {
            continue;
        }
#endif //INCLUDE_TRAMS

        // 最後尾の車両の位置により判定する
        // Judge by the position of the tail vehicle
#ifdef GENERATE_VEHICLE_VELOCITY_ZERO
        if (itr->tailAgent() == nullptr)
        {
            result_lanes.emplace_back(itr);
        }
        else if (
            itr->tailAgent()->distance() - itr->tailAgent()->bodyLength() / 2
                - _waitingVehicles.front()->bodyLength()
            > 1.0)
        {
            result_lanes.emplace_back(itr);
        }
#else  // GENERATE_VEHICLE_VELOCITY_ZERO not defined
        const ObjectInLane* tail = nullptr;
        double              distance;
        itr->getFrontAgentFar(0.0, 100, &tail, &distance);
        if (!tail)
        {
            result_lanes.emplace_back(itr);
        }
        else if (
            distance - tail->bodyLength()
            > _waitingVehicles.front()->determiner()->calcSafetyDistance(
                itr->speedLimit() / 3.6, 0, 2.0))
        {
            result_lanes.emplace_back(itr);
        }
#endif // GENERATE_VEHICLE_VELOCITY_ZERO
    }
}

//==============================================================================
void ODNode::_placeVehicleInLane(
    Vehicle* vehicle, const Lane* lane, RoadMap* roadMap)
{
    bool result = AppMates::getObjectManager().addVehicleToRoadMap(vehicle);
    assert(result);

    Section* section = _incSections[0];

    // RoadMap を登録
    // Register RoadMap
    vehicle->setRoadMap(roadMap);

    // 道路上に登場
    // Appear on the road
    vehicle->addToSection(
        roadMap, section, const_cast<Lane*>(lane), vehicle->bodyLength() / 2);

    // グローバル経路を探索する
    // Search global route
    vehicle->reroute(this, section->anotherIntersection(this));
    const_cast<VehicleGlobalRoute*>(vehicle->globalRoute())
        ->setLastPassedIntersectionIndex(this);

    // 経路探索結果にもとづいてローカル経路を探索する
    // Search local route based on the result of global routing
    vehicle->firstLocalReroute(
        section, const_cast<Lane*>(lane), vehicle->bodyLength() / 2);

    // 流入時刻を保存する
    // Save inflow time
    const_cast<VehicleLocation*>(vehicle->location())
        ->setStartingTime(AppMates::getTimeManager().time());
}
