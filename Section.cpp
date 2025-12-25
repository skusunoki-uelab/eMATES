/* **************************************************************************
 * Copyright 2014-2024 ADVENTURE Project
 * All Rights Reserved
 * ************************************************************************** */
/**
 * @file Section.cpp
 */
#include "Section.hpp"
#include "AppMates.hpp"
#include "Config.hpp"
#include "GVManager.hpp"
#include "Intersection.hpp"
#include "Lane.hpp"
#include "RoadMap.hpp"
#include "RoutingNode.hpp"
#include "ObjectInLane.hpp"
#include "Vehicle.hpp"
#include "VehicleLaneChangePerceiver.hpp"
#include "io/SectionBuilder.hpp"
#include <AmuLineSegment.hpp>
#include <algorithm>
#include <cassert>
#include <sstream>
#ifdef INCLUDE_TRAMS
#include "tram/SectionTramExt.hpp"
#endif //INCLUDE_TRAMS

using namespace std;
using namespace amu::geometry;

//==============================================================================
Section::Section(
    const string& id, Intersection* first, Intersection* second,
    RoadMap* parent)
    : LaneBundle(id, parent)
{
    _incInters[0] = first;
    _incInters[1] = second;

    int dirFirst  = first->direction(second);
    int dirSecond = second->direction(first);
    _numIn[0]     = first->numOut(dirFirst);
    _numOut[0]    = first->numIn(dirFirst);
    _numIn[1]     = second->numOut(dirSecond);
    _numOut[1]    = second->numIn(dirSecond);

    for (int i = 0; i < 2; i++)
    {
        _speedLimit[i]
            = AppMates::getGVManager().getNumeric("SPEED_LIMIT_SECTION");
        _routingProbability[i].clear();
    }

    _linkFlowMonitors[0] = nullptr;
    _linkFlowMonitors[1] = nullptr;

#ifdef INCLUDE_TRAMS
    _tramExt = new SectionTramExt(this);
#endif //INCLUDE_TRAMS
}

//==============================================================================
Section::~Section()
{
    // _subsecs と _lanes は ~LaneBundle() で delete される
    // _subsecs and _lanes are deleted with ~LaneBundle()

    _incInters[0] = nullptr;
    _incInters[1] = nullptr;

    for (auto itr : _speedLimitList)
    {
        delete itr;
    }
    _speedLimitList.clear();

#ifdef INCLUDE_TRAMS
    delete _tramExt;
#endif //INCLUDE_TRAMS
}

//==============================================================================
double Section::sidewalkWidth(Intersection* inter, bool leftSide) const
{
    assert(inter == _incInters[0] || inter == _incInters[1]);
    if ((inter == _incInters[0] && leftSide == true)
        || (inter == _incInters[1] && leftSide == false))
    {
        /*
         * _incInters[0] から見て左側 ＝ _incInters[1] から見て右側
         *
         * Left side as seen from _incInters[0] = Right side as seen
         * from _incInters[1]
         */
        return _sidewalkWidth[1];
    }
    else
    {
        /*
         * _incInters[0] から見て右 ＝ _incInters[1] から見て左
         *
         * Right side as seen from _incInters[0] = Left side as seen
         * from _incInters[1]
         */
        return _sidewalkWidth[0];
    }
}

//==============================================================================
LaneBundle* Section::nextBundle(const Lane* lane) const
{
    return nextIntersection(lane);
}

//==============================================================================
LaneBundle* Section::previousBundle(const Lane* lane) const
{
    return previousIntersection(lane);
}

//==============================================================================
Intersection* Section::sharedIntersection(const Section* another) const
{
    for (unsigned int i = 0; i <= 1; i++)
    {
        for (int j = 0; j < _incInters[i]->numNexts(); j++)
        {
            if (_incInters[i]->nextSection(j) == another)
            {
                return _incInters[i];
            }
        }
    }
    cerr << "ERROR: no intersection shared between section: " << _id
         << " and section: " << another->id() << endl;
    return NULL;
}

