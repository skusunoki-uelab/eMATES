/* **************************************************************************
 * Copyright 2014-2024 ADVENTURE Project
 * All Rights Reserved
 * ************************************************************************** */
/**
 * @file LocalLaneRouter.cpp
 */
#include "LocalLaneRouter.hpp"
#include "CSNodeFast.hpp" // [eMATES]
#include "CSNodeNormal.hpp" // [eMATES]
#include "CustomMessage.hpp"
#include "Lane.hpp"
#include "LaneInSection.hpp"
#include "RelativeDirection.hpp"
#include "ObjectInLane.hpp"
#include "Vehicle.hpp"
#include "VehicleLocation.hpp"
#include "Signal.hpp"
#include "RandomNumberGenerator.hpp"
#include <algorithm>
#include <deque>
#include <cfloat>
#include <iterator>
#include <typeinfo>
#include <unordered_map>
#ifdef INCLUDE_TRAMS
#include "tram/VehicleTram.hpp"
#include "tram/TramLaneInIntersection.hpp"
#include "tram/TramLaneInSection.hpp"
#endif //INCLUDE_TRAMS

//#define DEBUG_LOCALROUTER

using namespace std;
using LP = LanePosition;

//==============================================================================
void LocalLaneRouter::setVehicle(
    Vehicle* vehicle, VehicleGlobalRoute* globalRoute,
    VehicleLocalRoute* localRoute)
{
    _vehicle     = vehicle;
    _globalRoute = globalRoute;
    _localRoute  = localRoute;
}

//==============================================================================
void LocalLaneRouter::localReroute(
    const Section* section, const Lane* lane, const double distance)
{
    assert(section->containsLane(lane));

    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    // 進行方向の交差点と転回方向
    // Intersections ahead and direction of turning
    const Intersection* frontIntersection = nullptr;
    const Intersection* nextIntersection  = nullptr;
    int                 frontDirection    = -1;
    _getFrontIntersectionAndTurning(
        section, lane, &frontIntersection, &nextIntersection, &frontDirection);

    // 経由する単路
    // Section(s) to pass
    vector<const Section*> ways;
    ways.push_back(section);

    // ゴールとなるレーン
    // Goal lanes
    vector<const Lane*> goalLanes;
    _decideGoalLanes(section, frontIntersection, frontDirection, goalLanes);

    if (goalLanes.empty())
    {
        ostringstream sse;
        sse << "Vehicle[" << _vehicle->id() << "] in Lane[" << lane->id()
            << "(@" << section->id() << ") could not decide goal lanes."
            << endl;
        sse << "\tfrontIntersection: " << frontIntersection->id();
        if (nextIntersection)
        {
            sse << ", nextIntersection: " << nextIntersection->id();
        }
        sse << endl << "\tways: ";
        for (auto itr : ways)
        {
            sse << itr->id() << " ";
        }
        sse << endl;
        amu::msg::error(sse.str());
        exit(EXIT_FAILURE);
    }

    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    // 探索
    // Routing
    _isRouteConsistentFarLane[LP::Left]  = false;
    _isRouteConsistentFarLane[LP::Right] = false;

    // 周囲のレーンの探索
    // Search peripheral lanes
    _searchPeripheralLanes(
        lane, distance, frontIntersection, nextIntersection, ways, goalLanes);
#ifdef INCLUDE_TRAMS
    // 路面電車の場合は以下の処理は必要ない
    // In the case of trams, the following processing is not needed.
    if (typeid(*_vehicle) == typeid(VehicleTram))
    {
        return;
    }
#endif //INCLUDE_TRAMS

    bool hasFoundValidLocalRoute = _isRouteConsistentLane[LP::Left]
        || _isRouteConsistentLane[LP::Center]
        || _isRouteConsistentLane[LP::Right]
        || _isRouteConsistentFarLane[LP::Left]
        || _isRouteConsistentFarLane[LP::Right];

    /*
     * 経路が見つからなかった場合，ひとまず目前の交差点を目的の方向に通過する
     * ことだけをめざす
     *
     * If a valid local route still cannot be found, just aim to pass the
     * intersection in front of the vehicle in the desired direction.
     */
    if (!hasFoundValidLocalRoute)
    {
        const Section* nextSection = nullptr;
        if (nextIntersection)
        {
            nextSection = frontIntersection->nextSection(nextIntersection);
        }
        if (nextSection)
        {
            vector<const Section*> shortWays;
            shortWays.push_back(section);
            shortWays.push_back(nextSection);
            vector<const Lane*> exitLanes
                = nextSection->lanesFrom(frontIntersection);
            _searchPeripheralLanes(
                lane, distance, frontIntersection, nextIntersection, shortWays,
                exitLanes);
        }
    }

    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    // 車線変更の必要回数を求める
    // Find the number of lane-changes required
    _calcNumLaneChangeRequired();

    /*
     * 対象レーンの効用を計算し車線変更先を決定する
     *
     * Calculate the utilities of the candidate lanes and decide target lane to
     * change to
     */
    decideTargetLane(section, lane, distance);
}

