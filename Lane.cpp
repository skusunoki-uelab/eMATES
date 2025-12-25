/* **************************************************************************
 * Copyright 2014-2024 ADVENTURE Project
 * All Rights Reserved
 * ************************************************************************** */
/**
 * @file Lane.cpp
 */
#include "Lane.hpp"
#include "AppMates.hpp"
#include "Config.hpp"
#include "CSNodeFast.hpp" // [eMATES]
#include "CSNodeNormal.hpp" // [eMATES]
#include "CustomMessage.hpp"
#include "GVManager.hpp"
#include "LaneBundle.hpp"
#include "ObjectInLane.hpp"
#include "SubLaneBundle.hpp"
#include "TrafficCounterComponent.hpp"
#include "Vehicle.hpp"
#include <AmuVector.hpp>
#include <algorithm>
#include <cfloat>
#include <cmath>
#include <iostream>
#include <sstream>
#include <typeinfo>

using namespace std;
using namespace amu::geometry;
using namespace amu::math;

//==============================================================================
Lane::Lane(
    const string& id, const Connector* ptBegin, const Connector* ptEnd,
    AmuLineSegment* ptLineSegment, LaneBundle* parent)
    : _id(id),
      _parent(parent),
      _beginConnector(ptBegin),
      _endConnector(ptEnd),
      _lineSegment(ptLineSegment)
{
    _lastArrivalTime = 0;

    _tmpAgents.clear();
    _tmpAgentsToAdd.clear();
    _agents.clear();
    _counters.clear();

#ifdef _OPENMP
    omp_init_lock(&_lock);
#endif //_OPENMP

#ifdef INCLUDE_PEDESTRIANS
    _pedExt = new LanePedExt(this);
#endif //INCLUDE_PEDESTRIANS
}

//==============================================================================
Lane::~Lane()
{
    if (_lineSegment)
    {
        delete _lineSegment;
    }

#ifdef _OPENMP
    omp_destroy_lock(&_lock);
#endif //_OPENMP

#ifdef INCLUDE_PEDESTRIANS
    delete _pedExt;
#endif //INCLUDE_PEDESTRIANS
}

//==============================================================================
void Lane::renewAgentOrder()
{
    // _tmpAgentsToAdd を _tmpAgents に統合
    // Merge _tmpAgentsToAdd to _tmpAgents
    if (_tmpAgentsToAdd.size() > 0)
    {
        _tmpAgents.insert(
            _tmpAgents.end(), _tmpAgentsToAdd.begin(), _tmpAgentsToAdd.end());
    }

    // 2つのvectorを入れ替えて _agents をソートする
    // Swap 2 vectors and sort _agents
    _agents.swap(_tmpAgents);
    sort(
        _agents.begin(), _agents.end(),
        [](const ObjectInLane* rl, const ObjectInLane* rr)
        {
            return (rl->distance() < rr->distance());
        });

    // 一時的コンテナをクリアする
    // Clear temporary containers
    _tmpAgents.clear();
    _tmpAgentsToAdd.clear();
}

//==============================================================================
void Lane::makeAgentsPreperceive()
{
    for (auto itr : _agents)
    {
#ifdef INCLUDE_VEHICLES
        Vehicle* vehicle = dynamic_cast<Vehicle*>(itr);
        if (vehicle)
        {
            vehicle->preperceive();
        }
#endif //INCLUDE_VEHICLES
    }
}

//==============================================================================
void Lane::makeAgentsPerceive()
{
    for (auto itr : _agents)
    {
#ifdef INCLUDE_VEHICLES
        Vehicle* vehicle = dynamic_cast<Vehicle*>(itr);
        if (vehicle)
        {
            vehicle->perceive();
        }
#endif //INCLUDE_VEHICLES
    }
}

//==============================================================================
void Lane::makeAgentsDetermine()
{
    for (auto itr : _agents)
    {
#ifdef INCLUDE_VEHICLES
        Vehicle* vehicle = dynamic_cast<Vehicle*>(itr);
        if (vehicle)
        {
            vehicle->determine();
        }
#endif //INCLUDE_VEHICLES
    }
}

//==============================================================================
void Lane::makeAgentsAct()
{
    for (auto itr : _agents)
    {
#ifdef INCLUDE_VEHICLES
        Vehicle* vehicle = dynamic_cast<Vehicle*>(itr);
        if (vehicle)
        {
            vehicle->act();
        }
#endif //INCLUDE_VEHICLES
    }
}

