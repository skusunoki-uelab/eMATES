/* **************************************************************************
 * Copyright 2014-2024 ADVENTURE Project
 * All Rights Reserved
 * ************************************************************************** */
/**
 * @file LocalLaneRouter.hpp
 */
#ifndef __LOCAL_LANE_ROUTER_HPP__
#define __LOCAL_LANE_ROUTER_HPP__
#include "Intersection.hpp"
#include "ODNode.hpp"
#include "RoadMap.hpp"
#include "Section.hpp"
#include "VehicleGlobalRoute.hpp"
#include "VehicleLocalRoute.hpp"
#include <string>
#include <map>
#include <iostream>
#include <cstdlib>

class Vehicle;

//##############################################################################
/**
 * @~japanese 車線単位の局所経路の探索器
 * @~english  Lane-wise local router
 * @~ @ingroup Routing
 */
class LocalLaneRouter
{
public:
    LocalLaneRouter()
    {
        _vehicle     = nullptr;
        _globalRoute = nullptr;
        _localRoute  = nullptr;
        _desiredLocalRoute.clear();
        clear();
    }
    ~LocalLaneRouter() {}

    /**
     * @~japanese 所有者とその部分クラスオブジェクトを登録する
     * @~english  Set owner and its part class objects
     */
    void setVehicle(
        Vehicle* vehicle, VehicleGlobalRoute* globalRoute,
        VehicleLocalRoute* localRoute);

    /**
     * @~japanese ローカル経路を探索する
     *
     * @note
     * 確実に車線変更できる単路部まで車線を探索する
     *
     * @~english  Search local route
     *
     * @note
     * Search lanes to the section where the vehicle can reliably
     * change lanes
     */
    void localReroute(
        const Section* section, const Lane* lane, const double length);

    /**
     * @~japanese 車線変更を伴わない局所経路が見つかっているかどうか
     * @~english  Whether a local route was found without lane-change
     */
    bool hasFoundLocalRoute() const
    {
        return _isRouteConsistentLane[LanePosition::Center];
    }

    /**
     * @~japanese @p lane の次のレーン戻す
     * 
     * 車線変更を伴わない経路が見つかっている場合にはその経路に従った次の
     * レーンを戻す．そうでなければひとまず直進レーンを戻す．
     *
     * @~english  Return the next lane of @p lane
     *
     * If a route without lane-change is found, return the next lane following
     * the route. If not, return the straight lane for the time being. 
     */
    const Lane* nextLaneWithoutLaneChange(const Lane* lane) const
    {
        if (hasFoundLocalRoute())
        {
            return _localRoute->next(lane);
        }
        else
        {
            return lane->nextStraightLane();
        }
    }

    /**
     * @~japanese
     * 車線変更候補の効用を更新し車線変更先のレーンを決定する
     *
     * @~english
     * Update utilities of lane-change candidates and decide the target lane to
     * change to
     */
    void decideTargetLane(
        const Section* section, const Lane* lane, const double length);

    /**
     * @~japanese 探索結果を消去する
     * @~english  Clear search result
     */
    void clear()
    {
        for (unsigned int i = 0; i < 3; i++)
        {
            _targetLanes[i]              = nullptr;
            _isRouteConsistentLane[i]    = false;
            _isRouteConsistentFarLane[i] = false;
            _numLaneChangeRequired[i]    = 0;
            _routeIncentives[i]          = 0.0;
            _speedIncentives[i]          = 0.0;
            _utilities[i]                = 0.0;
        }
    }

private:
    //==========================================================================
    /**
     * @~japanese
     * グローバル経路の1つ先と2つ先の交差点および転回方向を取得する
     *
     * 車両の現在位置-> front -> nextの順で_routeが構成されている
     *
     * @~english
     * Get first and second intersections of planned global route and determine
     * turning behavior at next section
     *
     * current vehicle position -> frontIntersection -> nextIntersection
     */
    void _getFrontIntersectionAndTurning(
        const Section* section, const Lane* lane,
        const Intersection** result_frontIntersection,
        const Intersection** result_nextIntersection,
        int*                 result_frontDirection);