//==============================================================================
void LocalLaneRouter::decideTargetLane(
    const Section* section, const Lane* lane, const double distance)
{
    // それぞれの車線 (Left, Center, Right) の効用を求める
    // calculate utility for each lane (Left, Center, Right)
    _calcUtilities(section, lane, distance);

    // 効用の最大値とその車線を求める
    // Find the maximum utility and it lane
    int    maxDirection = LP::Center;
    double maxUtility   = 0.0;

    for (unsigned int i = 0; i < 3; i++)
    {
        if (i == LP::Center || !_targetLanes[i])
        {
            continue;
        }
        if (_utilities[i] > maxUtility)
        {
            maxDirection = i;
            maxUtility   = _utilities[i];
        }
    }

    _localRoute->setTargetDirection(static_cast<LP::Type>(maxDirection));

    _localRoute->setTargetLane(_targetLanes[maxDirection]);
    _localRoute->setTargetUtility(min(maxUtility, 1.0));
}

//==============================================================================
void LocalLaneRouter::_getFrontIntersectionAndTurning(
    const Section* section, const Lane* lane,
    const Intersection** result_frontIntersection,
    const Intersection** result_nextIntersection, int* result_frontDirection)
{
    // 背後の交差点
    // Intersection behind
    const Intersection* rearIntersection
        = section->intersection(!section->isUp(lane));
    assert(rearIntersection);

    // 目前の交差点
    // Intersection in front
    *result_frontIntersection = section->intersection(section->isUp(lane));
    assert(result_frontIntersection);

    // 目前の交差点の次に目指す交差点
    // The intersection aimed for after the intersection in front
    *result_nextIntersection = nullptr;

    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    /*
     * frontIntersectionがODノードの場合，それ以上前方の交差点は存在しない．
     * nextIntersectionはnullptrのままでよい．
     *
     * In the case that frontIntersection is an OD node, there are no further
     * intersections ahead. nextIntersection can be left as nullptr.
     */
    if (typeid(**result_frontIntersection) == typeid(ODNode)
        || typeid(**result_frontIntersection) == typeid(CSNodeFast) // [eMATES]
        || typeid(**result_frontIntersection) == typeid(CSNodeNormal)) // [eMATES]
    {
        *result_frontDirection = 0;
        return;
    }

    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    // frontIntersectionがODノード以外の場合
    // In the case that frontIntersection is not an OD node

    // 大域的経路に沿った正当なnextIntersection
    // Proper next intersection along the global route
    *result_nextIntersection
        = _globalRoute->next(rearIntersection, *result_frontIntersection);

    /*
     * 事前の大域的経路探索に失敗しているとnextIntersectionを決められない．
     * 規制を守りながら到達可能な交差点をnextIntersectionとしてひとまず登録する．
     *
     * In the case the previous route search has failed, nextIntersection
     * cannot be determined. The intersection that can be reached while keeping
     * regulations is temporarily registered as nextIntersection.
     */
    if (!(*result_nextIntersection))
    {
        *result_nextIntersection = _getAlternativeNextInter(
            rearIntersection, *result_frontIntersection, true);
    }

    // それでもnextIntersectionが決まらない場合は規制を無視する
    // If the next intersection is still not determined, ignore regulations
    if (!(*result_nextIntersection))
    {
        ostringstream ssw;
        ssw << "Vehicle[" << _vehicle->id() << "] violates traffic regulation"
            << " around intersection[" << (*result_frontIntersection)->id()
            << "]" << endl;
        amu::msg::warn(ssw.str());
        *result_nextIntersection = _getAlternativeNextInter(
            rearIntersection, *result_frontIntersection, false);
    }

    *result_frontDirection
        = (*result_frontIntersection)->direction(*result_nextIntersection);
}