//==============================================================================
void Lane::makeAgentsPostact()
{
    for (auto itr : _agents)
    {
#ifdef INCLUDE_VEHICLES
        Vehicle* vehicle = dynamic_cast<Vehicle*>(itr);
        if (vehicle)
        {
            vehicle->postact();
        }
#endif //INCLUDE_VEHICLES
    }
}

//==============================================================================
const ObjectInLane* Lane::headAgent() const
{
    // _agents は 始端に近い順に並んでいる
    // _agents are listed in ordered by closest to the start
    if (!_agents.empty())
    {
        return _agents.back();
    }
    return nullptr;
}

//==============================================================================
const Vehicle* Lane::headVehicle() const
{
    const Vehicle* head = nullptr;
    if (!_agents.empty())
    {
        vector<ObjectInLane*>::const_reverse_iterator itr;
        for (itr = _agents.rbegin(); itr != _agents.rend(); itr++)
        {
            head = dynamic_cast<Vehicle*>(*itr);
            if (head)
            {
                return head;
            }
        }
    }
    return head;
}

//==============================================================================
const ObjectInLane* Lane::tailAgent() const
{
    // _agents は 始端に近い順に並んでいる
    // _agents are listed in ordered by closest to the start
    if (!_agents.empty())
    {
        return _agents.front();
    }
    return NULL;
    ;
}

//==============================================================================
const Vehicle* Lane::tailVehicle() const
{
    Vehicle* tail = nullptr;
    if (!_agents.empty())
    {
        vector<ObjectInLane*>::const_iterator itr;
        for (itr = _agents.begin(); itr != _agents.end(); itr++)
        {
            tail = dynamic_cast<Vehicle*>(*itr);
            if (tail)
            {
                break;
            }
        }
    }
    return tail;
}

//==============================================================================
const ObjectInLane* Lane::frontAgent(const ObjectInLane* agent) const
{
    auto itr = find(_agents.begin(), _agents.end(), agent);
    if (itr != _agents.end())
    {
        itr++;
        if (itr != _agents.end())
        {
            return *itr;
        }
    }
    return nullptr;
}

//==============================================================================
const Vehicle* Lane::frontVehicle(const ObjectInLane* agent) const
{
    const ObjectInLane* front = frontAgent(agent);
    while (front && typeid(*front) != typeid(Vehicle))
    {
        front = frontAgent(front);
    }
    return dynamic_cast<Vehicle*>(const_cast<ObjectInLane*>(front));
}

//==============================================================================
void Lane::getFrontAgentFar(
    const ObjectInLane* agent, double threshold,
    const ObjectInLane** result_agent, double* result_distance) const
{
    getFrontAgentFar(
        agent->distance(), threshold, result_agent, result_distance);
}

//==============================================================================
const ObjectInLane* Lane::frontAgent(double distance) const
{
    if (distance > _lineSegment->length())
    {
        return nullptr;
    }
    vector<ObjectInLane*>::const_iterator itr;
    for (itr = _agents.begin(); itr != _agents.end(); itr++)
    {
        if ((*itr)->distance() >= distance)
        {
            return *itr;
        }
    }
    return nullptr;
}

//==============================================================================
const Vehicle* Lane::frontVehicle(double distance) const
{
    const ObjectInLane* front = frontAgent(distance);
    while (front && typeid(*front) != typeid(Vehicle))
    {
        front = frontAgent(front);
    }
    return dynamic_cast<Vehicle*>(const_cast<ObjectInLane*>(front));
}

//==============================================================================
void Lane::getFrontAgentFar(
    double startDistance, double threshold, const ObjectInLane** result_agent,
    double* result_distance) const
{
    const ObjectInLane* front = frontAgent(startDistance);

    // 同じレーン内に先行エージェントが見つかった場合
    // If a preceding agent is found in the same lane
    if (front)
    {
        *result_agent    = front;
        *result_distance = front->distance() - startDistance;
        return;
    }

    // 下流まで検索する場合
    // When searching downstream
    double totalDistance = this->length() - startDistance;
    double distance      = DBL_MAX;
    if (totalDistance < threshold)
    {
        const Lane* lookupLane = this;
        while (totalDistance < threshold)
        {
            // 下流レーンを取得
            // Get downstream lane
            lookupLane = lookupLane->nextStraightLane();
            if (!lookupLane)
            {
                break;
            }

            // ODノードに到達したら探索を止める
            // Stop searching when reached ODNode
            auto& parent = *(lookupLane->parent());
            if (typeid(parent) == typeid(ODNode)
                || typeid(parent) == typeid(CSNodeFast) // [eMATES]
                || typeid(parent) == typeid(CSNodeNormal)) // [eMATES]
            {
                break;
            }

            // レーンの末尾のエージェントを取得
            // Get the agent at the tail of the lane
            front = lookupLane->tailAgent();
            if (front)
            {
                distance = totalDistance + front->distance();
                break;
            }
            else
            {
                totalDistance += lookupLane->length();
            }
        }
    }
    *result_agent    = front;
    *result_distance = distance;
    return;
}

