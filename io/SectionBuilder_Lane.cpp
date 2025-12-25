/* ******************************************************************
 * Copyright 2014-2024 ADVENTURE Project
 * All Rights Reserved
 * ****************************************************************** */
/**
 * @file SectionBuilder_Lane.cpp
 */
#include "SectionBuilder.hpp"
#include "../AppMates.hpp"
#include "../Config.hpp"
#include "../GVManager.hpp"
#include "../Intersection.hpp"
#include "../LaneInSection.hpp"
#include "../ObjectManager.hpp"
#include "../SubSection.hpp"
#ifdef INCLUDE_TRAMS
#include "../tram/TramLaneInSection.hpp"
#endif //INCLUDE_TRAMS
#include <AmuConverter.hpp>
#include <AmuInterval.hpp>
#include <AmuLinePoly.hpp>
#include <AmuLineSegment.hpp>
#include <AmuPoint.hpp>
#include <AmuVector.hpp>
#include <iostream>
#include <typeinfo>

using namespace std;
using namespace amu::geometry;
using namespace amu::math;
using amu::converter::formatId;
using LP = LanePosition;

//======================================================================
bool SectionBuilder::_generateLanes()
{
    for (int i = 0; i < 2; i++)
    {
        if (_section->numIn(i) == _section->numOut((i + 1) % 2))
        {
            /*
             * 流入点と流出点の数が等しい場合
             *   境界上のコネクタを単純に結ぶ
             *
             * If the numbers of inflow and outflow points are equal:
             *   Simply connect the connectors on the both borders
             */
            _generateSimpleLanes(i, 0, _section->numIn(i), 0);
        }
        else if (
            _section->numIn(i) + 1 == _section->numOut((i + 1) % 2))
        {
            /*
             * 流出点が流入点よりひとつ多い場合
             * - 内部コネクタを作って最内レーンを分岐する
             * - 左側通行の場合は最大のIDを持つ流入点，右側通行の場合は
             *   最小のIDを持つ流入点を始点とするレーンが該当する
             *
             * If there are one more outflow points than inflow points:
             * - Make an internal connector and branch innermost lane.
             * - In the case of left-hand traffic, the lane starting
             *   from the inflow point with the largest ID number is
             *   applicable, and in the case of right-hand traffic, the
             *   lane starting from the inflow point with the smallest
             *   ID number is applicable.
             */
#ifdef RIGHT_HAND_TRAFFIC
            _generateBranchedLane(i, 0, 1, LP::Left);
            _generateSimpleLanes(i, 1, _section->numIn(i), 1);
#else
            _generateSimpleLanes(i, 0, _section->numIn(i) - 1, 0);
            _generateBranchedLane(
                i, _section->numIn(i) - 1, 0, LP::Right);
#endif
        }
        else if (
            _section->numIn(i) + 2 == _section->numOut((i + 1) % 2))
        {
            /*
             * 流出点が流入点よりふたつ多い場合
             * - 最内レーンと最外レーンを分岐する
             * - 左側通行でも右側通行でも同じ処理
             *
             * If there are two more outflow points than inflow points:
             * - Branch innermost and outermost lanes with creating
             *   internal connectors.
             * - Same process for left-hand and right-hand traffic.
             */
            _generateBranchedLane(i, 0, 1, LP::Left);
            _generateSimpleLanes(i, 1, _section->numIn(i) - 1, 1);
            _generateBranchedLane(
                i, _section->numIn(i) - 1, 1, LP::Right);
        }
        else
        {
            // その他のケースは現在対応できない
            // Other cases are currently not supported
            cerr << "ERROR: " << _section->numIn(i)
                 << " inflow points and " << _section->numOut(i)
                 << " in direction of " << i << " at section["
                 << _section->id() << "] not supported." << endl;
            exit(EXIT_FAILURE);
        }
    }

#ifdef INCLUDE_TRAMS
    // 路面電車レーン
    // Tram lane
    _builderTramExt->generateTramLanes();
#endif
    return true;
}

