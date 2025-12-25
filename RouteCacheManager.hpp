/* ******************************************************************
 * Copyright 2014-2024 ADVENTURE Project
 * All Rights Reserved
 * ****************************************************************** */
/**
 * @file RouteCacheManager.hpp
 */
#ifndef __ROUTE_CACHE_MANAGER_HPP__
#define __ROUTE_CACHE_MANAGER_HPP__
#include "Intersection.hpp"
#include "ManagerBase.hpp"
#include "RoadMap.hpp"
#include "RouteCacheContainer.hpp"
#include <vector>
#include <typeinfo>

//######################################################################
/**
 * @~japanese
 * 大域的経路探索を管理する
 *
 * 経路探索用ネットワーク(RoutingMap)と経路探索器(Router)を持つ
 *
 * @~english
 * Manage global routing
 *
 * Retain map for route search (RoutingMap) and route searcher
 * (pathfinder : PF)
 * 
 * @~ @ingroup Manager Routing
 */
class RouteCacheManager : public ManagerBase
{
    friend class ManagerPool;

private:
    RouteCacheManager()
    {
        _usedContainers.clear();
        _className = typeid(this).name();
    }
    ~RouteCacheManager() {}

    /**
     * @~japanese 親クラスの関数のオーバーライド
     * @~english  Override functions of parent class
     */
    void _finalizeInside() override {};

    //==================================================================
    /**
     * @~japanese @name 経路探索結果の保存に関する操作
     * @~english  @name Operations for storing route search results
     */
    ///@{
public:
    /**
     * @~japanese
     * 使用された経路探索結果コンテナ @p container を追加する
     *
     * @~english
     * Add used route search result container @p container.
     */
    void addUsedContainer(RouteCacheContainer* container)
    {
        _usedContainers.emplace_back(container);
    }

    /**
     * @~japanese 経路探索結果コンテナを更新する
     * @~english  Update route search result containers
     */
    void renewContainers()
    {
#ifdef INCLUDE_VEHICLES
        for (auto itr : _usedContainers)
        {
            itr->renewRouteCache();
        }
        _usedContainers.clear();
#endif //INCLUDE_VEHICLES
    }

    /**
     * @~japanese すべての経路探索結果コンテナを削除する
     * @~english  Delete all route search result containers
     */
    void deleteAllContainers()
    {
        for (auto itr : _roadMap->intersections())
        {
            itr.second->deleteRouteCacheContainer();
        }
    }

    ///@}

    //==================================================================
private:
    /**
     * @~japanese 道路地図
     * @~english  Road map
     */
    RoadMap* _roadMap;

    /**
     * @~japanese
     * 更新のあった経路探索結果コンテナ
     *
     * _usedContainerの要素に対してのみrenewRouteResultを実行する．
     * renewContainer()の最後にクリアされる．
     *
     * @~english
     * Updated route search result container
     *
     * Run renewRouteResult() only for components _usedRouteContainer.
     * Cleared at the end of renewRouteCacheContainer().
     */
    std::vector<RouteCacheContainer*> _usedContainers;

    //==================================================================
    /**
     * @~japanese @name アクセッサ
     * @~english  @name Accessor
     */
    ///@{
public:
    void setRoadMap(RoadMap* roadMap)
    {
        _roadMap = roadMap;
    }

    ///@}
};

#endif //__ROUTE_CACHE_MANAGER_HPP__