//==============================================================================
const Intersection* LocalLaneRouter::_getAlternativeNextInter(
    const Intersection* rearIntersection, const Intersection* frontIntersection,
    bool keepsRegulation) const
{
    vector<const Intersection*> candidates;

    /*
     * 直進，左折，右折，その他の転回の順で優先される
     *
     * Priority is given in the following order: going straight, turning left,
     * turning right, others.
     */
    if (frontIntersection->nextStraight(rearIntersection))
    {
        candidates.emplace_back(
            frontIntersection->nextStraight(rearIntersection));
    }
    if (frontIntersection->nextLeft(rearIntersection))
    {
        candidates.emplace_back(frontIntersection->nextLeft(rearIntersection));
    }
    if (frontIntersection->nextRight(rearIntersection))
    {
        candidates.emplace_back(frontIntersection->nextRight(rearIntersection));
    }
    if (frontIntersection->nextAnother(rearIntersection))
    {
        candidates.emplace_back(
            frontIntersection->nextAnother(rearIntersection));
    }
    for (auto itr : candidates)
    {
        const Section* nextSection = frontIntersection->nextSection(itr);
        bool           isUp        = nextSection->isUp(frontIntersection, itr);

        if (!keepsRegulation
            || (frontIntersection->permitsPassing(
                    rearIntersection, itr, *(_vehicle->body()->type()))
                && nextSection->permitsPassing(
                    isUp, *(_vehicle->body()->type()))))
        {
            return itr;
        }
    }

    return nullptr;
}

//==============================================================================
bool LocalLaneRouter::_decideGoalLanes(
    const Section* section, const Intersection* frontIntersection,
    int frontDirection, vector<const Lane*>& result_goalLanes)
{
    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    // 目前がODノードである場合
    // In the case that the front is an ODNode
    if (typeid(*frontIntersection) == typeid(ODNode)
        || typeid(*frontIntersection) == typeid(CSNodeFast) // [eAMTES]
        || typeid(*frontIntersection) == typeid(CSNodeNormal)) // [eMATES]
    {
        vector<const Lane*> lanes = frontIntersection->lanesFrom(0);
        result_goalLanes.swap(lanes);
        return !(result_goalLanes.empty());
    }

    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    /*
     * 車線変更が可能な単路部に流入するための farGoalLanes の決定
     *
     * Determine farGoalLanes for flowing into a section where lane change is
     * executable
     */
    const Intersection* prevIntersection = frontIntersection;
    ASSERT_MSG(frontDirection != -1);

    // prevIntersectionからnextIntersectionへの単路部
    // Section from prevIntersection to nextIntersection
    const Section* nextSection = frontIntersection->nextSection(frontDirection);

    // nextSectionの次に通過することになる単路部
    // Section to be passed after nextIntersection
    const Intersection* nextIntersection
        = nextSection->anotherIntersection(prevIntersection);

    // farGoalLanesまでに通過すべき単路部の列
    // Section sequence to be passed to farGoalLanes
    vector<const Section*> farWays;
    farWays.push_back(nextSection);

    // 車線変更可能な単路部に到達するまで探索する
    // Search until reaching section where lane-change is executable
    while (true)
    {
        if (nextSection->canAcceptLaneShift(prevIntersection))
        {
            break;
        }
        else if ((nextSection->lanesFrom(prevIntersection)).size() == 1)
        {
            // 1車線しかないので車線変更は必要ない
            // Only one lane, so no lane-change needed
            break;
        }
        else if (typeid(*nextIntersection) == typeid(ODNode)
            || typeid(*nextIntersection) == typeid(CSNodeFast) // [eAMTES]
            || typeid(*nextIntersection) == typeid(CSNodeNormal)) // [eMATES]
        {
            // 次の単路部はないので探索できない
            // Cannot search because there is no next section
            break;
        }

        // 次の単路部に探索を進める
        // Advance the search to the next section
        const Intersection* oldIntersection = prevIntersection;
        prevIntersection                    = nextIntersection;
        nextIntersection
            = _globalRoute->next(oldIntersection, prevIntersection);
        if (!nextIntersection)
        {
            // 大域経路探索に失敗していた場合
            // In the case the global routing has failed
            prevIntersection = oldIntersection;
            break;
        }

        nextSection = prevIntersection->nextSection(nextIntersection);
        farWays.push_back(nextSection);
    }

    vector<const Lane*> farGoalLanes = nextSection->lanesFrom(prevIntersection);

    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    // farGoalLanes から逆順に辿り goalLanes を決定する
    // Determine goalLanes by tracing backwards from farGoalLanes
    _bfReverseSearchLocalRoute(
        farGoalLanes, farWays, frontIntersection, result_goalLanes);

    return !(result_goalLanes.empty());
}