//======================================================================
void SectionBuilder::_generateSimpleLanes(
    int dir, int nBegin, int nEnd, int offset)
{
    int           sum = _section->numIn(dir) + _section->numOut(dir);
    int           dirAnother = (dir + 1) % 2;
    Intersection* interBegin = _section->intersection(dir);
    Intersection* interEnd   = _section->intersection(dirAnother);
    const Border* borderBegin
        = interBegin->border(interBegin->direction(interEnd));
    const Border* borderEnd
        = interEnd->border(interEnd->direction(interBegin));

    for (int n = nBegin; n < nEnd; n++)
    {
        int idIntBegin = dir * 100 + sum - 1 - n;
        int idIntEnd   = dirAnother * 100 + n + offset;
        int idInt      = idIntBegin * 10000 + idIntEnd;

        const Connector* pointBegin
            = borderBegin->connector(sum - 1 - n);
        const Connector* pointEnd = borderEnd->connector(n + offset);
        assert(pointBegin && pointEnd);

        _generateLane(idInt, pointBegin, pointEnd);
    }
}

//======================================================================
void SectionBuilder::_generateBranchedLane(
    int dir, int n, int offset, LP::Type direction)
{
    int sum        = _section->numIn(dir) + _section->numOut(dir);
    int dirAnother = (dir + 1) % 2;
    int branchDir  = direction == LP::Left ? -1 : 1;

    Intersection* interBegin = _section->intersection(dir);
    Intersection* interEnd   = _section->intersection(dirAnother);
    const Border* borderBegin
        = interBegin->border(interBegin->direction(interEnd));
    const Border* borderEnd
        = interEnd->border(interEnd->direction(interBegin));

    int idIntBegin = dir * 100 + sum - 1 - n;
    int idIntEnd1  = dirAnother * 100 + n + offset;

    /*
     * 分岐したレーンの終点ID
     *   左に分岐なら1小さく，右に分岐なら1大きくなる
     *
     * End point ID number of the branched lane
     *   Decrease by 1 if branching to the left, increase by 1
     *   if branching to the right
     */
    int idIntEnd2 = dirAnother * 100 + n + offset + branchDir;

    const Connector* pointBegin = borderBegin->connector(sum - 1 - n);
    const Connector* pointEnd1  = borderEnd->connector(n + offset);
    const Connector* pointEnd2
        = borderEnd->connector(n + offset + branchDir);
    assert(pointBegin && pointEnd1 && pointEnd2);

    // 分岐点から流出点までの距離
    // Distance from branch point to outflow point
    double distanceM = _branchLaneLength[dirAnother];

    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    /*
     * 単路部の長さと対向車線の分岐の有無によって場合分けする
     *
     * Differentiate depending on the section length and whether or not
     * the opposite lane branches.
     */
    double length = pointBegin->point().distance(pointEnd1->point());
    bool   isAnotherBranched
        = (_section->numOut(dir) > _section->numIn(dirAnother));

    if (length < distanceM && !isAnotherBranched)
    {
        /*
         * [1] 短い単路部，対向分岐なし
         *       始点から分岐
         *
         * [1] Short section, no opposite branch:
         *       Branch from start point
         */
        int idInt1 = idIntBegin * 10000 + idIntEnd1;
        _generateLane(idInt1, pointBegin, pointEnd1);
        int idInt2 = idIntBegin * 10000 + idIntEnd2;
#ifdef POLYLINE_BRANCH_LANE
        _generatePolylineBranchLane(
            idInt2, pointBegin, pointEnd2, pointEnd1);
#else //POLYLINE_BRANCH_LANE not defined
        _generateLane(idInt2, pointBegin, pointEnd2);
#endif
        return;
    }

    if (isAnotherBranched)
    {
        /*
         * [2] 短い単路部，対向分岐あり
         *       中点から分岐
         *
         * [2] Short section, opposite branch exists:
         *       Branch from midpoint
         *
         * [4] 長い単路部，対向分岐あり
         *       中点より下流で分岐
         *
         * [4] Long section, opposite branch exists:
         *       Branch downstream from midpoint 
         */
        distanceM = min(distanceM, length / 2);
    }
    /*
     * [3] 長い単路部，対向分岐なし
     *       指定した距離で分岐
     *
     * [3] Long section, no opposite branch:
     *       Branch at given distance
     */
    int              idIntM;
    const Connector* pointM = _createInternalConnector(
        pointBegin->point(), pointEnd1->point(), distanceM, &idIntM);
    int idInt0 = idIntBegin * 10000 + idIntM;
    _generateLane(idInt0, pointBegin, pointM);
    int idInt1 = idIntM * 10000 + idIntEnd1;
    _generateLane(idInt1, pointM, pointEnd1);
    int idInt2 = idIntM * 10000 + idIntEnd2;
#ifdef POLYLINE_BRANCH_LANE
    _generatePolylineBranchLane(idInt2, pointM, pointEnd2, pointEnd1);
#else //POLYLINE_BRANCH_LANE not defined
    _generateLane(idInt2, pointM, pointEnd2);
#endif
}

