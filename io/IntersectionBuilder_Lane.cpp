/* ******************************************************************
 * Copyright 2014-2024 ADVENTURE Project
 * All Rights Reserved
 * ****************************************************************** */
/**
 * @file IntersectionBuilder_Lane.cpp
 */
#include "IntersectionBuilder.hpp"
#include "../AppMates.hpp"
#include "../GVManager.hpp"
#include "../Intersection.hpp"
#include "../LaneInIntersection.hpp"
#include "../RelativeDirectionTable.hpp"
#ifdef INCLUDE_TRAMS
#include "../tram/BorderTram.hpp"
#include "../tram/IntersectionTramExt.hpp"
#endif //INCLUDE_TRAMS
#include <AmuConverter.hpp>
#include <AmuPoint.hpp>
#include <AmuStringOperator.hpp>
#include <cassert>
#include <iostream>

using namespace std;
using namespace amu::geometry;
using namespace amu::math;
using namespace amu::converter;
using namespace amu::string_operator;

// #define IB_DEBUG

//======================================================================
bool IntersectionBuilder::_readLaneConnectionFromFile(ifstream* fin)
{
    int numLanes = 0;

#ifdef IB_DEBUG
    cout << "intersection[" << _inter->id() << "], has Lanes below."
         << endl;
#endif

    // 読み込み前の位置を保存する
    // Save position before loading
    ifstream::pos_type pos = fin->tellg();

    while ((*fin).good())
    {
        string         line;
        vector<string> tokens;
        if (!getTokens(fin, &line, &tokens, ','))
        {
            break;
        }
        transform(
            tokens[0].begin(), tokens[0].end(), tokens[0].begin(),
            ::tolower);

        // 相対方向テーブルの指定であれば読み飛ばす
        // Skip if relative direction table specification
        if (tokens[0] == "s" || tokens[0] == "t" || tokens[0] == "l"
            || tokens[0] == "r")
        {
            continue;
        }

        /*
         * "vertex"に到達したら読み込みを終え読み込み前の位置に戻す
         *
         * Stop loading when "vertex" is reached and restore position
         * before loading
         */
        if (tokens[0] == "vertex")
        {
            break;
        }

#ifdef IB_DEBUG
        cout << numLanes + 1 << ": " << line << endl;
#endif
        if (tokens.size() != 2)
        {
            cerr << "line: " << line << endl;
            cerr << "ERROR: " << numLanes + 1 << "th lane in file("
                 << _inter->id() << ".txt) must specify 2 connectors."
                 << endl;
            exit(EXIT_FAILURE);
        }

        _connectorIds.push_back(tokens[0]);
        _connectorIds.push_back(tokens[1]);
        numLanes++;
    }

    // 読み込み前の位置に戻す
    // Restore position before loading
    fin->seekg(pos);
    return true;
}

//======================================================================
bool IntersectionBuilder::_generateLanes()
{
    if (!_connectorIds.empty())
    {
        _generateLanesFromFile();
    }
    else
    {
        _generateDefaultLanes();
    }

#ifdef INCLUDE_TRAMS
    _builderTramExt->generateTramLanes();
#endif //INCLUDE_TRAMS
    return true;
}

//======================================================================
void IntersectionBuilder::_generateLanesFromFile()
{
    unsigned int numLanes = _connectorIds.size() / 2;

    //------------------------------------------------------------------
    // 中心サブセクション
    // Central subsection
    for (unsigned int i = 0; i < numLanes; i++)
    {
        int idIntBegin = stoi(_connectorIds[i * 2]);
        int idIntEnd   = stoi(_connectorIds[i * 2 + 1]);
        int idInt      = idIntBegin * 10000 + idIntEnd;

        const Connector* pointBegin = _inter->edgeConnector(idIntBegin);
        const Connector* pointEnd   = _inter->edgeConnector(idIntEnd);

        if (!pointBegin)
        {
            cerr << "ERROR: pointBegin[" << idIntBegin
                 << "] not found at intersection[" << _inter->id()
                 << "]" << endl;
            exit(EXIT_FAILURE);
        }
        else if (!pointEnd)
        {
            cerr << "ERROR: pointEnd[" << idIntEnd
                 << "] not found at intersection[" << _inter->id()
                 << "]" << endl;
            exit(EXIT_FAILURE);
        }
        _generateLane(idInt, pointBegin, pointEnd);
    }

    //------------------------------------------------------------------
    // 中心サブセクション以外は自動生成する
    // Generate automatically except central subsection
    for (unsigned int i = 0; i < _inter->borders().size(); i++)
    {
        if (_inter->crosswalkWidth(i) < 1e-6)
        {
            continue;
        }
        _generateCrosswalkLanes(i);
    }
}