//==============================================================================
void LocalLaneRouter::_searchPeripheralLanes(
    const Lane* lane, double distance, const Intersection* frontIntersection,
    const Intersection* nextIntersection, vector<const Section*>& ways,
    vector<const Lane*>& goalLanes)
{
    vector<const Lane*> lanes;

    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    // 中央レーンの探索
    // Searching center lane
    _targetLanes[LP::Center]           = lane;
    _isRouteConsistentLane[LP::Center] = _getLocalLaneRoute(
        lane, frontIntersection, nextIntersection, ways, goalLanes, lanes);

    // 中央レーンの探索が失敗した場合は代替経路を設定
    // Set alternate route if fail to search the center lane
    if (lanes.empty())
    {
        _getAltLocalRoute(lane, frontIntersection, ways, lanes);
    }

    // 中央レーンの探索結果のみ記録
    // Record only the search result of the center lane
    _localRoute->clearLocalRoute();
    _localRoute->clearLanesInIntersection();
    _localRoute->setMainLaneInIntersection(nullptr);
    _localRoute->setTurning(RD::NONE);
    _localRoute->setLocalRoute(lanes);
    _setIntersectionLanes(lanes, frontIntersection);

    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    // 左レーンの探索
    // Searching left lane
    _searchSideLanes(
        LP::Left, lane, distance, frontIntersection, nextIntersection, ways,
        goalLanes);

    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    // 右レーンの探索
    // Searching right lane
    _searchSideLanes(
        LP::Right, lane, distance, frontIntersection, nextIntersection, ways,
        goalLanes);
}

//==============================================================================
void LocalLaneRouter::_searchSideLanes(
    LP::Type direction, const Lane* lane, double distance,
    const Intersection* frontIntersection, const Intersection* nextIntersection,
    vector<const Section*>& ways, vector<const Lane*>& goalLanes)
{

    _targetLanes[direction] = lane->sideLane(direction, distance);
    if (!_targetLanes[direction])
    {
        return;
    }

    // 到達可能性の更新
    // Update reachability
    vector<const Lane*> lanes;
    _isRouteConsistentLane[direction] = _getLocalLaneRoute(
        _targetLanes[direction], frontIntersection, nextIntersection, ways,
        goalLanes, lanes);

    /*
     * 中央レーンから到達可能であれば左右のレーンの処理は不要
     *
     * If reachable from the center lane, processing left and right lanes is not
     * required.
     */
    if (_isRouteConsistentLane[LP::Center])
    {
        return;
    }

    /*
     * 左レーンから到達可能であれば右レーンの処理は不要
     *
     * If reachable from the center or left lane, processing right lane is not
     * required.
     */
    if (direction == LP::Right && _isRouteConsistentLane[LP::Left])
    {
        return;
    }

    // 希望経路が見つかれば探索結果を格納する
    // Store the search result if the desired route is found
    if (!(lanes.empty()))
    {
        _desiredLocalRoute.resize(lanes.size());
        copy(lanes.begin(), lanes.end(), _desiredLocalRoute.begin());
        return;
    }

    /*
     * 希望経路が見つからなければさらに隣のレーンを探索する
     *   車線変更完了後の再探索で希望経路を格納するため，この時点では探索結果を
     *   格納する必要はなく，到達可能性を記録するだけでよい
     *
     * If the desired route not found, further search the adjacent lane
     *   Since the desired route is stored in the re-search after the lane-
     *   change is complete, there is no need to store the search result at this
     *   point, just record the reachability.
     */
    const Lane* targetLane
        = _targetLanes[direction]->sideLane(direction, distance);
    while (targetLane && lanes.empty())
    {
        _isRouteConsistentFarLane[direction] = _getLocalLaneRoute(
            targetLane, frontIntersection, nextIntersection, ways, goalLanes,
            lanes);
        targetLane = targetLane->sideLane(direction, distance);
    }
}

