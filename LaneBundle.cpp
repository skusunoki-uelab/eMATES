/* **************************************************************************
 * Copyright 2014-2024 ADVENTURE Project
 * All Rights Reserved
 * ************************************************************************** */
/**
 * @file LaneBundle.cpp
 */
#include "LaneBundle.hpp"
#include "Connector.hpp"
#include "CustomMessage.hpp"
#include "Lane.hpp"
#include "RoadMap.hpp"
#include "ObjectInLane.hpp"
#include "Vehicle.hpp"
#include "io/LaneBundleBuilder.hpp"
#ifdef INCLUDE_TRAMS
#include "tram/TramLaneInIntersection.hpp"
#include "tram/TramLaneInSection.hpp"
#endif //INCLUDE_TRAMS
#include <AmuLineSegment.hpp>
#include <AmuPoint.hpp>
#include <algorithm>
#include <cstdlib>
#include <functional>
#include <sstream>
#include <string>
#include <typeinfo>

using namespace std;
using namespace amu::geometry;

//==============================================================================
LaneBundle::LaneBundle(const string& id, RoadMap* parent)
{
    _id     = id;
    _parent = parent;
    _isUsed = false;
    _watchedVehicles.clear();
#ifdef _OPENMP
    omp_init_lock(&_lock);
#endif //_OPENMP
}

//==============================================================================
LaneBundle::~LaneBundle()
{
    for (auto itr : _subsecs)
    {
        delete itr.second;
    }
    _subsecs.clear();

    for (auto itr : _lanes)
    {
        delete itr.second;
    }
    _lanes.clear();

#ifdef _OPENMP
    omp_destroy_lock(&_lock);
#endif //_OPENMP
}

//==============================================================================
double LaneBundle::distanceToNext(const Lane* lane, double distance) const
{
    ASSERT_MSG(containsLane(lane));
    const Lane* target = lane;
    double      result = target->length() - distance;
    while (containsNextLane(target))
    {
        target = target->nextStraightLane();
        result += target->length();
    }
    return result;
}

//==============================================================================
double LaneBundle::distanceFromPrevious(const Lane* lane, double distance) const
{
    ASSERT_MSG(containsLane(lane));
    const Lane* target = lane;
    double      result = distance;
    while (containsPreviousLane(target))
    {
        target = target->previousStraightLane();
        result += target->length();
    }
    return result;
}

//==============================================================================
const Connector* LaneBundle::internalConnector(const string& id) const
{
    auto itr = _internalConnectors.find(id);
    if (itr != _internalConnectors.end())
    {
        return (*itr).second;
    }
    else
    {
        ostringstream sse;
        sse << "No Internal Connector[" << id << "] in LaneBundle[" << _id
            << "]" << endl;
        amu::msg::error(sse.str());
        exit(EXIT_FAILURE);
    }
}

//==============================================================================
bool LaneBundle::containsSubLaneBundle(const SubLaneBundle* subsec) const
{
    for (auto itr : _subsecs)
    {
        if (itr.second == subsec)
        {
            return true;
        }
    }
    return false;
}

//==============================================================================
vector<const Lane*> LaneBundle::lanesFromConnector(
    const Connector* connector) const
{
    ASSERT_MSG(connector);
    vector<const Lane*> result_lanes;
    for (auto itr : _lanes)
    {
        if (itr.second->beginConnector() == connector)
        {
            result_lanes.emplace_back(itr.second);
        }
    }
    return result_lanes;
}

//==============================================================================
vector<const Lane*> LaneBundle::lanesToConnector(
    const Connector* connector) const
{
    ASSERT_MSG(connector);
    vector<const Lane*> result_lanes;
    for (auto itr : _lanes)
    {
        if (itr.second->endConnector() == connector)
        {
            result_lanes.emplace_back(itr.second);
        }
    }
    return result_lanes;
}