//==============================================================================
bool Section::hasValidPath(const Lane* lane, int dir) const
{
    bool                result = false;
    vector<const Lane*> lanes  = nextLanes(lane);
    if (containsNextLane(lane))
    {
        vector<const Lane*> lanes = nextLanes(lane);

        // 次のレーンも同じ単路部内なら引き続き検索する
        // If the next lanes are on the same section, continue searching
        for (unsigned int i = 0; i < lanes.size() && !result; i++)
        {
            result = hasValidPath(lanes[i], dir);
        }
    }
    else
    {
        /*
         * 次のレーンが交差点内なら交差点の isReachable() を呼ぶ
         *
         * If the next lanes are in an intersection, call isReachable()
         * of the intersection.
         */
        Intersection* inter = intersection(isUp(lane));
        for (unsigned int i = 0; i < lanes.size() && !result; i++)
        {
            result = inter->hasValidPath(lanes[i], dir);
        }
    }
    return result;
}


//==============================================================================
bool Section::isUp(const Lane* lane) const
{
    int idBorder = atoi(lane->id().c_str()) / 1000000;

    // IDが"00"から始まる場合
    // If the ID starts from "00"
    if (idBorder == 0)
    {
        return true;
    }

    // IDが"01"から始まる場合
    // If the ID starts from "01"
    else if (idBorder == 1)
    {
        return false;
    }

    // それ以外は前のレーンで判定する
    // Otherwise, judge based on the previous lane
    else
    {
        assert(lanesToConnector(lane->beginConnector()).size() != 0);
        return isUp(lanesToConnector(lane->beginConnector())[0]);
    }
}

//==============================================================================
bool Section::isUp(const Intersection* from, const Intersection* to) const
{
    if (from == _incInters[0] && to == _incInters[1])
    {
        return true;
    }
    else if (from == _incInters[1] && to == _incInters[0])
    {
        return false;
    }
    else
    {
        cerr << "section:" << _id << " is not connected from "
             << "intersection:" << from->id() << " to intersection:" << to->id()
             << std::endl;
        exit(EXIT_FAILURE);
    }
}

//==============================================================================
vector<const Lane*> Section::nextLanes(const Lane* lane) const
{
    vector<const Lane*> result_lanes = lanesFromConnector(lane->endConnector());
    if (result_lanes.empty())
    {
        // 次のレーンがこの単路部にない場合
        // If the next lanes are not on this section
        Intersection* nextInter = intersection(isUp(lane));
        result_lanes = nextInter->lanesFromConnector(lane->endConnector());
    }
    if (result_lanes.empty())
    {
        cerr << "no next lane error at " << _id << endl;
        exit(1);
    }
    return result_lanes;
}

//==============================================================================
vector<const Lane*> Section::previousLanes(const Lane* lane) const
{
    vector<const Lane*> result_lanes = lanesToConnector(lane->beginConnector());
    if (result_lanes.empty())
    {
        // 前のレーンがこの単路部にない場合
        // If the previous lanes are not on this section
        Intersection* prevInter = intersection(!isUp(lane));
        result_lanes = prevInter->lanesToConnector(lane->beginConnector());
    }
    if (result_lanes.empty())
    {
        cerr << "no previous lane error at " << _id << endl;
        exit(1);
    }
    return result_lanes;
}

//==============================================================================
vector<const Lane*> Section::lanesWithDirection(bool dir) const
{
    vector<const Lane*> result_lanes;
    for (auto itr : _lanes)
    {
        if (isUp(itr.second) == dir)
        {
            result_lanes.emplace_back(itr.second);
        }
    }
    return result_lanes;
}

//==============================================================================
vector<const Lane*> Section::lanesFrom(const Intersection* inter) const
{
    assert(inter == _incInters[0] || inter == _incInters[1]);
    vector<const Lane*>      result_lanes;
    vector<const Connector*> connectors
        = inter->border(inter->direction(this))->outPoints();

    for (int i = 0; i < static_cast<signed int>(connectors.size()); i++)
    {
        vector<const Lane*> lanes = lanesFromConnector(connectors[i]);
        for (int j = 0; j < static_cast<signed int>(lanes.size()); j++)
        {
            result_lanes.push_back(lanes[j]);
        }
    }

#ifdef INCLUDE_TRAMS
    if (_tramExt)
    {
        _tramExt->getTramLanesFrom(result_lanes, inter);
    }
#endif //INCLUDE_TRAMS

    return result_lanes;
}