//==============================================================================
void LocalLaneRouter::_calcNumLaneChangeRequired()
{
    if (_isRouteConsistentLane[LP::Left])
    {
        _numLaneChangeRequired[LP::Left] = 0;
        _numLaneChangeRequired[LP::Center]
            = _isRouteConsistentLane[LP::Center] ? 0 : 1;
        _numLaneChangeRequired[LP::Right] = _isRouteConsistentLane[LP::Right]
            ? 0
            : (_numLaneChangeRequired[LP::Center] + 1);
    }
    else if (_isRouteConsistentLane[LP::Center])
    {
        _numLaneChangeRequired[LP::Center] = 0;
        _numLaneChangeRequired[LP::Right]
            = _isRouteConsistentLane[LP::Right] ? 0 : 1;
        _numLaneChangeRequired[LP::Left]
            = _isRouteConsistentLane[LP::Left] ? 0 : 1;
    }
    else if (_isRouteConsistentLane[LP::Right])
    {
        _numLaneChangeRequired[LP::Right] = 0;
        _numLaneChangeRequired[LP::Center]
            = _isRouteConsistentLane[LP::Center] ? 0 : 1;
        _numLaneChangeRequired[LP::Left] = _isRouteConsistentLane[LP::Left]
            ? 0
            : (_numLaneChangeRequired[LP::Center] + 1);
    }
    else
    {
        // 1回以内の車線変更では大域的経路を満足できない場合
        // In the case the global route cannot be satisfied within a lane-change
        if (_isRouteConsistentFarLane[LP::Left])
        {
            _numLaneChangeRequired[LP::Left]   = 1;
            _numLaneChangeRequired[LP::Center] = 2;
            _numLaneChangeRequired[LP::Right]  = 3;
        }
        else if (_isRouteConsistentFarLane[LP::Right])
        {
            _numLaneChangeRequired[LP::Left]   = 3;
            _numLaneChangeRequired[LP::Center] = 2;
            _numLaneChangeRequired[LP::Right]  = 1;
        }
    }
}

//==============================================================================
void LocalLaneRouter::_calcUtilities(
    const Section* section, const Lane* lane, double distance)
{
    // インセンティブの計算
    // Calculate incentives
    _calcRouteIncentives(section, lane, distance);
    // _calcSpeedIncentives(section, lane, distance);

    // 効用の更新
    // Update utility
    for (unsigned int i = 0; i < 3; i++)
    {
        if (!_targetLanes[i] || i == LP::Center)
        {
            _utilities[i] = 0.0;
            continue;
        }
        _utilities[i] = _routeIncentives[i]; // + _speedIncentives[i];
    }
}

//==============================================================================
void LocalLaneRouter::_calcRouteIncentives(
    const Section* section, const Lane* lane, double distance)
{
    double distanceToNext = section->distanceToNext(lane, distance);

    //------------------------------------------------------------------
    // レーンから離脱する願望
    // Desire to leave lane
    double dr[3];
    for (unsigned int i = 0; i < 3; i++)
    {
        if (_numLaneChangeRequired[i] == 0)
        {
            dr[i] = 0;
        }
        else if (!_targetLanes[i])
        {
            dr[i] = 1;
        }
        else
        {
            double timeToNext
                = distanceToNext / (_vehicle->velocity() * 1000); // [m/s]

            dr[i] = max(
                1
                    - (distanceToNext
                       / (_numLaneChangeRequired[i] * ANTICIPATION_DISTANCE)),
                1
                    - (timeToNext
                       / (_numLaneChangeRequired[i] * ANTICIPATION_TIME)));
            if (dr[i] < 0)
            {
                dr[i] = 0;
            }
        }
    }

    //------------------------------------------------------------------
    // それぞれのレーンの経路インセンティブを求める
    // Calculate route incentive for each lane
    for (unsigned int i = 0; i < 3; i++)
    {
        if (i == LP::Center)
        {
            _routeIncentives[i] = 0;
            continue;
        }
        if (_numLaneChangeRequired[i] < _numLaneChangeRequired[LP::Center])
        /* _numLaneChangeRequired[i] == 0 in the paper */
        {
            if (dr[LP::Center] > dr[i])
            {
                _routeIncentives[i] = dr[LP::Center];
            }
            else if (fabs(dr[i] - dr[1]) < 1.0e-6)
            {
                _routeIncentives[i] = 0;
            }
            else
            {
                _routeIncentives[i] = -dr[i];
            }
        }
        else
        {
            _routeIncentives[i] = -dr[i]; // -DBL_MAX in the paper
        }
    }
}

