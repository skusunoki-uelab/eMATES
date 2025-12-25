/* **************************************************************************
 * Copyright 2014-2024 ADVENTURE Project
 * All Rights Reserved
 * ************************************************************************** */
/**
 * @file Intersection.cpp
 */
#include "Intersection.hpp"
#include "Connector.hpp"
#include "CustomMessage.hpp"
#include "Lane.hpp"
#include "LaneInIntersection.hpp"
#include "RoadMap.hpp"
#include "RouteCacheContainer.hpp"
#include "RoutingLink.hpp"
#include "RoutingNode.hpp"
#include "Section.hpp"
#include "Signal.hpp"
#include "SubIntersection.hpp"
#include "Vehicle.hpp"
#include "io/IntersectionBuilder.hpp"
#include <AmuConverter.hpp>
#include <AmuLineSegment.hpp>
#include <AmuPoint.hpp>
#include <cmath>
#include <iostream>
#include <algorithm>

using namespace std;
using namespace amu::geometry;
using amu::converter::formatId;

//#define INTERSECTION_DEBUG

//==============================================================================
Intersection::Intersection(
    const std::string& id, const std::string& type, RoadMap* parent)
    : LaneBundle(id, parent)
{
    /*
     * 引数 type は numIn，numOut の列．文字数は偶数でなければならない．
     *
     * Argument type represents a sequence of numIn and numOut. The number fo
     * characters must be even.
     */
    if ((type.size() % 2) != 0)
    {
        ostringstream sse;
        sse << "Number of digits of the type of Intersection[" << _id
            << "] is not even (" << type << ")" << endl;
        amu::msg::error(sse.str());
        exit(EXIT_FAILURE);
    }

    /*
     * 1文字ずつ切り出しintに変換する．ひとまずnumIn，numOutは一桁の整数に限定．
     *
     * Extract each character and convert to int. For now, limit numIn and
     * numOut to 1-digit integers.
     */
    for (unsigned int i = 0; i < type.size(); i += 2)
    {
        if (!isdigit(type[i]) || !isdigit(type[i + 1]))
        {
            ostringstream sse;
            sse << "Type of Intersection[" << _id << "] is invalid (" << type
                << ")" << endl;
            amu::msg::error(sse.str());
            exit(EXIT_FAILURE);
        }
        _numIn.emplace_back(type[i] - '0');
        _numOut.emplace_back(type[i + 1] - '0');
    }

    unsigned int size = type.size() / 2;

    // _incSections の初期化
    // Initialize _incSections
    _incSections.reserve(size);
    for (unsigned int i = 0; i < size; i++)
    {
        _incSections.emplace_back(nullptr);
    }

    // _linkFlowRecordsの初期化
    // Initialize _linkFlowRecords
    for (unsigned int i = 0; i < size; i++)
    {
        LinkFlowRecord* record = new LinkFlowRecord();
        record->setIntersection(this);
        record->prepareRecording(size);
        _linkFlowRecords.emplace_back(record);
    }

    _signal = nullptr;

    _sidewalkWidth = 0.0;
    _crosswalkWidth.clear();

    _restrictions.resize(size);
    for (unsigned int i = 0; i < size; i++)
    {
        _restrictions[i].resize(size);
    }

    initializeMembers();
}

//==============================================================================
void Intersection::initializeMembers()
{
    _routeCacheContainer = new RouteCacheContainer(this);
#ifdef INCLUDE_PEDESTRIANS
    _pedExt = new IntersectionPedExt(this);
#endif //INCLUDE_PEDESTRIANS

#ifdef INCLUDE_TRAMS
    _tramExt = new IntersectionTramExt(this);
#endif //INCLUDE_TRAMS
}

//==============================================================================
Intersection::~Intersection()
{
    // _subsecs と _lanes は ~LaneBundle() で delete される
    // _subsecs and _lanes are deleted with ~LaneBundle()
    for (auto itr : _borders)
    {
        delete itr;
    }
    _borders.clear();

    delete _rdTable;

    if (_routeCacheContainer)
    {
        delete _routeCacheContainer;
    }

    for (auto itr : _linkFlowRecords)
    {
        delete itr;
    }
    _linkFlowRecords.clear();

#ifdef INCLUDE_PEDESTRIANS
    delete _pedExt;
#endif //INCLUDE_PEDESTRIANS

#ifdef INCLUDE_TRAMS
    delete _tramExt;
#endif //INCLUDE_TRAMS
}