//==============================================================================
vector<const Lane*> Section::lanesTo(const Intersection* inter) const
{
    assert(inter == _incInters[0] || inter == _incInters[1]);
    vector<const Lane*>      result_lanes;
    vector<const Connector*> connectors
        = inter->border(inter->direction(this))->inPoints();

    for (unsigned int i = 0; i < connectors.size(); i++)
    {
        vector<const Lane*> lanes = lanesToConnector(connectors[i]);
        for (unsigned int j = 0; j < lanes.size(); j++)
        {
            result_lanes.push_back(lanes[j]);
        }
    }
#ifdef INCLUDE_TRAMS
    if (_tramExt)
    {
        _tramExt->getTramLanesTo(result_lanes, inter);
    }
#endif //INCLUDE_TRAMS

    return result_lanes;
}

//==============================================================================
SubLaneBundle* Section::pairedSubLaneBundle(
    SubLaneBundle* subsec, int edgeNum) const
{
    assert(0 <= edgeNum && edgeNum < subsec->numVertexes());
    AmuLineSegment commonEdge = subsec->edge(edgeNum);

    // 同一の単路部内から検索
    // Search within the same section
    for (auto itr_e : _subsecs)
    {
        if (itr_e.second == subsec)
        {
            continue;
        }
        for (int i = 0; i < itr_e.second->numVertexes(); i++)
        {
            AmuLineSegment line = itr_e.second->edge(i);

            /*
             * 対象の辺と共通した始点終点を持つサブセクションを探す
             *
             * Find the subsection that has the same start and end
             * points as the target edge
             */
            if ((commonEdge.pointBegin() == line.pointBegin()
                 && commonEdge.pointEnd() == line.pointEnd())
                || (commonEdge.pointBegin() == line.pointEnd()
                    && commonEdge.pointEnd() == line.pointBegin()))
            {
                return itr_e.second;
            }
        }
    }
    for (int j = 0; j < 2; j++)
    {
        for (auto itr_s : _incInters[j]->subLaneBundles())
        {
            for (int i = 0; i < itr_s.second->numVertexes(); i++)
            {
                AmuLineSegment line = itr_s.second->edge(i);

                /*
                 * 対象の辺と共通した始点終点を持つサブセクションを探す
                 *
                 * Find the subsection that has the same start and end
                 * points as the target edge
                 */
                if ((commonEdge.pointBegin() == line.pointBegin()
                     && commonEdge.pointEnd() == line.pointEnd())
                    || (commonEdge.pointBegin() == line.pointEnd()
                        && commonEdge.pointEnd() == line.pointBegin()))
                {
                    return itr_s.second;
                }
            }
        }
    }
    return nullptr;
}

//==============================================================================
int Section::_numAgents(double start, double distance, bool up) const
{
    int result = -1;
    if (start < 0.1)
    {
        start = 0;
    }
    if (distance > length())
    {
        distance = length();
    }
    if (start < length())
    {
        result = 0;
        for (auto itr_l : _lanes)
        {
            // 指定した方向のレーンあるかどうか
            // Whether the lane is in the specified direction
            if (isUp(itr_l.second) != up)
            {
                continue;
            }

            for (auto itr_a : itr_l.second->agents())
            {
                /*
                 * 統計に含めるかどうか判断
                 * - 例えば，路面電車は除外する
                 *
                 * Determine whether to include in statistics
                 * - For example, trams are excluded
                 */
                if (itr_a->distanceFromInflowBorder() >= start
                    && itr_a->distanceFromInflowBorder() < start + distance)
                {
                    result++;
                }
            }
        }
    }
    return result;
}

//==============================================================================
bool Section::canAcceptLaneShift(const Intersection* intersection) const
{
    vector<const Lane*> inflowLanes = lanesFrom(intersection);
    for (auto itr : inflowLanes)
    {
        if (!(itr->canAcceptLaneShift()))
        {
            return false;
        }
    }
    return true;
}