//======================================================================
void IntersectionBuilder::_generateDefaultLanes()
{
    //------------------------------------------------------------------
    // 中心サブセクション
    // Central subsection
    for (unsigned int i = 0; i < _inter->borders().size(); i++)
    {
        if (_inter->numIn(i) == 0)
        {
            continue;
        }

        for (unsigned int j = 0; j < _inter->borders().size(); j++)
        {
            if (_inter->numOut(j) == 0)
            {
                continue;
            }
            if ((*_rdTable)(i, j).value() == RD::NONE)
            {
                continue;
            }
            else if ((*_rdTable)(i, j).value() == RD::BACK)
            {
                // 現状ではレーンを生成しない
                // Not generate lanes currently
                continue;
            }
            else if ((*_rdTable)(i, j).value() == RD::RIGHT)
            {
                _generateDefaultRightLanes(i, j);
            }
            else if ((*_rdTable)(i, j).value() == RD::STRAIGHT)
            {
                _generateDefaultStraightLanes(i, j);
            }
            else if ((*_rdTable)(i, j).value() == RD::LEFT)
            {
                _generateDefaultLeftLanes(i, j);
            }
        }
    }

    //------------------------------------------------------------------
    // 中心サブセクション以外
    // Non-central subsection
    for (unsigned int i = 0; i < _inter->borders().size(); i++)
    {
        if (_inter->crosswalkWidth(i) < 1e-6)
        {
            continue;
        }
        _generateCrosswalkLanes(i);
    }
}

//======================================================================
void IntersectionBuilder::_generateDefaultRightLanes(
    unsigned int from, unsigned int to)
{
    /*
     * 境界[from]の最右の流入コネクタと境界toの各流出コネクタを結ぶ
     *
     * Connect the rightmost inflow connector of the boundary[from]
     * and each outflow connector of the boundary[to]
     */
    int idIntBegin = from * 100 + _inter->numIn(from) - 1;
    for (int k = 0; k < _inter->numOut(to); k++)
    {
        int idIntEnd = to * 100 + _inter->numIn(to) + k;
        int idInt    = idIntBegin * 10000 + idIntEnd;

        const Connector* pointBegin = _inter->edgeConnector(idIntBegin);
        const Connector* pointEnd   = _inter->edgeConnector(idIntEnd);
        assert(pointBegin && pointEnd);

        _generateLane(idInt, pointBegin, pointEnd);
    }
}