//==============================================================================
double Intersection::length(int inDir, int outDir) const
{
    ASSERT_MSG(inDir >= 0);
    ASSERT_MSG(inDir < _nexts.size());
    ASSERT_MSG(outDir >= 0);
    ASSERT_MSG(outDir < _nexts.size());

    const AmuPoint ip
        = _borders[inDir]->lineSegment().createInteriorPoint(1, 1);
    const AmuPoint op
        = _borders[outDir]->lineSegment().createInteriorPoint(1, 1);

    return ip.distance(op);
}

//==============================================================================
LaneBundle* Intersection::nextBundle(const Lane* lane) const
{
    if (containsNextLane(lane))
    {
        return nullptr;
    }
    else
    {
        return nextSection(lane);
    }
}

//==============================================================================
LaneBundle* Intersection::previousBundle(const Lane* lane) const
{
    if (containsPreviousLane(lane))
    {
        return nullptr;
    }
    else
    {
        return previousSection(lane);
    }
}

//==============================================================================
void Intersection::setNext(Intersection* inter)
{
    // 重複があればコメントする
    // Comment if there is a duplication
    if (find(_nexts.begin(), _nexts.end(), inter) != _nexts.end())
    {
        ostringstream ssw;
        ssw << "Intersection[" << _id
            << "] has duplicated adjacent Interseciton[" << inter->id() << "]."
            << endl;
        amu::msg::warn(ssw.str());
        return;
    }
    _nexts.emplace_back(inter);
}

//==============================================================================
Intersection* Intersection::nextStraight(const Intersection* from) const
{
    int inflowDir = direction(from);
    for (unsigned int i = 0; i < _nexts.size(); i++)
    {
        if ((*_rdTable)(inflowDir, i) == RD::STRAIGHT
            && hasValidPath(from, _nexts[i]))
        {
            return _nexts[i];
        }
    }
    return nullptr;
}

//==============================================================================
Intersection* Intersection::nextLeft(const Intersection* from) const
{
    int inflowDir = direction(from);
    for (unsigned int i = 0; i < _nexts.size(); i++)
    {
        if ((*_rdTable)(inflowDir, i) == RD::LEFT
            && hasValidPath(from, _nexts[i]))
        {
            return _nexts[i];
        }
    }
    return nullptr;
}

//==============================================================================
Intersection* Intersection::nextRight(const Intersection* from) const
{
    int inflowDir = direction(from);
    for (unsigned int i = 0; i < _nexts.size(); i++)
    {
        if ((*_rdTable)(inflowDir, i) == RD::RIGHT
            && hasValidPath(from, _nexts[i]))
        {
            return _nexts[i];
        }
    }
    return nullptr;
}

//==============================================================================
Intersection* Intersection::nextAnother(const Intersection* from) const
{
    unsigned int inflowDir = direction(from);
    for (unsigned int i = 0; i < _nexts.size(); i++)
    {
        if (i == inflowDir)
        {
            continue;
        }
        else if (hasValidPath(from, _nexts[i]))
        {
            return _nexts[i];
        }
    }
    return nullptr;
}

//==============================================================================
bool Intersection::hasValidPath(
    const Intersection* from, const Intersection* to) const
{
    // ODノードではUターン禁止
    // No U-turn at ODNode
    if (_nexts.size() == 1 && from == to)
    {
        return false;
    }

    int inflowDir  = direction(from);
    int outflowDir = direction(to);

    // 流入レーンを取得する
    // Get inflow lanes
    vector<const Lane*> inflowLanes;
    for (auto itr : _borders[inflowDir]->inPoints())
    {
        auto lanes = lanesFromConnector(itr);
        copy(lanes.begin(), lanes.end(), back_inserter(inflowLanes));
    }

    for (auto itr : inflowLanes)
    {
        if (hasValidPath(itr, outflowDir))
        {
            return true;
        }
    }
    return false;
}