//==============================================================================
void LocalLaneRouter::_calcSpeedIncentives(
    const Section* section, const Lane* lane, double distance)
{
    double vAnt[3];

    for (unsigned int i = 0; i < 3; i++)
    {
        vAnt[i]             = 0;
        _speedIncentives[i] = 0.0;

        if (!_targetLanes[i])
        {
            continue;
        }

        // 先行車を取得する
        // Get the preceding vehicle
        const ObjectInLane* agent;
        double              gap;
        _targetLanes[i]->getFrontAgentFar(
            distance + 1.0e-3, ANTICIPATION_DISTANCE, &agent, &gap);
        const Vehicle* front
            = dynamic_cast<Vehicle*>(const_cast<ObjectInLane*>(agent));

        if (front && front->behavior()->isNotifying())
        {
            // 先行車両が車線変更中は速度インセンティブを求めない
            // Not calculate speed incentives
            for (unsigned int j = 0; j < 3; j++)
            {
                _speedIncentives[j] = 0.0;
            }
            return;
        }

        double distanceToNext = section->distanceToNext(lane, distance);
        if (distanceToNext < _vehicle->body()->bodyLength() * 5
            && _routeIncentives[i] < 0)
        {
            /*
             * 交差点に近ければ大域的経路を外れる車線変更は行わない
             *
             * Not change lanes that deviate from the global route if the
             * vehicle is close to the next intersection
             */
            _speedIncentives[i] = 0.0;
            continue;
        }

        // 希望速度
        // Desired speed
        double vDes = _targetLanes[i]->speedLimit() / 3.6; // [km/h]->[m/sec]

        // 予想速度を求める
        // Calculate anticipation speed
        if (!front)
        {
            vAnt[i] = vDes;
        }
        else
        {
            vAnt[i] = ((1 - gap / ANTICIPATION_DISTANCE) * front->velocity()
                       * 1000) // [m/msec]->[m/sec]
                + (gap / ANTICIPATION_DISTANCE * vDes);
        }
    }

    for (unsigned int i = 0; i < 3; i++)
    {
        if (i == LP::Center)
        {
            _speedIncentives[i] = 0;
        }
        else
        {
            _speedIncentives[i]
                = (vAnt[i] - vAnt[LP::Center] - 1) / VELOCITY_GAIN;
        }
    }
}

//==============================================================================
bool LocalLaneRouter::_getLocalLaneRoute(
    const Lane* startLane, const Intersection* frontIntersection,
    const Intersection* nextIntersection, vector<const Section*>& ways,
    vector<const Lane*>& goalLanes, vector<const Lane*>& result_lanes)
{

    vector<const Lane*> foundLanes;
    foundLanes.clear();
    const Section* nextSection = nullptr;
    if (nextIntersection)
    {
        nextSection = frontIntersection->nextSection(nextIntersection);
    }
    bool isNextSectionIncluded = false;

    // 探索
    // Searching
    _dfSearchLocalRoute(
        startLane, ways, nextSection, goalLanes, foundLanes,
        isNextSectionIncluded);

    if (!(foundLanes.empty()))
    {
        assert(foundLanes.front() == startLane);
        result_lanes.swap(foundLanes);
        return true;
    }

    return false;
}

//==============================================================================
bool LocalLaneRouter::_getAltLocalRoute(
    const Lane* lane, const Intersection*, std::vector<const Section*>& ways,
    std::vector<const Lane*>& result_lanes)
{
    result_lanes.clear();
    result_lanes.push_back(lane);

    // nextStraightLane を次々に追加する
    // Add nextStraightLane one after another
    const Lane* nextLane = lane;
    while (true)
    {
        nextLane = nextLane->nextStraightLane();
        if (!nextLane)
        {
            // TramLane の場合に nextLane が NULL となりうる
            // nextLane can be NULL for TramLane
            break;
        }
        result_lanes.push_back(nextLane);

        // nextLane が ways から外れたら終了
        // Exit when nextLane is out of ways
        auto& parent = *(nextLane->parent());
        if ((typeid(parent) == typeid(Section)
             && find(ways.begin(), ways.end(), nextLane->parent())
                 == ways.end())
            || typeid(parent) == typeid(ODNode)
            || typeid(parent) == typeid(CSNodeFast) // [eAMTES]
            || typeid(parent) == typeid(CSNodeNormal)) // [eMATES]
        {
            break;
        }
    }

    return true;
}