//======================================================================
void IntersectionBuilder::_generateDefaultStraightLanes(
    unsigned int from, unsigned int to)
{
    /*
     * 境界[from]の流入点と境界[to]の流出点を外から順に結ぶ
     *   左側通行の場合は流入点のIDの昇順，右側通行の場合は降順に結ぶ
     *
     * Connect the inflow points of the boundary[from] and the outflow
     * points of the boundary[to] in order from the outside.
     *   In the case of left-hand traffic, connect in ascending order of
     *   inflow point ID numbers, and in the case of right-hand traffic,
     *   connect in descending order.
     */

    /*
     * 流入側が余るのを抑止するため，次の場合は外から2番目から接続
     *  - （流入点数-流出点数 == 2）の場合
     *  - （流入点数-流出点数 == 1）で右折レーンがない場合
     * 流出側が余ったら最後の流入点につなげる
     *
     * In the following cases, connect from second from the outside
     * in order to prevent surplus on the inflow side;
     * - "num. of inflow points" - "num. of outflow points" == 2,
     * - "num. of inflow points" - "num. of outflow points" == 1 and
     *   no right-turn lane.
     * If any outflow points remain, connect them to last inflow point.
     */
    int          kOffset    = 0;
    unsigned int numBorders = _inter->borders().size();
    if (_inter->numIn(from) - _inter->numOut(to) >= 2
        || (_inter->numIn(from) - _inter->numOut(to) == 1
            && (from + 1) % numBorders == to))
    {
        kOffset = 1;
    }

    for (int k = 0; k < _inter->numOut(to); k++)
    {
        int idIntBegin, idIntEnd;
#ifdef RIGHT_HAND_TRAFFIC
        if (k < _inter->numIn(from))
        {
            // ID[numIn-1-kOffset]から降順
            // Descending from ID[numIn-1-kOffset]
            idIntBegin
                = from * 100 + (_inter->numIn(from) - 1 - kOffset) - k;
        }
        else
        {
            // 最も内側 (=最左=ID[0]) の流入点
            // Innermost (=leftmost=ID[0]) inflow point
            idIntBegin = from * 100;
        }
        // ID[numIn]から昇順
        // Ascending from ID[numIn]
        idIntEnd = to * 100 + _inter->numIn(to) + k;

#else //RIGHT_HAND_TRAFFIC not defined
        if (k < _inter->numIn(from))
        {
            // ID[kOffset]から昇順
            // Ascending from ID[kOffset]
            idIntBegin = from * 100 + k + kOffset;
        }
        else
        {
            // 最も内側 (=最右=ID[numIn-1]) の流入点
            // Innermost (=rightmost=ID[numIn-1]) inflow point
            idIntBegin = from * 100 + _inter->numIn(from) - 1;
        }
        // ID[_numIn+_numOut-1]から降順
        // Descending from ID[_numIn+_numOut-1]
        idIntEnd = to * 100
                   + (_inter->numIn(to) + _inter->numOut(to) - 1) - k;

#endif //RIGHT_HAND_TRAFFIC
        int idInt = idIntBegin * 10000 + idIntEnd;

        const Connector* pointBegin = _inter->edgeConnector(idIntBegin);
        const Connector* pointEnd   = _inter->edgeConnector(idIntEnd);
        assert(pointBegin && pointEnd);

        _generateLane(idInt, pointBegin, pointEnd);
    }
}

//======================================================================
void IntersectionBuilder::_generateDefaultLeftLanes(
    unsigned int from, unsigned int to)
{
    /*
     * 境界[from]の最左の流入コネクタと境界toの各流出コネクタを結ぶ
     *
     * Connect the leftmost inflow connector of the boundary[from]
     * and each outflow connector of the boundary[to]
     */
    int idIntBegin = from * 100;
    for (int k = 0; k < _inter->numOut(to); k++)
    {
        int idIntEnd = to * 100 + _inter->numIn(to) + k;
        int idInt    = idIntBegin * 10000 + idIntEnd;

        const Connector* pointBegin = _inter->edgeConnector(idIntBegin);
        const Connector* pointEnd   = _inter->edgeConnector(idIntEnd);
        assert(pointBegin && pointEnd);

        _generateLane(idInt, pointBegin, pointEnd);
    }
}

//======================================================================
void IntersectionBuilder::_generateCrosswalkLanes(int dir)
{
    for (int k = 0; k < _inter->numIn(dir); k++)
    {
        int idIntBegin = 1 * 1000 + dir * 100 + k;
        int idIntEnd   = dir * 100 + k;
        int idInt      = idIntBegin * 10000 + idIntEnd;

        const Connector* pointBegin = _inter->border(dir)->connector(k);
        const Connector* pointEnd = _inter->internalConnector(formatId(
            to_string(idIntEnd), NUM_FIGURE_FOR_CONNECTOR_LOCAL));
        if (pointBegin && pointEnd)
        {
            _generateLane(idInt, pointBegin, pointEnd);
        }
    }
    for (int k = _inter->numIn(dir);
         k < _inter->numIn(dir) + _inter->numOut(dir); k++)
    {
        int idIntBegin = dir * 100 + k;
        int idIntEnd   = 1 * 1000 + dir * 100 + k;
        int idInt      = idIntBegin * 10000 + idIntEnd;

        const Connector* pointBegin
            = _inter->internalConnector(formatId(
                to_string(idIntBegin), NUM_FIGURE_FOR_CONNECTOR_LOCAL));
        const Connector* pointEnd = _inter->border(dir)->connector(k);
        if (pointBegin && pointEnd)
        {
            _generateLane(idInt, pointBegin, pointEnd);
        }
    }
}