    /**
     * @~japanese
     * 上流交差点 @p rearIntersection ，下流交差点 @p frontIntersection の次に
     * 到達する交差点を仮に定める
     * 
     * ( @p rearIntersection, @p frontIntersection ) を接続する単路部にいる際に
     * 呼び出される．事前の大域的な経路探索に失敗している場合に呼び出されるため
     * 仮の局所経路でしかない． 
     * 
     * @param keepsRegulation
     * @p frontIntersection における通行規制を遵守する交差点を優先するかどうか
     * 
     * @~english
     * Temporarily determine the intersection to reach after the upstream
     * intersection @p rearInter and the downstream intersection @p frontInter.
     * 
     * Called by the vehicle in the section connecting ( @p rearIntersection, 
     * @p frontIntersection ). Returns just a tentative local route because it
     * is called in the case that the previous global route search has failed.
     * 
     * @param keepsRegulation
     * Whether to give priority to intersections that comply with traffic
     * regulations at @p frontIntersection or not
     */
    const Intersection* _getAlternativeNextInter(
        const Intersection* rearIntersection,
        const Intersection* frontIntersection, bool keepsRegulation) const;

    /**
     * @~japanese 局所経路探索のゴールとなるレーンを決定する
     * @~english  Decide lane as the goal of local routing
     */
    bool _decideGoalLanes(
        const Section* section, const Intersection* frontIntersection,
        int frontDirection, std::vector<const Lane*>& result_goalLanes);

    /**
     * @~japanese @p lane の周囲のレーンを探索する
     *
     * @note
     * 1回以下の車線変更で大域的経路を満たせない場合は，さらに隣のレーンを探索
     * する．
     *
     * @~english  Search peripheral lanes of @p lane
     *
     * @note
     * If the global route cannot be satisfied with 1 or fewer lane-changes,
     * search further adjacent lanes. 
     */
    void _searchPeripheralLanes(
        const Lane* lane, double distance,
        const Intersection* frontIntersection,
        const Intersection* nextIntersection, std::vector<const Section*>& ways,
        std::vector<const Lane*>& goalLanes);

    /**
     * @~japanese
     * @p lane に対し @p direction で指定された方向のレーンを探索する
     *
     * @note
     * _searchPeripheralLanes から呼び出される
     *
     * @~english
     * Search lanes in the direction specified by @p direction for @p lane
     *
     * @note
     * Called from _searchPeripheralLanes.
     */
    void _searchSideLanes(
        LanePosition::Type direction, const Lane* lane, double distance,
        const Intersection* frontIntersection,
        const Intersection* nextIntersection, std::vector<const Section*>& ways,
        std::vector<const Lane*>& goalLanes);

    /**
     * @~japanese 必要な車線変更の回数を求める
     * @~english  Find the number of lane-changes required
     */
    void _calcNumLaneChangeRequired();

    /**
     * @~japanese 車線 @p lane の効用を求める
     * @~english  Calculate the utility of the lane @p lane
     */
    void _calcUtilities(
        const Section* section, const Lane* lane, double length);

    /**
     * @~japanese 経路インセンティブを求める
     * @~english  Calculate route incentives
     */
    void _calcRouteIncentives(
        const Section* section, const Lane* lane, double length);

    /**
     * @~japanese 速度インセンティブを求める
     * @note 現在は使用されていない
     *
     * @~english  Calculate speed incentives
     * @note Not currently used
     */
    void _calcSpeedIncentives(
        const Section* section, const Lane* lane, double length);

    /**
     * @~japanese 交差点内レーンを設定する
     * @~english  Set lanes within an intersection
     */
    void _setIntersectionLanes(
        const std::vector<const Lane*>& lanes,
        const Intersection*             frontIntersection)
    {
        for (auto itr : lanes)
        {
            if (frontIntersection->containsLane(itr))
            {
                _localRoute->addLanesInIntersection(itr);
                if (frontIntersection->hasMainLane(itr))
                {
                    _localRoute->setTurning(
                        frontIntersection->relativeDirection(itr));
                    _localRoute->setMainLaneInIntersection(itr);
                }
            }
        }
    }

    //==========================================================================
    /**
     * @~japanese @name 探索のコア関数
     * @~english  @name Core functions for routing 
     */
    ///@
private:
    /**
     * @~japanese
     * 車線の接続にもとづいて車線変更をともなわない局所的経路を探索し，
     * 結果を @p result_lanes に格納する 
     *
     * @~english
     * Search for a local route without lane-changes based on lane
     * connection, and store the search result in @p result_lanes
     */
    bool _getLocalLaneRoute(
        const Lane* startLane, const Intersection* frontIntersection,
        const Intersection* nextIntersection, std::vector<const Section*>& ways,
        std::vector<const Lane*>& goalLanes,
        std::vector<const Lane*>& result_lanes);

