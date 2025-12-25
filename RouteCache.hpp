/* ******************************************************************
 * Copyright 2014-2024 ADVENTURE Project
 * All Rights Reserved
 * ****************************************************************** */
/**
 * @file RouteCache.hpp
 */
#ifndef __ROUTE_CACHE_HPP__
#define __ROUTE_CACHE_HPP__
#include "Config.hpp"
#include "Route.hpp"
#include "RouteKeyBase.hpp"
#include "VehicleTypeManager.hpp"
#include <vector>
#include <iostream>
#include <algorithm>

class Intersection;

//######################################################################
/**
 * @~japanese 経路のキャッシュ
 *
 * 経路 (Route) と経路探索パラメータからなる．RouteCacheContainerに
 * 格納される要素．
 *
 * @~english  Route cache
 *
 * Consists of a Route and routing parameters. It is a component stored
 * in RouteCacheContainer . 
 * 
 * @~ @ingroup Routing
 */
struct RouteCache
{
public:
    RouteCache();
    ~RouteCache();

    /**
     * @~japanese 経路をキャッシュする
     * @~english  Cache route
     */
    void makeCache(const RouteKeyBase* key, const Route& route);

    /**
     * @~japanese 指定した @p key と同じキーを持つかどうか
     * @~english  Whether to have the same key as the specified @p key
     */
    bool hasEqualKey(RouteKeyBase* key) const;

    /**
     * @~japanese
     * @p key, @p route を持つキャッシュと同じかどうか
     *
     * @~english
     * Whether to be the same as the cache that has @p key and @p route
     */
    bool equals(RouteKeyBase* key, const Route& route) const;

    /**
     * @~japanese 使用回数をカウントアップする
     * @~english  Count up the number of uses
     */
    void countUp()
    {
        _count++;
    }

    /**
     * @~japanese 使用回数を@p n で除す
     *
     * @attention
     * _countが大きくなった場合の補正に使用する．端数の扱いは考慮して
     * いないので注意すること．

     * @~english  Divide the number of uses by @p n
     *
     * @attention
     * Used for correction when count becomes large. Note that handling
     * of fractions is not considered.
     */
    void divideCountBy(int num)
    {
        _count = std::max(1, _count / num);
    }

    /**
     * @~japanese キャッシュされた内容を@p out に出力する
     * @~english  Output cached contents to @p out 
     */
    void print(std::ostream& out) const;

private:
    /**
     * @~japanese 経路探索条件をあらわすキー
     * @~english  Key representing a routing condition
     */
    const RouteKeyBase* _key;

    /**
     * @~japanese 保存されている経路
     * @~english  Saved route
     */
    Route _route;

    /**
     * @~japanese 静的な経路かどうか
     *
     * ここでは，たとえば道路の混雑によって旅行時間が変動するなど，
     * 時間の経過にともなって変化する経路を動的な経路とする
     *
     * @~english  Whether the saved route is static
     *
     * Here, dynamic routes are defined as routes that change over time,
     * such as trip times that vary due to road congestion.  
     */
    bool _isStatic;

    /**
     * @~japanese 使用回数
     * @~english  Number of uses
     */
    int _count;

    //==================================================================
    /**
     * @~japanese @name アクセッサ
     * @~english  @name Accessor
     */
    ///@{
public:
    const Route& route() const
    {
        return _route;
    }

    bool isStatic() const
    {
        return _isStatic;
    }

    int count() const
    {
        return _count;
    }

    void setCount(int count)
    {
        _count = count;
    }

    ///@}
};

#endif //__ROUTE_CACHE_HPP__