//======================================================================
void IntersectionBuilder::_generateLane(
    int idInt, const Connector* pointBegin, const Connector* pointEnd)
{
    string fmtId = formatId(to_string(idInt), NUM_FIGURE_FOR_LANE);
    AmuLineSegment* lineSegment
        = new AmuLineSegment(pointBegin->point(), pointEnd->point());
    Lane* lane = new LaneInIntersection(
        fmtId, pointBegin, pointEnd, lineSegment, _inter);
    lane->setSpeedLimit(AppMates::getGVManager().getNumeric(
        "SPEED_LIMIT_INTERSECTION"));
    _inter->addLane(lane);
}

//======================================================================
bool IntersectionBuilder::setLaneCollision()
{
    for (auto itr : _inter->lanes())
    {
        if (!_decideCollisionLane(itr.second))
        {
            return false;
        }
    }
    return true;
}

//======================================================================
bool IntersectionBuilder::_decideCollisionLane(const Lane* lane)
{
    vector<const Lane*> clInter;   // collision lanes in intersection
    vector<const Lane*> clSection; // collision lanes in section

    for (auto itr : _inter->lanes())
    {
        // イテレータが指すレーン自体か既に登録済みのレーン
        // Lane itself pointed by iterator or already registered lane
        if (itr.second == lane
            || find(clInter.begin(), clInter.end(), itr.second)
                   != clInter.end())
        {
            continue;
        }

        AmuPoint tmpPoint;
        if (lane->createIntersectionPoint(
                itr.second->lineSegment(), &tmpPoint))
        {
            // 交点があれば登録
            // Register lane if there is an intersection point
            clInter.push_back(itr.second);

            // 上流も調査する
            // Check upstream lanes
            _addUpstreamCollisionLanes(
                lane, itr.second, clInter, clSection);
        }
    }

    if (clInter.empty() && clSection.empty())
    {
        return true;
    }

    // 見つかった交錯レーンを引数で与えられたlaneに登録する
    // Register found crossing lanes with lane given in the argument
    LaneInIntersection* laneInter
        = dynamic_cast<LaneInIntersection*>(const_cast<Lane*>(lane));
    for (auto itr : clInter)
    {
        laneInter->addCollisionLanesInIntersection(itr);
    }
    for (auto itr : clSection)
    {
        laneInter->addCollisionLanesInSection(itr);
    }

    return true;
}

//======================================================================
bool IntersectionBuilder::_addUpstreamCollisionLanes(
    const Lane* lane, const Lane* collisionLane,
    vector<const Lane*>& result_clInter,
    vector<const Lane*>& result_clSection)
{
    // 調査対象のレーン
    // Lane to be checked
    vector<const Lane*> lookupLanes;

    const vector<const Lane*>& prevLanes
        = collisionLane->previousLanes();
    lookupLanes.insert(
        lookupLanes.end(), prevLanes.begin(), prevLanes.end());

    while (!lookupLanes.empty())
    {
        if (_inter->containsLane(lookupLanes[0]))
        {
            /*
             * 交差点内レーンの場合，イテレータが指すレーン自体か
             * 既に登録済みのレーンは無視する
             *
             * In the case the lane is in intersection, ignore the lane
             * itself pointed by iterator or already registered lanes
             */
            if (lookupLanes[0] == lane
                || (find(
                        result_clInter.begin(), result_clInter.end(),
                        lookupLanes[0])
                    != result_clInter.end()))
            {
                // do nothing
            }
            else
            {
                result_clInter.push_back(lookupLanes[0]);

                // さらに上流のレーンを調査する
                // Check further upstream lanes
                const vector<const Lane*>& morePrevLanes
                    = lookupLanes[0]->previousLanes();
                lookupLanes.insert(
                    lookupLanes.end(), morePrevLanes.begin(),
                    morePrevLanes.end());
            }
        }
        else
        {
            /*
             * 単路部内レーンの場合，既に登録済みのレーンは無視する
             *
             * In the case the lane is in section, ignore already
             * registered lanes 
             */
            if (find(
                    result_clSection.begin(), result_clSection.end(),
                    lookupLanes[0])
                != result_clSection.end())
            {
                // do nothing
            }
            else
            {
                result_clSection.push_back(lookupLanes[0]);

                // さらに上流のレーンまでは調査しない
                // Not check further upstream lanes
            }
        }

        // 先頭の要素の処理が終わったら削除する
        // Remove the first element after processed
        lookupLanes.erase(lookupLanes.begin());
        if (lookupLanes.empty())
        {
            break;
        }
    }
    return true;
}