//==============================================================================
bool Intersection::isNetworked(const Intersection* inter)
{
    // 直接接続しているかどうか調査
    // Check whether they are directly connected
    for (const auto itr : _nexts)
    {
        if (itr == inter)
        {
            return true;
        }
    }

    // 間接的に接続しているかどうか調査
    // Check whether they are indirectly connected
    bool                        result = false;
    vector<const Intersection*> alreadySearched;
    alreadySearched.emplace_back(this);

    for (unsigned int i = 0; i < _nexts.size() && result == false; i++)
    {
        if (_nexts[i] == this)
        {
            continue;
        }
        result = _nexts[i]->_isNetworked(alreadySearched, inter);
    }
    return result;
}

//==============================================================================
bool Intersection::_isNetworked(
    std::vector<const Intersection*>& alreadySearched,
    const Intersection*               inter)
{
    // 直接接続しているかどうか調査
    // Check whether they are directly connected
    for (const auto itr : _nexts)
    {
        if (itr == inter)
        {
            return true;
        }
    }

    // 間接的に接続しているかどうか調査
    // Check whether they are indirectly connected
    bool result = false;
    alreadySearched.emplace_back(this);
    for (unsigned int i = 0; i < _nexts.size() && result == false; i++)
    {
        if (find(alreadySearched.begin(), alreadySearched.end(), _nexts[i])
            != alreadySearched.end())
        {
            continue;
        }
        result = _nexts[i]->_isNetworked(alreadySearched, inter);
    }
    return result;
}

//==============================================================================
void Intersection::addBorder(Border* border, int edgeNum)
{
    ASSERT_MSG(border);
    ASSERT_MSG(edgeNum >= 0);
    ASSERT_MSG(edgeNum < numVertexes());
    ASSERT_MSG(_dir2edge.size() == _borders.size());

    _dir2edge.emplace_back(edgeNum);
    _borders.emplace_back(border);
}

//==============================================================================
int Intersection::direction(const Connector* connector) const
{
    // 該当する connector がなければ-1を返す
    // Return -1 if there is no corresponding connector
    int dir = -1;
    for (unsigned int i = 0; i < _nexts.size(); i++)
    {
        auto inPoints = _borders[i]->inPoints();
        if (find(inPoints.begin(), inPoints.end(), connector) != inPoints.end())
        {
            return i;
        }
        auto outPoints = _borders[i]->outPoints();
        if (find(outPoints.begin(), outPoints.end(), connector)
            != outPoints.end())
        {
            return i;
        }
    }

#ifdef INCLUDE_TRAMS
    if (_tramExt)
    {
        dir = _tramExt->tramDirection(connector);
    }
#endif //INCLUDE_TRAMS

    return dir;
}

//==============================================================================
vector<int> Intersection::inflowDirections(const Lane* lane) const
{
    vector<int> result;
    if (containsPreviousLane(lane))
    {
        for (auto itr : lane->previousLanes())
        {
            vector<int> prevResult = inflowDirections(itr);
            result.insert(result.end(), prevResult.begin(), prevResult.end());
        }
    }
    else
    {
        result.emplace_back(direction(lane->beginConnector()));
    }
    return result;
}

//==============================================================================
vector<int> Intersection::outflowDirections(const Lane* lane) const
{
    vector<int> result;
    if (containsNextLane(lane))
    {
        for (auto itr : lane->nextLanes())
        {
            vector<int> nextResult = outflowDirections(itr);
            result.insert(result.end(), nextResult.begin(), nextResult.end());
        }
    }
    else
    {
        result.emplace_back(direction(lane->endConnector()));
    }
    return result;
}

//==============================================================================
int Intersection::oppositeDirection(int direction) const
{
    ASSERT_MSG(direction >= 0);
    ASSERT_MSG(direction < _nexts.size());
    for (int i = 0; i < static_cast<signed int>(_nexts.size()); i++)
    {
        if ((*_rdTable)(direction, i) == RD::STRAIGHT)
        {
            return i;
        }
    }
    return -1;
}