    /**
     * @~japanese
     * 分岐をすべて直進する代替経路を求める
     *
     * 必要な車線変更を実行できないときにこの経路が選択される
     *
     * @~english
     * Find alternate route that go straight through all branches
     *
     * This route is chosen when tthe required lane-change cannot be performed. 
     */
    bool _getAltLocalRoute(
        const Lane* lane, const Intersection* frontIntersection,
        std::vector<const Section*>& ways,
        std::vector<const Lane*>&    result_lanes);

    /**
     * @~japanese 深さ優先で局所的経路を探索する
     * @~english  Depth-first search for local route
     */
    bool _dfSearchLocalRoute(
        const Lane* lane, std::vector<const Section*>& ways,
        const Section* nextSection, std::vector<const Lane*>& goalLanes,
        std::vector<const Lane*>& result_lanes,
        bool&                     result_isNextSectionIncluded);

    /**
     * @~japanese 幅優先で局所的経路を逆順に探索する
     * @~english  Breadth-first search for local route in reverse order
     */
    bool _bfReverseSearchLocalRoute(
        std::vector<const Lane*>&    farGoalLanes,
        std::vector<const Section*>& farWays,
        const Intersection*          goalIntersection,
        std::vector<const Lane*>&    result_lanes);

    ///@}

    //==========================================================================
public:
    /**
     * @~japanese 情報を @p out に出力する
     * @~english  Output status to @p out
     */
    void print(std::ostream& out) const;

    //==========================================================================
private:
    /**
     * @~japanese 所有者
     * @~english  Owner
     */
    Vehicle* _vehicle;

    /// 大域経路
    /**
     * @~japanese 大域的経路
     * @~english  Global route
     */
    const VehicleGlobalRoute* _globalRoute;

    /**
     * @~japanese 見つかった局所的経路
     * @~english  Local route found
     */
    VehicleLocalRoute* _localRoute;

    /**
     * @~japanese 車線変更を伴う希望経路
     * @~english  Desired route with lane-change
     */
    std::vector<const Lane*> _desiredLocalRoute;

    //==========================================================================
    /**
     * @~japanese
     * @name 探索対象の車線の状況を保存する変数
     * @note 左，中央，右の3要素を持つ 
     *
     * @~english
     * @name Variables to store the conditions of the lanes to search for
     * @note Include 3 elements: Left, Center, Right
     *
     * @~ @see LanePosition
     */
    ///@{

    /**
     * @~japanese 調査対象のレーン
     * @~english  Lane to be searched
     */
    const Lane* _targetLanes[3];

    /**
     * @~japanese 1回以下の車線変更で大域的経路を満足するか
     * @~english  Global route can be satisfied by 1 or less lane-change
     */
    bool _isRouteConsistentLane[3];

    /**
     * @~japanese
     * その方向への2回以上の車線変更で大域的経路を満足するか
     *
     * @note
     * _isRouteConsistentFarLane[LanePosition::Center] はダミー．
     * LanePosition::Type をインデックスとして扱うために配列化．
     * 
     * @~english
     *  Global route can be satisfied by 2 or more lane-changes
     *
     * @note
     * _isRouteConsistentFarLane[LanePosition::Center] is dummy. Arrayed
     * to treat LanePosition::Type as an index.
     */
    bool _isRouteConsistentFarLane[3];

    /**
     * @~japanese
     * 大域的経路を満足するために必要な車線変更回数
     *
     * @~english
     *  Number of lane-changes required to satisfy the global route
     */
    int _numLaneChangeRequired[3];

    /**
     * @~japanese 経路インセンティブ
     * @~english  Route incentives
     */
    double _routeIncentives[3];

    /**
     * @~japanese 速度インセンティブ
     * @note 現在は使用されていない
     *
     * @~english  Speed incentives
     * @note Not currently used.
     */
    double _speedIncentives[3];

    /**
     * @~japanese 車線の効用
     * @~english  Lane utilities
     */
    double _utilities[3];

    ///@}
};

#endif // __LOCAL_LANE_ROUTER__
