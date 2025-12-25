/* ******************************************************************
 * Copyright 2014-2024 ADVENTURE Project
 * All Rights Reserved
 * ****************************************************************** */
/**
 * @file VehicleLocalRoute.hpp
 */
#ifndef __VEHICLE_LOCAL_ROUTE_HPP__
#define __VEHICLE_LOCAL_ROUTE_HPP__
#include "Lane.hpp"
#include "RelativeDirection.hpp"
#include <algorithm>
#include <cstdlib>
#include <iostream>
#include <string>
#include <vector>

class Vehicle;

//######################################################################
/**
 * @~japanese 自動車エージェントの局所的経路
 * @~english  Local route for vehicle agent
 * @~ @ingroup Vehicle Routing
 */
struct VehicleLocalRoute
{
public:
    VehicleLocalRoute()
    {
        _vehicle = nullptr;

        _localRoute.clear();

        _mainLaneInIntersection = nullptr;
        _turning                = RD::STRAIGHT;
        _lanesInIntersection.clear();

        _targetLane      = nullptr;
        _targetDirection = LanePosition::Center;
        _targetUtility   = 0.0;
    }

    ~VehicleLocalRoute() {}

    /**
     * @~japanese @p currentLane の下流のレーンを戻す
     * @~english  Return the downstream lane of @p currentLane
     */
    const Lane* next(const Lane* currentLane) const
    {
        auto itr = find(_localRoute.begin(), _localRoute.end(), currentLane);
        if (itr != _localRoute.end())
        {
            itr++;
            if (itr != _localRoute.end())
            {
                return *itr;
            }
        }
        return nullptr;
    }

    /**
     * @~japanese
     * @p currentLane の下流にあるレーン列を @p result_lanes に格納する
     *
     * @~english
     * Store the lane sequence downstream of @p currentLane in
     * @p result_lanes
     */
    void getNextLanes(
        const Lane* currentLane, std::vector<const Lane*>& result_lanes) const;

    /**
     * @~japanese @p currentLane の上流のレーンを戻す
     * @~english  Return the upstream lane of @p currentLane
     */
    const Lane* previous(const Lane* currentLane) const
    {
        auto itr = find(_localRoute.rbegin(), _localRoute.rend(), currentLane);
        if (itr != _localRoute.rend())
        {
            itr++;
            if (itr != _localRoute.rend())
            {
                return *itr;
            }
        }
        return nullptr;
    }

    /**
     * @~japanese
     * @p currentLane の上流にあるレーン列を @p result_lanes に格納する
     *
     * @~english
     * Store the lane sequence upstream of @p currentLane in
     * @p result_lanes
     */
    void getPreviousLanes(
        const Lane* currentLane, std::vector<const Lane*>& result_lanes) const;

    /**
     * @~japanese 先頭のレーンを戻す
     * @~english  Return the head lane
     */
    const Lane* firstLane() const
    {
        if (_localRoute.empty())
            return nullptr;
        else
            return _localRoute.front();
    }

    /**
     * @~japanese 末尾のレーンを戻す
     * @~english  Return the tail lane
     */
    const Lane* lastLane() const
    {
        if (_localRoute.empty())
            return nullptr;
        else
            return _localRoute.back();
    }

    /**
     * @~japanese 車線変更の必要があるかいなか
     * @~english  Whether to require lane-change
     */
    bool requiresLaneChange() const
    {
        if (!_targetLane || _targetDirection == LanePosition::Center)
        {
            return false;
        }
        return true;
    }

    /**
     * @~japanese 走行予定のレーン列をクリアする
     * @~english  Clear the scheduled lane sequence
     */
    void clearLocalRoute()
    {
        _localRoute.clear();
    }

    /**
     * @~japanese
     * 次の交差点で通過するレーン列をクリアする
     *
     * @~english
     * Clear thte lane sequence to be passed at the next intersection
     */
    void clearLanesInIntersection()
    {
        _lanesInIntersection.clear();
    }

    /**
     * @~japanese
     * @p lane を次の交差点で通過するレーン列に追加する
     *
     * @~english
     * Add @p lane to the lane sequence to be passed the next
     * intersection
     */
    void addLanesInIntersection(const Lane* lane)
    {
        _lanesInIntersection.push_back(lane);
    }

    /**
     * @~japanese 格納している変数を @p out に出力する
     * @~english  Output the stored variables to @p out 
     */
    void print(std::ostream& out) const;

    //==================================================================
private:
    /**
     * @~japanese 所有者
     * @~english  Owner
     */
    const Vehicle* _vehicle;

    /**
     * @~japanese 走行予定のレーン列
     *
     * 車線変更を伴わないものなので，希望するレーン列とは限らない
     *
     * @~english  Scheduled lane sequence
     *
     * Not always be the desired lane sequence, since it does not
     * involve any lane-changes. 
     */
    std::vector<const Lane*> _localRoute;

    /**
     * @~japanese 次の交差点における中心レーン
     * @~english  Main lane at the next intersection
     */
    const Lane* _mainLaneInIntersection;

    /**
     * @~japanese 次の交差点で通過するレーン列
     * @attention 必ず通過順に格納する必要がある
     *
     * @~english  Lane sequence to be passed at the next intersection
     * @attention Must be stored in the order of passage.
     */
    std::vector<const Lane*> _lanesInIntersection;

    /**
     * @~japanese
     * @brief 車線変更しない場合の，次の交差点での転回方向
     */
    RelativeDirection _turning;

    /**
     * @~japanese 車線変更の目標レーン
     * @~english  Target lane of lane-change
     */
    const Lane* _targetLane;

    /**
     * @~japanese 車線変更の方向
     * @~english  Lane-change direction
     */
    LanePosition::Type _targetDirection;

    /**
     * @~japanese 車線変更の効用
     * @~english  Utility of lane-change
     */
    double _targetUtility;

    //==================================================================
    /**
     * @~japanese @name アクセッサ
     * @~english  @name Accessor
     */
    ///@{
public:
    void setVehicle(const Vehicle* vehicle)
    {
        _vehicle = vehicle;
    }

    const std::vector<const Lane*>& localRoute() const
    {
        return _localRoute;
    }

    void setLocalRoute(std::vector<const Lane*>& lanes)
    {
        _localRoute.resize(lanes.size());
        std::copy(lanes.begin(), lanes.end(), _localRoute.begin());
    }

    const Lane* mainLaneInIntersection() const
    {
        return _mainLaneInIntersection;
    }

    void setMainLaneInIntersection(const Lane* lane)
    {
        _mainLaneInIntersection = lane;
    }

    const std::vector<const Lane*>& lanesInIntersection() const
    {
        return _lanesInIntersection;
    }

    const RelativeDirection& turning() const
    {
        return _turning;
    }

    void setTurning(const RelativeDirection& turning)
    {
        _turning.setValue(turning.value());
    }

    void setTurning(RD_t dir)
    {
        _turning.setValue(dir);
    }

    const Lane* targetLane() const
    {
        return _targetLane;
    }

    void setTargetLane(const Lane* targetLane)
    {
        _targetLane = targetLane;
    }

    LanePosition::Type targetDirection() const
    {
        return _targetDirection;
    }

    void setTargetDirection(LanePosition::Type direction)
    {
        _targetDirection = direction;
    }

    double targetUtility() const
    {
        return _targetUtility;
    }

    void setTargetUtility(double utility)
    {
        _targetUtility = utility;
    }

    ///@}
};

#endif //__VEHICLE_LOCAL_ROUTE_HPP__