//==============================================================================
vector<const Lane*> Intersection::lanesFrom(int direction) const
{
    vector<const Lane*> resultLanes;
    resultLanes.clear();
    for (auto itr : _borders[direction]->inPoints())
    {
        vector<const Lane*> tmpLanes = lanesFromConnector(itr);
        resultLanes.insert(resultLanes.end(), tmpLanes.begin(), tmpLanes.end());
    }
    return resultLanes;
}

//==============================================================================
vector<const Lane*> Intersection::lanesTo(int direction) const
{
    vector<const Lane*> resultLanes;
    resultLanes.clear();

    for (auto itr : _borders[direction]->outPoints())
    {
        vector<const Lane*> tmpLanes = lanesToConnector(itr);
        resultLanes.insert(resultLanes.end(), tmpLanes.begin(), tmpLanes.end());
    }

    return resultLanes;
}

//==============================================================================
bool Intersection::hasValidPath(const Lane* lane, int dir) const
{
    bool flag = false;
    if (containsLane(lane))
    {
        if (containsNextLane(lane))
        {
            /*
             * 次のレーンも交差点内の場合は，次のレーンを引数にして
             * 再帰呼び出し
             *
             * If the next lane is also within this intersection, 
             * recursively call with the next lane as an argment.
             */
            for (auto itr : lane->nextLanes())
            {
                flag = hasValidPath(itr, dir);
                if (flag)
                {
                    break;
                }
            }
        }
        else
        {
            /*
             * 次のレーンが交差点外の場合は，このレーンの終点で判定する
             *
             * If the next lane is outside this intersection, judge by
             * the end point of the lane
             */
            if (direction(lane->endConnector()) == dir)
            {
                flag = true;
            }
        }
    }
#ifdef INTERSECTION_DEBUG
    ostringstream oss;
    if (flag)
    {
        oss << "At Intersection:[" << _id << "], Lane[" << lane->id()
            << "] is reachable to direction[" << dir << "]" << endl;
        amu::msg::message(cout, oss.str());
    }
    else
    {
        oss << "At Intersection[" << _id << "], Lane[" << lane->id()
            << "] is NOT reachable to direction[" << dir << "]" << endl;
        amu::msg::message(cout, oss.str());
    }
#endif //INTERSECTION_DEBUG

    return flag;
}

//==============================================================================
bool Intersection::hasMainLane(const Lane* lane) const
{
    ASSERT_MSG(containsLane(lane));

    /*
     * レーンの始点の周番号は識別番号の10 000 000の位
     *
     * The loop number of the start point of the lane is the 10 000 000s
     * position of its ID number.
     */
    int beginRound = stoi(lane->id()) / 10000000;

    /*
     * レーンの終点の周番号は識別番号の1 000の位
     *
     * The loop number of the end point of the lane is the 1 000s position of
     * its ID number.
     */
    int endRound = (stoi(lane->id()) % 10000) / 1000;

    /*
     * 交差点内中心レーンは始点，終点とも周番号が0
     *
     * The main lane in the intersection has a loop number of 0 at both the
     * start and end points.
     */
    if (beginRound == 0 && endRound == 0)
    {
        return true;
    }
    else
    {
        return false;
    }
}

//==============================================================================
vector<const Lane*> Intersection::mainLanes() const
{
    vector<const Lane*> result_lanes;
    for (auto itr : _lanes)
    {
        if (hasMainLane(itr.second))
        {
            result_lanes.emplace_back(itr.second);
        }
    }
    return result_lanes;
}