//======================================================================
void SectionBuilder::_generateLane(
    int idInt, const Connector* pointBegin, const Connector* pointEnd)
{
    string id = formatId(to_string(idInt), NUM_FIGURE_FOR_LANE);
    AmuLineSegment* lineSegment
        = new AmuLineSegment(pointBegin->point(), pointEnd->point());
    Lane* lane = new LaneInSection(
        id, pointBegin, pointEnd, lineSegment, _section);
    lane->setSpeedLimit(
        AppMates::getGVManager().getNumeric("SPEED_LIMIT_SECTION"));
    _section->addLane(lane);
}

//======================================================================
void SectionBuilder::_generatePolylineBranchLane(
    int idInt, const Connector* pointBegin, const Connector* pointEnd,
    const Connector* nextEndPoint)
{
    string    id = formatId(to_string(idInt), NUM_FIGURE_FOR_LANE);
    AmuVector dv
        = AmuVector(pointBegin->point(), nextEndPoint->point());
    dv.normalize();
    AmuVector nv = AmuVector(pointEnd->point(), nextEndPoint->point());
    double    length = 2 * _laneWidth;
    if (pointBegin->point().distance(pointEnd->point()) < length - 2)
    {
        length = pointBegin->point().distance(pointEnd->point()) - 2;
    }
    AmuPoint mp = pointBegin->point() + length * dv - nv;

    AmuLinePoly* lineSegment
        = new AmuLinePoly(pointBegin->point(), pointEnd->point());
    lineSegment->addPoint(mp);
    lineSegment->createInternalLines();
    Lane* lane = new LaneInSection(
        id, pointBegin, pointEnd, lineSegment, _section);
    lane->setSpeedLimit(
        AppMates::getGVManager().getNumeric("SPEED_LIMIT_SECTION"));
    _section->addLane(lane);
}

//======================================================================
const Connector* SectionBuilder::_createInternalConnector(
    const AmuPoint& begin, const AmuPoint& end, double distance,
    int* result_id)
{
    const AmuLineSegment line = AmuLineSegment(begin, end);
    const AmuPoint       internalPoint
        = line.createInteriorPoint(line.length() - distance, distance);

    // 9000は内部コネクタ用のprefixに相当
    // 9000 corresponds to the prefix for internal connectors
    (*result_id) = 9000 + _section->numInternalConnectors();
    Connector* result_connector
        = AppMates::getObjectManager().createConnector(
            internalPoint.x(), internalPoint.y(), internalPoint.z());
    _section->addInternalConnector(
        formatId(to_string(*result_id), NUM_FIGURE_FOR_CONNECTOR_LOCAL),
        result_connector);

    return result_connector;
}

//======================================================================
bool SectionBuilder::setLaneConnection()
{
    for (auto itr : _bundle->lanes())
    {
        Lane* lane = itr.second;
        if (!(_decideNextLanes(lane)) || !(_decidePrevLanes(lane))
            || !(_decideLeftLane(lane)) || !(_decideRightLane(lane)))
        {
            return false;
        }
    }
    return true;
}

//======================================================================
bool SectionBuilder::_decideLeftLane(Lane* lane)
{
    /*
     * 始点が互いにずれている場合に相手からこのレーンが見えないことが
     * あるので，相手が見つかった時点で相手にもこのレーンを登録する．
     * mapへの登録で重複は避けられる
     *
     * If the starting points are shifted from each other, this lane may
     * not be detected by the other lane, so register this lane to the
     * other lane if the other lane is found. Duplication can be avoided
     * by registering to map.
     */
    Lane* leftLane = _calcSideLane(lane, LP::Left);
    if (leftLane)
    {
        if (!(lane->isSideLaneFound(leftLane, LP::Left)))
        {
            _setSideLane(lane, leftLane, LP::Left);
        }
        if (!(leftLane->isSideLaneFound(lane, LP::Right)))
        {
            _setSideLane(leftLane, lane, LP::Right);
        }
    }
    return true;
}

