/* ******************************************************************
 * Copyright 2014-2024 ADVENTURE Project
 * All Rights Reserved
 * ****************************************************************** */
/**
 * @file VehicleGlobalRoute.hpp
 */
#ifndef __VEHICLE_GLOBAL_ROUTE_HPP__
#define __VEHICLE_GLOBAL_ROUTE_HPP__
#include "Intersection.hpp"
#include "Route.hpp"
#include <iostream>
#include <vector>

//######################################################################
/**
 * @~japanese 自動車エージェントの大域的経路
 * @~english  Global route for vehicle agents
 * @~ @ingroup Vehicle Routing
 */
struct VehicleGlobalRoute
{
public:
    VehicleGlobalRoute();
    ~VehicleGlobalRoute() {};

    //==================================================================
    /**
     * @~japanese @name 経路に関する関数
     * @~english  @name Functions related to route
     */
    ///@{

    /**
     * @~japanese 最後に通過した交差点のインデックスを設定する．
     *
     * @p prev， @p curr の順で通過したことを指定する． 同時に，
     * 必要であれば最後に通過したゲートのインデックスも指定する．
     *
     * @~english  Set the last passed intersection index
     *
     * Specifies @p prev and @p curr have been passed in this order.
     * Also set the last passed gate index if necessary.  
     */
    void setLastPassedIntersectionIndex(
        const Intersection* prev, const Intersection* curr);

    /**
     * @~japanese 最後に通過した交差点のインデックスを設定する．
     *
     * 発生直後など，上流交差点の指定がない場合．
     *
     * @~english  Set the last passed intersection index
     *
     * When no upstream intersection is specified, such as immediately
     * after generation.
     */
    void setLastPassedIntersectionIndex(const Intersection* curr);

    /**
     * @~japanese 最後に通過した交差点のインデックスをリセットする
     * @~english  Reset last passed intersection index
     */
    void resetLastPassedIntersectionIndex()
    {
        _lastPassedIntersectionIndex = -1;
    }

    /**
     * @~japanese
     * @p prev，@p cur の次に通過すべき交差点を戻す
     *
     * @note
     * 同じ単路を複数回通ることを想定しないが，先頭から順に探索すれば
     * 所望の機能を果たすと考えられる．
     *
     * @~english
     * Return the intersection to be passed after @p prev and @p cur.
     *
     * @note
     * Although it is not assumed that the same section will be passed
     * multiple times, thought that the desired function will be
     * achieved by searching sequentially from the beginning.
     */
    const Intersection* next(
        const Intersection* prev, const Intersection* curr) const;

    /**
     * @~japanese 次に通過すべき交差点を戻す
     *
     * @note
     * 同じ交差点を複数回通る場合を考慮し，より確実に次の交差点を
     * 定めるため，next(const Intersection*, const Intersection*)を
     * 使うことを推奨する．
     *
     * @~english  Return the next intersection to pass
     *
     * @note
     * Recommended to use next(const Intersection*, const Intersection*)
     * to determine the next intersection more reliably, considering the
     * case that the same intersection is passed multiple times.
     */
    const Intersection* next(const Intersection* curr) const;

    /**
     * @~japanese 経路の再探索回数をインクリメントする
     * @~english  Increment the number of rerouting
     */
    void incrementNumRerouting()
    {
        _numRerouting++;
    }

    ///@}

    //==================================================================
    /**
     * @~japanese @name ゲートに関する関数
     *
     * @note
     * ゲートとは通過すべき交差点で，出発地，目的地を含む
     *
     * @~english @name Functions related to gates
     *
     * @note
     * Gates are intersections to be passed including the origin  and
     * the destination. 
     */
    ///@{

    /**
     * @~japanese
     * 車両が現在位置以降に通過すべきゲートの集合を求め，@p result_gates
     * に代入する
     *
     * @~english
     * Find the set of gates the vehicle should pass after the current
     * position, and assign them intto @p result_gates
     */
    void getGatesToPass(std::vector<const Intersection*>& result_gates) const
    {
        for (unsigned int i = _lastPassedGateIndex; i < _gates.size(); i++)
        {
            result_gates.emplace_back(_gates[i]);
        }
    }

    /**
     * @~japanese ゲート @p gates を設定する
     * @~english  Set gates @p gates
     */
    void setGates(std::vector<const Intersection*>& gates)
    {
        _gates.clear();
        _gates.insert(_gates.end(), gates.begin(), gates.end());
    }

    /**
     * @~japanese 最後に通過したゲートのインデックスを設定する
     * @~english  Set the last passed gate index
     */
    void setLastPassedGateIndex(const Intersection* gate);

    /**
     * @~japanese 出発地を戻す
     * @~english  Return the origin
     */
    const Intersection* start() const
    {
        return _gates[0];
    }

    /**
     * @~japanese 目的地を返す
     * @~english  Return the destination
     */
    const Intersection* goal() const
    {
        return _gates[_gates.size() - 1];
    }

    ///@}

    //==================================================================
    /**
     * @~japanese 経路を @p out に出力する
     * @~english  Output route to @p out
     */
    void print(std::ostream& out) const;

    //==================================================================
private:
    /**
     * @~japanese
     * 通過すべき交差点を通過する順に保持する経路
     *
     * @~english
     * Route keeping the intersections to be passed in order of passing
     */
    Route _route;

    /**
     * @~japanese 最後に通過した交差点のインデックス
     * @~english  Index of the last passed intersection
     */
    int _lastPassedIntersectionIndex;

    /**
     * @~japanese ゲートのリスト
     * @~english  List of gates
     */
    std::vector<const Intersection*> _gates;

    /**
     * @~japanese 最後に通過したゲートのindex
     * @~english  Index of the last passed gate 
     */
    int _lastPassedGateIndex;

    /**
     * @~japanese 過去に経路探索に失敗した目的地
     * @~english  Destinations for which routing has failed in the past
     */
    std::vector<const Intersection*> _failedGoals;

    /**
     * @~japanese 走行中の経路の再探索の回数
     * @~english  Number of times rerouting while driving
     */
    int _numRerouting;

    //==================================================================
    /**
     * @name Accessor
     */
    ///@{
public:
    const Route& route() const
    {
        return _route;
    }

    void setRoute(const Route& route)
    {
        _route.clearIntersections();
        _route = route;
    }

    int lastPassedIntersectionIndex() const
    {
        return _lastPassedIntersectionIndex;
    }

    const std::vector<const Intersection*>& gates() const
    {
        return _gates;
    }

    void addGate(const Intersection* gate)
    {
        _gates.push_back(gate);
    }

    int lastPassedGateIndex() const
    {
        return _lastPassedGateIndex;
    }

    const std::vector<const Intersection*>& failedGoals() const
    {
        return _failedGoals;
    }

    const Intersection* failedGoal(int i) const
    {
        return _failedGoals[i];
    }

    void addFailedGoal(const Intersection* goal)
    {
        _failedGoals.push_back(goal);
    }

    int numRerouting() const
    {
        return _numRerouting;
    }

    ///@}
};

#endif //__VEHICLE_GLOBAL_ROUTE_HPP__