//==============================================================================
vector<const Lane*> Intersection::collisionLanes(const Lane* lane) const
{
    ASSERT_MSG(containsLane(lane));

    /*
     * 計算量節約のため，mainLaneに限定する．限定せず _lanes を対象にするほうが
     * 厳密かもしれない．歩行者を考慮しない場合はどちらでもよい．
     *
     * Limited to mainLane to save computational cost. It may be more strict to 
     * target _lanes without limitation. If not consider pedestrians, either is
     * fine.
     */
    vector<const Lane*> result_lanes;

    AmuPoint thisBegin = lane->beginConnector()->point();
    AmuPoint thisEnd   = lane->endConnector()->point();

    for (auto itr : mainLanes())
    {
        if (itr == lane)
        {
            continue;
        }
        else
        {
            /*
             * 2線分 (thisBegin, thisEnd) と (thatBegin, thatEnd) の交差判定
             *
             * Intersection detection between 2 line segments
             * (thisBegin, thisEnd) and (thatBegin, thatEnd).  
             */
            AmuPoint thatBegin = itr->beginConnector()->point();
            AmuPoint thatEnd   = itr->endConnector()->point();
            double   v0        = (thisBegin.x() - thisEnd.x())
                    * (thatBegin.y() - thisBegin.y())
                + (thisBegin.y() - thisEnd.y())
                    * (thisBegin.x() - thatBegin.x());
            double v1
                = (thisBegin.x() - thisEnd.x()) * (thatEnd.y() - thisBegin.y())
                + (thisBegin.y() - thisEnd.y()) * (thisBegin.x() - thatEnd.x());
            if (abs(v0) < 1.0e-5)
                v0 = 0.0;
            if (abs(v1) < 1.0e-5)
                v1 = 0.0;

            // 交差する場合はv0とv1が異符号
            // If they intersect, v0 and v1 have opposite signes
            if (v0 * v1 <= 0)
            {
                result_lanes.push_back(itr);

                // 上流のレーンも含める
                // Include upstream lanes
                if (containsPreviousLane(itr))
                {
                    const vector<const Lane*>& prevLanes = itr->previousLanes();
                    result_lanes.insert(
                        result_lanes.end(), prevLanes.begin(), prevLanes.end());
                }
            }
        }
    }
    return result_lanes;
}

//==============================================================================
void Intersection::getCollisionLanes(
    const vector<const Lane*>& lanes, vector<const Lane*>& result_inter,
    vector<const Lane*>& result_section) const
{
    result_inter.clear();
    result_section.clear();

    // 各レーンの交錯レーンを集約する
    // Aggregate crossing lanes for each lane
    for (auto itr : lanes)
    {
        LaneInIntersection* lookupLane
            = dynamic_cast<LaneInIntersection*>(const_cast<Lane*>(itr));
        if (!lookupLane)
        {
            continue;
        }
        ASSERT_MSG(containsLane(lookupLane));

        vector<const Lane*>& veci = lookupLane->collisionLanesInIntersection();
        vector<const Lane*>& vecs = lookupLane->collisionLanesInSection();
        result_inter.insert(result_inter.end(), veci.begin(), veci.end());
        result_section.insert(result_section.end(), vecs.begin(), vecs.end());
    }

    // result_inter から重複を削除する
    // Remove duplicates from result_inter
    {
        sortLanes(result_inter);
        auto itr = unique(result_inter.begin(), result_inter.end());
        result_inter.erase(itr, result_inter.end());
    }

    // result_inter から lanes に含まれる各要素を除去する
    // Remove each element in lanes from result_inter
    for (auto itr : lanes)
    {
        auto itr_l = remove(result_inter.begin(), result_inter.end(), itr);
        result_inter.erase(itr_l, result_inter.end());
    }

    // result_section から重複を除去する
    // Remove duplicates from result_section
    {
        sortLanes(result_section);
        auto itr = unique(result_section.begin(), result_section.end());
        result_section.erase(itr, result_section.end());
    }
}

//==============================================================================
vector<const Lane*> Intersection::collisionLanesFront(
    const Lane* lane, double length) const
{
    vector<const Lane*> result_lanes;
    for (auto itr : collisionLanes(lane))
    {
        AmuPoint tmpPoint;
        lane->createIntersectionPoint(itr->lineSegment(), &tmpPoint);
        double distance = lane->lineSegment()->pointBegin().distance(tmpPoint);
        if (distance > length)
        {
            result_lanes.push_back(itr);
        }
    }
    return result_lanes;
}

//==============================================================================
const Connector* Intersection::edgeConnector(int idInt)
{
    if (crosswalkWidth(idInt / 100) > 1e-6)
    {
        /*
         * 接続元の境界に横断歩道が設置されている場合は内部コネクタを戻す
         *
         * Return an internal connector if the border connecting from has a
         * crosswalk
         */
        return internalConnector(
            formatId(to_string(idInt), NUM_FIGURE_FOR_CONNECTOR_LOCAL));
    }
    else
    {
        // 接続元の境界に横断歩道が設置されていない場合
        // When the border connecting from has no crosswalk
        return _borders[idInt / 100]->connector(idInt % 100);
    }
}