//==============================================================================
bool LocalLaneRouter::_dfSearchLocalRoute(
    const Lane* startLane, vector<const Section*>& ways,
    const Section* nextSection, vector<const Lane*>& goalLanes,
    vector<const Lane*>& result_lanes, bool& result_isNextSectionIncluded)
{
    // 探索状況を保持するスタック
    // Stack that holds the search status
    vector<const Lane*> searchStack;
    searchStack.clear();
    searchStack.emplace_back(startLane);

    // 探索済みの要素
    // Already visited element
    vector<const Lane*> visited;
    visited.clear();

    /*
     * レーンとその上流のレーンのペア <下流, 上流> を格納するテーブル．合流を
     * 扱う必要．
     *
     * Table that stores pairs of lanes and their upstream lanes <downstream,
     * upstream>. Necessary to deal with confluence.
     */
    unordered_map<const Lane*, const Lane*> lanePairs;
    lanePairs.clear();

    // goalLanes の代替候補
    // Alternative candidates of goalLanes
    const Lane* altGoalLane     = nullptr;
    double      altGoalDistance = DBL_MAX;

    while (true)
    {
        //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
        // 探索終了
        // End of search
        if (searchStack.empty())
        {
            break;
        }

        //+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
        // 探索の先頭が成功条件を満たすか判定
        // Judge whether the head satisfies the condition of success
        const Lane* headLane = searchStack.back();

        if (!result_isNextSectionIncluded && headLane->parent() == nextSection)
        {
            result_isNextSectionIncluded = true;
        }
        if (find(goalLanes.begin(), goalLanes.end(), headLane)
            != goalLanes.end())
        {
            /*
             * 探索成功．探索結果を逆順に辿り，最後に反転する．
             *
             * Search succeeded. Traverse the search results in reverse order,
             * and then reverse at the end.
             */
            const Lane* downstream = headLane;
            while (true)
            {
                result_lanes.push_back(downstream);
                if (downstream == startLane)
                {
                    break;
                }
                else
                {
                    downstream = lanePairs[downstream];
                    continue;
                }
            }
            reverse(result_lanes.begin(), result_lanes.end());

            return true;
        }

        //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
        /*
         * 探索の先頭が失敗条件を満たすか判定
         *   正しい単路部の次の交差点はかならず正しいため，先頭が単路部にある
         *   場合のみ判定する
         *
         * Judge whether the head satisfies the condition of failure
         *   Since the intersection next to the correct section is always
         *   correct, judge only when the head at a section.
         */
        if (typeid(*(headLane->parent())) == typeid(Section)
            && find(ways.begin(), ways.end(), headLane->parent()) == ways.end())
        {
            // 代替ゴールとなりうる車線の保存
            // Save lane that can be an alternate goal
            if (headLane->parent() == goalLanes[0]->parent())
            {
                double distance = DBL_MAX;
                for (auto itr : goalLanes)
                {
                    distance = min(
                        distance,
                        headLane->lineSegment()->distance(
                            itr->lineSegment()->pointBegin()));
                }
                if (distance < altGoalDistance)
                {
                    altGoalLane     = headLane;
                    altGoalDistance = distance;
                }
            }

            // バックトレース
            // Backtrace
            searchStack.pop_back();
            continue;
        }

        //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
        /*
         * 探索の継続
         *   下流レーンをランダム順でスタックに追加する．
         *
         * Continue searching
         *   Add downstream lanes to the stack in random order.
         */
        searchStack.pop_back();
        visited.push_back(headLane);

        unsigned int numNextLanes = headLane->nextLanes().size();
        vector<int>  order
            = _vehicle->randomNumberGenerator()->getShuffled(numNextLanes);

        for (unsigned int i = 0; i < numNextLanes; i++)
        {
            const Lane* nextLane = headLane->nextLane(order[i]);
            if (find(visited.begin(), visited.end(), nextLane) == visited.end())
            {
                searchStack.emplace_back(nextLane);
                lanePairs.insert(make_pair(nextLane, headLane));
            }
        }
    }

    //------------------------------------------------------------------
    // 探索に失敗した場合，代替ゴールがあればそれを採用
    // If the search failed, adopt the alternative goal if available.
    if (altGoalLane)
    {
        const Lane* downstream = altGoalLane;
        while (true)
        {
            result_lanes.push_back(downstream);
            if (downstream == startLane)
            {
                break;
            }
            else
            {
                downstream = lanePairs[downstream];
                continue;
            }
        }
        reverse(result_lanes.begin(), result_lanes.end());
    }

    return false;
}

