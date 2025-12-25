/* ******************************************************************
 * Copyright 2014-2024 ADVENTURE Project
 * All Rights Reserved
 * ****************************************************************** */
/**
 * @file RouteKey.hpp
 */
#ifndef __ROUTE_KEY_BASE_HPP__
#define __ROUTE_KEY_BASE_HPP__
#include "Config.hpp"
#include "VehicleType.hpp"
#include "VehicleTypeManager.hpp"
#include <iostream>
#include <vector>

class Intersection;

//######################################################################
/**
 * @~japanese 経路のキャッシュを格納するときに使用するキーの基底クラス
 *
 * @note
 * 経路探索アルゴリズムに応じて拡張することを想定している
 *
 * @~english Base class for keys used when storing route cache
 *
 * @note
 * Assumed to be extended according to the routing algorithm.
 *
 * @~ @ingroup Routing
 */
struct RouteKeyBase
{
public:
    RouteKeyBase() {}
    RouteKeyBase(
        const VehicleType type, const double weights[],
        const Intersection* before, const Intersection* start,
        const Intersection* goal);

    virtual ~RouteKeyBase() {};

    /**
     * @~japanese @p another とキーが一致するかどうか
     * @~english  Whether the key matches @p another
     */
    virtual bool equals(RouteKeyBase* another) const;

    /**
     * @~japanese キーを表示する
     * @~english  Display key
     */
    virtual void print(std::ostream& out) const;

protected:
    /**
     * @~japanese 車種
     *
     * @note
     * 車種ごとに通行可能な道路が異なるため，探索結果が異なる
     *
     * @~english  Vehicle type
     *
     * @note
     * The search results may be different because the roads that can be
     * driven on differ for each vehicle type.
     */
    const VehicleType _vehicleType;

    /**
     * @~japanese 経路探索パラメータ
     * @~english  Routing parameters
     *
     * @todo std::arrayに変更を検討
     */
    double _weights[VEHICLE_ROUTING_PARAMETER_SIZE];

    /**
     * @~japanese 経路探索を開始する交差点に至る交差点
     * @~english  Intersection leading to intersection to start routing
     */
    const Intersection* _before;

    /**
     * @~japanese 経路探索を開始する交差点
     * @~english  Intersection to start routing
     */
    const Intersection* _start;

    /**
     * @~japanese 経路探索を終える交差点
     * @~english  Intersection to end routing
     */
    const Intersection* _goal;

    //==================================================================
    /**
     * @~japanese @name アクセッサ
     * @~english  @name Accessor
     */
    ///@{
public:
    const VehicleType& vehicleType() const
    {
        return _vehicleType;
    }

    double weight(unsigned int i) const
    {
        return _weights[i];
    }

    const Intersection* before() const
    {
        return _before;
    }

    const Intersection* start() const
    {
        return _start;
    }

    const Intersection* goal() const
    {
        return _goal;
    }

    ///@}
};

#endif //__ROUTE_KEY_BASE_HPP__