//==============================================================================
SubLaneBundle* Intersection::pairedSubLaneBundle(
    SubLaneBundle* subsec, int edgeNum) const
{
    ASSERT_MSG(edgeNum >= 0);
    ASSERT_MSG(edgeNum < subsec->numVertexes());
    AmuLineSegment commonEdge = subsec->edge(edgeNum);

    // 同一の交差点内から検索
    // Search from within the same intersection
    for (auto itr_s : _subsecs)
    {
        if (itr_s.second == subsec)
        {
            continue;
        }
        for (int i = 0; i < itr_s.second->numVertexes(); i++)
        {
            /*
             * 対象の辺と共通した始点終点を持つサブセクションを探す
             *
             * Find the subsection that has a common start and end
             * points with the target edge
             */
            if (commonEdge.coincidesWith(itr_s.second->edge(i)))
            {
                return itr_s.second;
            }
        }
    }

    // 接続する単路部内から検索
    // Search from within the connecting section
    for (const auto itr : _incSections)
    {
        if (itr == nullptr)
        {
            continue;
        }
        for (auto itr_s : itr->subLaneBundles())
        {
            for (int i = 0; i < itr_s.second->numVertexes(); i++)
            {
                /*
                 * 対象の辺と共通した始点終点を持つサブセクションを探す
                 *
                 * Find the subsection that has a common start and end
                 * points with the target edge
                 */
                if (commonEdge.coincidesWith(itr_s.second->edge(i)))
                {
                    return itr_s.second;
                }
            }
        }
    }
    return nullptr;
}

//==============================================================================
Signal::Permission Intersection::permission(int from) const
{
    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    // _signal がない場合（無信号交差点）は通行可
    // If there is no signal (unsignalized intersection), passage is permitted.
    if (!_signal)
    {
        return Signal::Permission::PERMISSION;
    }

    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    // 信号現示を取得
    // Get signal aspects
    SignalColor::MainState main = _signal->currentAspect()->mainColor(from);
    SignalColor::MainState prev = _signal->previousAspect()->mainColor(from);

    if (main == SignalColor::MainState::BLUE
        || (main == SignalColor::MainState::YELLOW
            && prev == SignalColor::MainState::BLUE))
    {
        return Signal::Permission::PERMISSION;
    }
    else if (main == SignalColor::MainState::YELLOWBLINK)
    {
        return Signal::Permission::CREEPING;
    }
    else if (main == SignalColor::MainState::REDBLINK)
    {
        return Signal::Permission::PAUSING;
    }
    return Signal::Permission::PROHIBITION;
}