//======================================================================
bool SectionBuilder::_decideRightLane(Lane* lane)
{
    /*
     * 始点が互いにずれている場合に相手からこのレーンが見えないことが
     * あるので，相手が見つかった時点で相手にもこのレーンを登録する．
     * mapへの登録で重複は避けられる
     *
     * If the starting points are shifted from each other, this lane may
     * not be detected by the other lane, so register this lane to the
     * other lane if the other lane is found. Duplication can be avoided
     * by registering to map.
     */
    Lane* rightLane = _calcSideLane(lane, LP::Right);
    if (rightLane)
    {
        if (!(lane->isSideLaneFound(rightLane, LP::Right)))
        {
            _setSideLane(lane, rightLane, LP::Right);
        }
        if (!(rightLane->isSideLaneFound(lane, LP::Left)))
        {
            _setSideLane(rightLane, lane, LP::Left);
        }
    }
    return true;
}

//======================================================================
Lane* SectionBuilder::_calcSideLane(
    Lane* lane, LP::Type direction) const
{
    if (direction != LP::Left && direction != LP::Right)
    {
        cerr << "ERROR: SectionBuilder::_calcSideLane"
             << " - bad direction specified." << endl;
        exit(EXIT_FAILURE);
    }

    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    // 探索用ベクトルの作成
    // Create search vector
    AmuVector searchVector = lane->directionVector();
    searchVector.normalize();

    /*
     * 始点を共有する車線を除外するため，探索用ベクトルの始点を
     * _beginConnector から少しだけずらす
     *
     * Shift start point of search vector slightly from _beginConnector
     * to exclude lanes that share the start point
     */
    AmuPoint beginPoint
        = lane->beginConnector()->point() + searchVector * 1.0e-1;
    if (direction == LP::Left)
    {
        searchVector.revoltXY(M_PI_2);
    }
    else
    {
        searchVector.revoltXY(-M_PI_2);
    }
    const AmuLineSegment searchLine
        = AmuLineSegment(
              beginPoint,
              beginPoint + searchVector * SEARCH_SIDE_LANE_LINE_LENGTH)
              .z0();

    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    /*
     * 候補レーンの集合を求める
     *   引数で与えられたlane自身を除き，laneと同じ方向
     *
     * Find candidate lanes
     *   Lanes with same direction as the lane given as the argument,
     *   except for the given lane.
     */
    vector<Lane*> candidateLanes;
    bool          isThisUp = _section->isUp(lane);

    for (auto itr : _section->lanes())
    {
        if (itr.second == lane)
        {
            continue;
        }
        if (_section->isUp(itr.second) == isThisUp)
        {
            candidateLanes.push_back(itr.second);
        }
    }

    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    /*
     * 探索ベクトルと候補レーンとの交点を求め，引数で与えられたlaneから
     * もっとも近いものを決定
     *
     * Find the intersection points of the search vector and candidate
     * lanes, and determine the closest from the given lane.
     */
    double minDistance = INFINITY;
    Lane*  resultLane  = NULL;
    for (unsigned int i = 0; i < candidateLanes.size(); i++)
    {
        AmuPoint crsPoint;

        if (candidateLanes[i]
                ->lineSegment()
                ->z0()
                .createIntersectionPoint(&searchLine, &crsPoint))
        {
            double distance = beginPoint.distance(crsPoint);
            if (distance < minDistance)
            {
                resultLane  = candidateLanes[i];
                minDistance = distance;
            }
        }
    }
    return resultLane;
}

//======================================================================
void SectionBuilder::_setSideLane(
    Lane* lane, Lane* anotherLane, LP::Type direction)
{
#ifdef INCLUDE_TRAMS
    // 路面電車レーンは除外
    // Exclude tram lane
    if (dynamic_cast<TramLaneInSection*>(anotherLane))
    {
        return;
    }
#endif //INCLUDE_TRAMS

    if (direction != LP::Left && direction != LP::Right)
    {
        cerr << "ERROR: SectionBuilder::_calcSideLane"
             << " - bad direction specified." << endl;
        exit(EXIT_FAILURE);
    }

    // 区間の始点
    // Start of interval
    double intervalBegin
        = lane->lineSegment()->calcFootOfPerpendicularLength(
            anotherLane->beginConnector()->point());
    if (intervalBegin < 0.0)
    {
        intervalBegin = 0.0;
    }

    // 区間の終点
    // End of interval
    double intervalEnd
        = lane->lineSegment()->calcFootOfPerpendicularLength(
            anotherLane->endConnector()->point());
    if (intervalEnd > lane->length())
    {
        intervalEnd = lane->length();
    }

    lane->addSideLane(
        AmuInterval(intervalBegin, intervalEnd), anotherLane,
        direction);
}
