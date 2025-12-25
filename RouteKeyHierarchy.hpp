/* ******************************************************************
 * Copyright 2014-2024 ADVENTURE Project
 * All Rights Reserved
 * ****************************************************************** */
/**
 * @file RouteKeyHierarchy.hpp
 */
#ifndef __ROUTE_KEY_HIERARCHY_HPP__
#define __ROUTE_KEY_HIERARCHY_HPP__
#include "Config.hpp"
#include "RouteKeyBase.hpp"
#include "VehicleTypeManager.hpp"
#include <iostream>
#include <vector>

class Intersection;

//######################################################################
/**
 * @~japanese
 * 経路のキャッシュを格納するときに使用するキーの階層ネットワーク版
 *
 * @~english 
 * Hierarchical network version of key used when storing route cache
 *
 * @~
 * @see RouteKeyBase
 * @ingroup Routing
 */
struct RouteKeyHierarchy : public RouteKeyBase
{
public:
    RouteKeyHierarchy() : _prefRank(0) {};
    RouteKeyHierarchy(
        const VehicleType type, const unsigned int prefRank,
        const double weights[], const Intersection* before,
        const Intersection* start, const Intersection* goal);

    virtual ~RouteKeyHierarchy() {};

    //==================================================================
    /**
     * @~japanese @name 親クラスの関数のオーバーライド
     * @~english  @name Override functions of parent class
     */
    ///@{
    virtual bool equals(RouteKeyBase* another) const override;

    virtual void print(std::ostream& out) const override;

    ///@}

private:
    /**
     * @~japanese 選好するネットワークランク
     * @~english  Preferred network rank
     */
    const unsigned int _prefRank;

    //==================================================================
    /**
     * @~japanese @name アクセッサ
     * @~english  @name Accessor
     */
    ///@{
public:
    unsigned int prefRank() const
    {
        return _prefRank;
    }

    ///@}
};

#endif //__ROUTE_KEY_HIERARCHY_HPP__