//==============================================================================
Signal::Permission Intersection::permission(
    int from, const RelativeDirection& rd, Vehicle*) const
{
    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    // _signal がない場合（無信号交差点）は通行可
    // If there is no signal (unsignalized intersection), passage is permitted.
    if (!_signal)
    {
        return Signal::Permission::PERMISSION;
    }

    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    // 信号現示を取得
    // Get signal aspects
    SignalColor::MainState mainColor = _signal->mainColor(from);
    SignalColor::SubState  subColor  = _signal->subColor(from);

    // mainColor が黄 -> 内部的には直前の現示と同じ
    // mainColor is yellow -> internally same as previous aspect
    if (mainColor == SignalColor::MainState::YELLOW)
    {
        mainColor = _signal->prevMainColor(from);
        subColor  = _signal->prevSubColor(from);
    }

    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    // mainColor が青
    // mainColor is green
    if (mainColor == SignalColor::MainState::BLUE
        || mainColor == SignalColor::MainState::YELLOW)
    {

        return Signal::Permission::PERMISSION;
    }
    // mainColor が黄点滅 -> 徐行
    // mainColor is flushing yellow -> going slowly
    else if (mainColor == SignalColor::MainState::YELLOWBLINK)
    {
        return Signal::Permission::CREEPING;
    }
    // mainColor が赤点滅 -> 一旦停止
    // mainColor is flushing red -> pause
    else if (mainColor == SignalColor::MainState::REDBLINK)
    {
        // mainColor が赤点滅->一旦停止
        return Signal::Permission::PAUSING;
    }

    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    // mainColorが赤
    // mainColor is red
    else
    {
        //----------------------------------------------------------------------
        // 直進
        // Going straight
        if (rd == RD::STRAIGHT)
        {
            if (subColor == SignalColor::SubState::ALL
                || subColor == SignalColor::SubState::STRAIGHT
                || subColor == SignalColor::SubState::STRAIGHTLEFT
                || subColor == SignalColor::SubState::STRAIGHTRIGHT)
            {
                return Signal::Permission::PERMISSION;
            }
            else
            {
                return Signal::Permission::PROHIBITION;
            }
        }
        //----------------------------------------------------------------------
        // 左折
        // Turning left
        else if (rd == RD::LEFT)
        {
            if (subColor == SignalColor::SubState::ALL
                || subColor == SignalColor::SubState::LEFT
                || subColor == SignalColor::SubState::STRAIGHTLEFT
                || subColor == SignalColor::SubState::LEFTRIGHT)
            {
                return Signal::Permission::PERMISSION;
            }
            else
            {
                return Signal::Permission::PROHIBITION;
            }
        }
        //----------------------------------------------------------------------
        // 右折
        // Turning right
        else if (rd == RD::RIGHT)
        {
            if (subColor == SignalColor::SubState::ALL
                || subColor == SignalColor::SubState::RIGHT
                || subColor == SignalColor::SubState::STRAIGHTRIGHT
                || subColor == SignalColor::SubState::LEFTRIGHT)
            {
                return Signal::Permission::PERMISSION;
            }
            else
            {
                return Signal::Permission::PROHIBITION;
            }
        }
        //----------------------------------------------------------------------
        // その他
        // Others
        else
        {
            return Signal::Permission::PROHIBITION;
        }
    }
}

//==============================================================================
void Intersection::addRoutingLink(RoutingLink* link)
{
    auto itr = find(_routingLinks.begin(), _routingLinks.end(), link);
    if (itr != _routingLinks.end())
    {
        ostringstream ssw;
        ssw << "RoutingLink[" << link->id() << "] has already registered at "
            << "Intersection[" << _id << "]" << endl;
        amu::msg::warn(ssw.str());
        return;
    }
    _routingLinks.emplace_back(link);
}

//==============================================================================
void Intersection::observeLinkFlow()
{
    for (auto itr : _linkFlowRecords)
    {
        itr->calcMacroQuantities();
        itr->reset();
        itr->recordPositionsInSection();
        itr->instructOutput();
    }
    for (const auto itr : _routingLinks)
    {
        itr->registerToBeUpdated();
    }
}

//==============================================================================
void Intersection::initializeLinkFlowObservation()
{
    for (auto itr : _linkFlowRecords)
    {
        itr->calcMacroQuantities();
    }

    for (const auto itr : _routingLinks)
    {
        itr->registerToBeUpdated();
    }
}

//==============================================================================
void Intersection::addAllowedVehicleType(
    const Intersection* from, const Intersection* to, const VehicleType& type)
{
    int fromDir = direction(from);
    if (fromDir == -1)
    {
        ostringstream ssw;
        ssw << "Intersection[" << from->id()
            << "] is not adjacent to Intersection[" << _id << endl;
        amu::msg::warn(ssw.str());
        return;
    }

    int toDir = direction(to);
    if (toDir == -1)
    {
        ostringstream ssw;
        ssw << "Intersection[" << to->id()
            << "] is not adjacent to Intersection[" << _id << endl;
        amu::msg::warn(ssw.str());
        return;
    }

    _restrictions[fromDir][toDir].addAllowedVehicleType(type);
}