//==============================================================================
bool Section::permitsPassing(bool isUp, const VehicleType& type) const
{
    return _restrictions[isUp].permitsPassing(type);
}

//==============================================================================
void Section::addRoutingProbability(
    bool isUp, const VehicleType& type, double probability)
{
    // 指定された type をキーに持つ要素を検索
    // Find elements with specified type as key
    if (_routingProbability[isUp].find(type) == _routingProbability[isUp].end())
    {
        // 見つからなかった場合は新規に登録
        // If not found, register it as a new element
        _routingProbability[isUp].insert(
            map<VehicleType, double>::value_type(type, probability));
    }
    else
    {
        cerr << "WARNING: VehicleType[ " << type
             << "] is already added to routing probability table of"
             << " section[" << _id << "]" << endl;
    }
}

//==============================================================================
double Section::routingProbability(bool isUp, VehicleType type) const
{
    // 指定された type をキーに持つ要素を検索
    // Find elements with specified type as key
    auto itr = _routingProbability[isUp].find(type);

    if (itr == _routingProbability[isUp].end())
    {
        return 1.0;
    }
    else
    {
        return (*itr).second;
    }
}

//==============================================================================
double Section::routingProbability(Intersection* inter, VehicleType type) const
{
    if (inter == _incInters[0])
    {
        return routingProbability(true, type);
    }
    else if (inter == _incInters[1])
    {
        return routingProbability(false, type);
    }
    else
    {
        cerr << "ERROR: intersection[" << inter->id()
             << "] is not a connected intersection of section[" << _id << "]"
             << endl;
        abort();
    }
}

//==============================================================================
double Section::averageVelocity(bool isUp) const
{
    unsigned int numVehicles = 0;
    double       sum         = 0.0;

    // 調和平均
    // Harmonic mean
    for (auto itr_l : lanesWithDirection(isUp))
    {
        for (auto itr_a : const_cast<Lane*>(itr_l)->agents())
        {
            Vehicle* vehicle = dynamic_cast<Vehicle*>(itr_a);
            if (vehicle)
            {
                numVehicles++;
                sum += 1.0 / vehicle->behavior()->aveVelocityInSection();
            }
        }
    }
    if (numVehicles == 0)
    {
        return _speedLimit[isUp] / 3600.0; // [km/h]->[m/ms]
    }
    else
    {
        return numVehicles / sum;
    }
}

//==============================================================================
void Section::setSpeedLimit(bool isUp, double speedLimit)
{
    _speedLimit[isUp] = speedLimit;

    // レーンの属性を更新する
    // Update lane property
    for (auto itr : _lanes)
    {
        if (this->isUp(itr.second) == isUp)
        {
            itr.second->setSpeedLimit(speedLimit);
        }
    }
}

//==============================================================================
void Section::print(ostream& out) const
{
    out << "--- Section Information ---" << endl;
    out << "ID: " << _id << ", Position: " << _center.x() << "," << _center.y()
        << "," << _center.z() << endl;
    out << "Incident Intersection ID: " << _incInters[0]->id() << ", "
        << _incInters[1]->id() << endl;

    out << "Lane Connection:" << endl;
    // ソート
    // Sorting
    map<string, Lane*> mapLanes(_lanes.begin(), _lanes.end());
    for (auto itr : mapLanes)
    {
        string laneId = itr.second->id();
        out << "  " << laneId.substr(0, 4) << ", " << laneId.substr(4, 4)
            << endl;
    }

    out << "Vertex:" << endl;
    for (auto itr : _vertexes)
    {
        out << "  " << itr.x() - _center.x() << ", " << itr.y() - _center.y()
            << ", " << itr.z() - _center.z() << endl;
    }

    out << "Length: " << _length << endl;
    out << "Speed Limit: " << _speedLimit[0] << ", " << _speedLimit[1] << endl;

    out << "Vehicle Restriction:" << endl;
    for (unsigned int i = 0; i < 2; i++)
    {
        out << "  [" << i << "]: ";
        _restrictions[i].print(out);
    }

    out << endl;
}