//==============================================================================
bool LocalLaneRouter::_bfReverseSearchLocalRoute(
    vector<const Lane*>& farGoalLanes, vector<const Section*>& farWays,
    const Intersection* goalIntersection, vector<const Lane*>& result_lanes)
{
    result_lanes.clear();

    // 探索状況を保持するキュー
    // Queue that holds the search status
    deque<const Lane*> searchQueue;
    searchQueue.clear();
    searchQueue.insert(
        searchQueue.end(), farGoalLanes.begin(), farGoalLanes.end());

    // 探索済みの要素
    // Already visited element
    vector<const Lane*> visited;
    visited.clear();
    visited.insert(visited.end(), farGoalLanes.begin(), farGoalLanes.end());

    // 最後に探索が成功した単路
    // Last road segment where the search succeeded.
    const Section* lastSection = nullptr;
    bool           lastIsUp;

    while (true)
    {
        //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
        // 探索終了
        // End of search
        if (searchQueue.empty())
        {
            break;
        }

        //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
        // 探索の先頭が goalIntersection に到達したら保存
        // Save when the head of the search reaches goalIntersection
        const Lane* headLane = searchQueue.front();
        if (typeid(*(headLane->parent())) == typeid(Section))
        {
            lastSection = dynamic_cast<Section*>(headLane->parent());
            lastIsUp    = lastSection->isUp(headLane);
        }
        if (headLane->previousStraightLane()->parent() == goalIntersection)
        {
            result_lanes.push_back(headLane);
        }

        //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
        // 探索の継続
        // Continue searching
        else
        {
            for (auto itr : headLane->previousLanes())
            {
                if ((typeid(*(itr->parent())) == typeid(Intersection)
                     || find(farWays.begin(), farWays.end(), itr->parent())
                         != farWays.end())
                    && find(visited.begin(), visited.end(), itr)
                        == visited.end())
                {
                    searchQueue.push_back(itr);
                    visited.push_back(itr);
                }
            }
        }
        searchQueue.pop_front();

        //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
        /*
         * この段階でキューが空になってしまった場合，最後の単路部の他のレーンを
         * 強制的にキューに加える
         *
         * If the queue is empty at this stage, forcibly add other lanes of the
         * last section to the queue.
         */
        if (result_lanes.empty() && searchQueue.empty())
        {
            for (auto itr :
                 lastSection->lanesFrom(lastSection->intersection(!lastIsUp)))
            {
                if (find(visited.begin(), visited.end(), itr) == visited.end())
                {
                    searchQueue.push_back(itr);
                    visited.push_back(itr);
                }
            }
        }
    }

    return !(result_lanes.empty());
}

//==============================================================================
void LocalLaneRouter::print(std::ostream& out) const
{
    static string lanePositionName[3] = {"left   ", "current", "right  "};

    stringstream oss;

    oss << "Lane Change Candidate Lane:" << endl;
    for (unsigned int i = 0; i < 3; i++)
    {
        oss << "\t" << lanePositionName[i] << ": "
            << (_targetLanes[i] ? _targetLanes[i]->id() : "NULL") << endl;
    }

    oss << "Route Followed on Candidate Lane:" << endl;
    for (unsigned int i = 0; i < 3; i++)
    {
        oss << "\t" << lanePositionName[i] << ": "
            << (_isRouteConsistentLane[i] ? "true" : "false") << endl;
    }

    oss << "Number of Lane Change Required:" << endl;
    for (unsigned int i = 0; i < 3; i++)
    {
        oss << "\t" << lanePositionName[i] << ": " << _numLaneChangeRequired[i]
            << endl;
    }

    if (!(_desiredLocalRoute.empty()))
    {
        oss << "Desired Local Route:" << endl;
        for (unsigned int i = 0; i < _desiredLocalRoute.size(); i++)
        {
            oss << "\t" << _desiredLocalRoute[i]->id() << " in "
                << _desiredLocalRoute[i]->parent()->id() << endl;
        }
    }

    oss << "Route Incentives:" << endl;
    for (unsigned int i = 0; i < 3; i++)
    {
        oss << "\t" << lanePositionName[i] << ": " << _routeIncentives[i]
            << endl;
    }

    oss << "Speed Incentives:" << endl;
    for (unsigned int i = 0; i < 3; i++)
    {
        oss << "\t" << lanePositionName[i] << ": " << _speedIncentives[i]
            << endl;
    }

    oss << "Utility:" << endl;
    double maxUtil = 0.0;
    for (unsigned int i = 0; i < 3; i++)
    {
        if (_targetLanes[i])
        {
            oss << "\t" << lanePositionName[i] << ": " << _utilities[i] << endl;
            if (_utilities[i] > maxUtil)
            {
                maxUtil = _utilities[i];
            }
        }
    }
    if (maxUtil > DESIRE_THRESHOLD_COOPERATION)
    {
        oss << "\tCOOPERATIVE LANE CHANGE MODE" << endl;
    }
    else if (maxUtil > DESIRE_THRESHOLD_SYNCHRONIZATION)
    {
        oss << "\tSYNCHRONIZED LANE CHANGE MODE" << endl;
    }
    else if (maxUtil > 0)
    {
        oss << "\tFREE LANE CHANGE MODE" << endl;
    }

    amu::msg::message(out, oss.str());
}