//==============================================================================
const ObjectInLane* Lane::followingAgent(const ObjectInLane* agent) const
{
    auto itr = find(_agents.begin(), _agents.end(), agent);
    if (itr != _agents.begin() && itr != _agents.end())
    {
        itr--;
        return *itr;
    }
    return nullptr;
}

//==============================================================================
const Vehicle* Lane::followingVehicle(const ObjectInLane* agent) const
{
    const ObjectInLane* follower = followingAgent(agent);
    while (follower && typeid(*follower) != typeid(Vehicle))
    {
        follower = followingAgent(follower);
    }
    return dynamic_cast<Vehicle*>(const_cast<ObjectInLane*>(follower));
}

//==============================================================================
void Lane::getFollowingAgentFar(
    const ObjectInLane* agent, double threshold,
    const ObjectInLane** result_agent, double* result_distance) const
{
    getFollowingAgentFar(
        agent->distance(), threshold, result_agent, result_distance);
}

//==============================================================================
const ObjectInLane* Lane::followingAgent(double distance) const
{
    vector<ObjectInLane*>::const_reverse_iterator itr;
    for (itr = _agents.rbegin(); itr != _agents.rend(); itr++)
    {
        if ((*itr)->distance() < distance)
        {
            return *itr;
        }
    }
    return nullptr;
}

//==============================================================================
const Vehicle* Lane::followingVehicle(double distance) const
{
    const ObjectInLane* follower = followingAgent(distance);
    while (follower && typeid(*follower) != typeid(Vehicle))
    {
        follower = followingAgent(follower);
    }
    return dynamic_cast<Vehicle*>(const_cast<ObjectInLane*>(follower));
}

//==============================================================================
void Lane::getFollowingAgentFar(
    double startDistance, double threshold, const ObjectInLane** result_agent,
    double* result_distance) const
{
    const ObjectInLane* follower = followingAgent(startDistance);

    // 同じレーン内に後続エージェントが見つかった場合
    // If a following agent is found in the same lane
    if (follower)
    {
        *result_agent    = follower;
        *result_distance = startDistance - follower->distance();
        return;
    }

    // 上流まで検索する場合
    // When searching upstream
    double totalDistance = startDistance;
    double distance      = DBL_MAX;
    if (totalDistance < threshold)
    {
        const Lane* lookupLane = this;
        while (totalDistance < threshold)
        {
            // 上流のレーンを取得
            // Get upstream lane
            lookupLane = lookupLane->previousStraightLane();
            if (!lookupLane)
            {
                break;
            }

            // ODノードに到達したら探索を止める
            // Stop searching when reached ODNodes
            auto& parent = *(lookupLane->parent());
            if (typeid(parent) == typeid(ODNode)
                || typeid(parent) == typeid(CSNodeFast) // [eMATES]
                || typeid(parent) == typeid(CSNodeNormal)) // [eMATES]
            {
                break;
            }

            // レーンの先頭のエージェントを取得
            // Get the agent at the head of the lane
            follower = lookupLane->headAgent();
            if (follower)
            {
                distance = totalDistance + lookupLane->length()
                    - follower->distance();
                break;
            }
            else
            {
                totalDistance += lookupLane->length();
            }
        }
    }
    *result_agent    = follower;
    *result_distance = distance;
    return;
}

//==============================================================================
bool Lane::putAgent(ObjectInLane* agent)
{
    // duplication check
    if (find(_tmpAgents.begin(), _tmpAgents.end(), agent) != _tmpAgents.end())
    {
        ostringstream sse;
        sse << "Agent[" << agent->id() << "] is already put in Lane[" << _id
            << "(@" << _parent->id() << ")]" << endl;
        amu::msg::error(sse.str());
        exit(EXIT_FAILURE);
    }
    _tmpAgents.push_back(agent);
    return true;
}