//==============================================================================
void Intersection::addDeniedVehicleType(
    const Intersection* from, const Intersection* to, const VehicleType& type)
{
    int fromDir = direction(from);
    if (fromDir == -1)
    {
        ostringstream ssw;
        ssw << "Intersection[" << from->id()
            << "] is not adjacent to Intersection[" << _id << endl;
        amu::msg::warn(ssw.str());
        return;
    }

    int toDir = direction(to);
    if (toDir == -1)
    {
        ostringstream ssw;
        ssw << "Intersection[" << to->id()
            << "] is not adjacent to Intersection[" << _id << endl;
        amu::msg::warn(ssw.str());
        return;
    }

    _restrictions[fromDir][toDir].addDeniedVehicleType(type);
}

//==============================================================================
bool Intersection::permitsPassing(
    const Intersection* from, const Intersection* to,
    const VehicleType& type) const
{
    int fromDir = direction(from);
    int toDir   = direction(to);
    ASSERT_MSG(fromDir != -1);
    ASSERT_MSG(toDir != -1);

    return _restrictions[fromDir][toDir].permitsPassing(type);
}

//==============================================================================
void Intersection::printMapInfo(ostream& out) const
{
    ostringstream oss;
    // (ID):(x,y,z)/(numIn numOut)-(IDs of next Intersection)
    oss << _id;
    if (_nexts.size() == 1)
    {
        oss << "(O)"; // OD node
    }
    else if (_signal)
    {
        oss << "(S)"; // signalized
    }
    else
    {
        oss << "(U)"; // unsignalized
    }
    oss << ":(" //
        << _center.x() << "," << _center.y() << "," << _center.z() << ")/";

    auto itr_in  = _numIn.begin();
    auto itr_out = _numOut.begin();
    while (itr_in != _numIn.end() && itr_out != _numOut.end())
    {
        oss << (*itr_in) << (*itr_out);
        itr_in++;
        itr_out++;
    }
    oss << "-";

    for (auto itr : _nexts)
    {
        oss << itr->id() << ",";
    }
    oss << endl;
    amu::msg::message(out, oss.str());
}

//==============================================================================
void Intersection::print(ostream& out, bool isODNode) const
{
    ostringstream oss;
    oss << "--- Intersection Information ---" << endl;
    oss << "ID: " << _id << ", ODNode: " << isODNode
        << ", Position: " << _center.x() << "," << _center.y() << ","
        << _center.z() << endl;
    oss << "Next Intersection ID: ";
    for (unsigned int i = 0; i < _nexts.size(); i++)
    {
        if (i != 0)
        {
            oss << ", ";
        }
        oss << _nexts[i]->id();
    }
    oss << endl;

    if (!isODNode)
    {
        oss << "Relative Direction:" << endl;
        for (unsigned int i = 0; i < _borders.size(); i++)
        {
            oss << "  ";
            for (unsigned int j = 0; j < _borders.size(); j++)
            {
                if (j != 0)
                {
                    oss << ", ";
                }
                oss << (*_rdTable)(i, j);
            }
            oss << endl;
        }

        oss << "Lane Connection:" << endl;
        // ソート
        // Sorting
        map<string, Lane*> mapLanes(_lanes.begin(), _lanes.end());
        for (auto itr : mapLanes)
        {
            string laneId = itr.second->id();
            oss << "  " << laneId.substr(0, 4) << ", " << laneId.substr(4, 4)
                << endl;
        }
    }
    oss << "Vertex:" << endl;
    for (auto itr : _vertexes)
    {
        oss << "  " << itr.x() - _center.x() << ", " << itr.y() - _center.y()
            << ", " << itr.z() - _center.z() << endl;
    }

    oss << "Vehicle Restriction:" << endl;
    for (unsigned int i = 0; i < _restrictions.size(); i++)
    {
        for (unsigned int j = 0; j < _restrictions[i].size(); j++)
        {
            oss << "  [" << i << "][" << j << "]: ";
            _restrictions[i][j].print(oss);
        }
    }

    oss << "Cost:" << endl;
    for (unsigned int from = 0; from < _nexts.size(); from++)
    {
        for (unsigned int to = 0; to < _nexts.size(); to++)
        {
            int diff = (to - from + numNexts()) % numNexts();
            oss << "  from:" << _nexts[from]->id()
                << ", to:" << _nexts[to]->id() << ", - "
                << _linkFlowRecords[from]->estimatedTravelTime(diff) << endl;
        }
    }
    oss << endl;
    amu::msg::message(out, oss.str());
}