//==============================================================================
bool LaneBundle::checkLaneConnectivity(bool isIntersection) const
{
    bool result = true;

    for (auto itr : _lanes)
    {
#ifdef INCLUDE_TRAMS
        // 路面電車レーンはここではチェックしない
        // Tram lanes are not checked here
        if ((isIntersection
             && dynamic_cast<TramLaneInIntersection*>(itr.second))
            || (!isIntersection
                && dynamic_cast<TramLaneInSection*>(itr.second)))
        {
            continue;
        }
#endif //INCLUDE_TRAMS

        if (itr.second->previousLanes().size() == 0)
        {
            ostringstream ssw;
            ssw << (isIntersection ? "Intersection" : "Section") << "[" << _id
                << "]: Lane[" << itr.second->id() << "] has no upstream lane.";
            amu::msg::warn(ssw.str());
            result = false;
        }
        if (itr.second->nextLanes().size() == 0)
        {
            ostringstream ss;
            ss << (isIntersection ? "Intersection" : "Section") << "[" << _id
               << "]: Lane[" << itr.second->id() << "] has no downstream lane.";
            amu::msg::warn(ss.str());
            result = false;
        }
    }

    return result;
}

//==============================================================================
void LaneBundle::renewAgentOrder()
{
    _usedLanes.clear();
    _isUsed = false;
    for (auto itr : _lanes)
    {
        itr.second->renewAgentOrder();
        if (itr.second->agents().size() > 0)
        {
            /*
             * エージェントの存在するレーンはこのあとの処理対象にする
             *
             * The lane containing agents will be subject to subsequent
             * processing.
             */
            _usedLanes.push_back(itr.second);
            if (!_isUsed)
            {
                _isUsed = true;
            }
        }
    }
}

//==============================================================================
bool LaneBundle::isHeadAgent(const ObjectInLane* agent, const Lane* lane) const
{
    ASSERT_MSG(containsLane(lane));

    if (lane->headAgent() != agent)
    {
        return false;
    }

    // 指定したエージェントががレーンの中で先頭である
    // The specified agent is the head in the lane
    if (containsNextLane(lane) == false)
    {
        // 指定したレーンがレーン束オブジェクトの先頭である
        // The specified lane is the head of the lane bundle object
        return true;
    }
    else
    {
        /*
         * 指定したレーンより下流レーンがレーン束オブジェクトに
         * 存在し，下流レーンにエージェントが存在しない
         *
         * A lane downstream of the specified lane exists in the
         * lane bundle object, and no agent exists in the downstream
         * lanes.
         */
        const Lane* nextLane = lane;
        while (containsNextLane(nextLane))
        {
            nextLane = lane->nextStraightLane();
            if (nextLane->headAgent())
            {
                return false;
            }
        }
        return true;
    }
}

//==============================================================================
void LaneBundle::addWatchedVehicle(const Vehicle* vehicle)
{
    ASSERT_MSG(vehicle);

    // 重複登録を認めない
    // Not allow duplicate registration
    if (_watchedVehicles.empty())
    {
#ifdef _OPENMP
        omp_set_lock(&_lock);
#endif //_OPENMP

        _watchedVehicles.emplace_back(vehicle);

#ifdef _OPENMP
        omp_unset_lock(&_lock);
#endif //_OPENMP
    }
    else
    {
        if (find(_watchedVehicles.begin(), _watchedVehicles.end(), vehicle)
            == _watchedVehicles.end())
        {
#ifdef _OPENMP
            omp_set_lock(&_lock);
#endif //_OPENMP

            _watchedVehicles.emplace_back(vehicle);

#ifdef _OPENMP
            omp_unset_lock(&_lock);
#endif //_OPENMP
        }
        else
        {
            // vehicle->print(cout);
        }
    }
}

//==============================================================================
void LaneBundle::eraseWatchedVehicle(const Vehicle* vehicle)
{
    ASSERT_MSG(vehicle);

#ifdef _OPENMP
    omp_set_lock(&_lock);
#endif //_OPENMP

    _watchedVehicles.remove(vehicle);

#ifdef _OPENMP
    omp_unset_lock(&_lock);
#endif //_OPENMP
}