//==============================================================================
bool Lane::registerAgentToAdd(ObjectInLane* agent)
{
#ifdef _OPENMP
    omp_set_lock(&_lock);
#endif //_OPENMP

    // duplication check
    if (find(_tmpAgentsToAdd.begin(), _tmpAgentsToAdd.end(), agent)
        != _tmpAgentsToAdd.end())
    {
        ostringstream sse;
        sse << "Agent[" << agent->id()
            << "] is already registered to add in Lane[" << _id << "(@"
            << _parent->id() << ")]" << endl;
        amu::msg::error(sse.str());
        exit(EXIT_FAILURE);
    }
    _tmpAgentsToAdd.push_back(agent);

#ifdef _OPENMP
    omp_unset_lock(&_lock);
#endif //_OPENMP

    return true;
}

//==============================================================================
bool Lane::canAcceptLaneShift() const
{
    if (length()
        < AppMates::getGVManager().getNumeric("THRESHOLD_VEHICLE_LANESHIFT"))
    {
        return false;
    }

    // 末尾の停止エージェントの位置による判定
    // Judgment by the position of the agent stopped at the tail
    for (auto itr : _agents)
    {
        if (itr->distance() >= AppMates::getGVManager().getNumeric(
                "THRESHOLD_VEHICLE_LANESHIFT"))
        {
            continue;
        }
        if (itr->accel() < 0.0
            || itr->velocity()
                < AppMates::getGVManager().getNumeric("VELOCITY_CREEP")
                    / 3600.0)
        {
            return false;
        }
    }
    return true;
}

//==============================================================================
double Lane::averageVel() const
{
    double       vel         = 0.0;
    unsigned int numVehicles = 0;

    for (auto itr : _tmpAgents)
    {
        if (dynamic_cast<Vehicle*>(itr) != NULL)
        {
            numVehicles++;
            vel += itr->velocity();
        }
    }

    // 車両がいない場合には制限速度を平均速度とする
    // If no vehicles, the speed limit is regarded as the average speed
    if (numVehicles == 0)
    {
        vel = _speedLimit / 60.0 / 60.0;
    }
    else
    {
        vel /= numVehicles;
    }
    return vel;
}

//==============================================================================
void Lane::addTrafficCounter(double distance, TrafficCounterComponent* counter)
{
    // 指定された distance をキーに持つ要素を検索
    // Find elements with specified distance as key
    if (_counters.find(distance) == _counters.end())
    {
#ifdef _OPENMP
        omp_set_lock(&_lock);
#endif //_OPENMP

        // 見つからなかった場合は新規に登録
        // If not found, register it as a new element
        _counters.insert(make_pair(distance, counter));

#ifdef _OPENMP
        omp_unset_lock(&_lock);
#endif //_OPENMP
    }
    else
    {
        ostringstream ssw;
        ssw << "Since there is already a traffic counter "
            << "installed at distance " << distance << " on Lane[" << _id
            << "(@" << _parent->id() << ")], " << "so TrafficCounter["
            << counter->parent()->id() << "] will not be installed." << endl;
        amu::msg::warn(ssw.str());
    }
}

//==============================================================================
void Lane::counters(
    vector<TrafficCounterComponent*>* result_counters, double oldDistance,
    double distance) const
{
    for (auto itr : _counters)
    {
        if (itr.first > oldDistance && itr.first <= distance)
        {
            result_counters->emplace_back(itr.second);
        }
    }
}

//==============================================================================
void Lane::print(ostream& out) const
{
    ostringstream oss;

    oss << "--- Lane Information ---" << endl;
    oss << "ID: " << _id << ", Parent ID: " << _parent->id() << endl;
    oss << "Length: " << this->length() << endl;

    if (!_nextLanes.empty())
    {
        oss << "NextLanes:" << endl;
        for (auto itr : _nextLanes)
        {
            oss << "  ID: " << itr->id() << "(@" << itr->parent()->id() << ")"
                << endl;
        }
        oss << "  NextStraightLane: " << _nextStraightLane->id() << endl;
    }
    else
    {
        oss << "NextLanes: none" << endl;
    }

    if (!_previousLanes.empty())
    {
        oss << "PreviousLanes:" << endl;
        for (auto itr : _previousLanes)
        {
            oss << "  ID: " << itr->id() << "(@" << itr->parent()->id() << ")"
                << endl;
        }
        oss << "  PreviousStraightLane: " << _previousStraightLane->id()
            << endl;
    }
    else
    {
        oss << "PreviousLanes: none" << endl;
    }

    amu::msg::message(out, oss.str());
}

