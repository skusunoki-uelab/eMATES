/* ******************************************************************
 * Copyright 2014-2024 ADVENTURE Project
 * All Rights Reserved
 * ****************************************************************** */
/**
 * @file RouteCacheContainer.hpp
 */
#ifndef __ROUTE_CACHE_CONTAINER_HPP__
#define __ROUTE_CACHE_CONTAINER_HPP__
#include "RandomNumberGenerator.hpp"
#include "Route.hpp"
#include "RouteCache.hpp"
#include "RouteKeyBase.hpp"
#include <deque>
#include <iostream>
#include <map>
#include <string>
#include <vector>
#ifdef _OPENMP
#include <omp.h>
#endif //_OPENMP

class Intersection;

//######################################################################
/**
 * @~japanese 経路のキャッシュを格納するコンテナ
 *
 * 計算時間の削減のため，過去の経路探索結果をキャッシュし，同条件の探索
 * クエリに対しキャッシュした内容を返す．キャッシュファイルをインポート
 * することで過去のランにおける経路探索結果も反映できる．各交差点が所有
 * する．
 *
 * @~english  Container that stores route caches
 *
 * In order to reduce computational time, cache past routing results
 * and return the cached contents for queries with the same conditions.
 * By importing a cache file, the routing results of the past runs can
 * be reflected. Owned by each intersection.
 *
 * @~ @ingroup Routing
 */
class RouteCacheContainer
{
public:
    RouteCacheContainer();
    explicit RouteCacheContainer(Intersection* inter);
    ~RouteCacheContainer();

    /**
     * @~japanese
     * @p key，@p routeからなる1つのキャッシュを格納する
     *
     * @note
     * キャッシュファイルのインポート時のように処理順が確定している
     * 場合に用いる．
     *
     * @~english
     * Store one cache consisting of @p key, @p route
     *
     * @note
     * Used when the processing order is fixed, such as when importing
     * a cache file.
     */
    void addRouteCacheDirectly(
        const RouteKeyBase* key, const Route& route, int count);

    /**
     * @~japanese
     * @p key, @p route からなる1つのキャッシュをあらたに格納する
     *
     * @note
     * マルチスレッドで動作する．処理順を固定するため，@p userId を
     * キーとして_componentsAddedに登録し，あとでまとめて更新する．
     * 
     * @~english
     * Store a new cache consisting of @p key, @p route
     *
     * @note
     * Work in multithread. So as to fix the processing order, register
     * to _componentsAdded using @p userId as a key, and update them
     * together later.
     */
    void addRouteCache(
        const std::string& userId, const RouteKeyBase* key, const Route& route);

    /**
     * @~japanese @p key を持つ経路のキャッシュを検索する
     * @~english  Search route cache with @p key
     */
    bool searchRouteCache(
        const std::string& userId, RouteKeyBase* key, Route& result_route);

    /**
     * @~japanese キャッシュを更新する
     *
     * 前のステップで_componentsAddedおよび_componentsReorderedに
     * 登録されたキャッシュを_componentsに移す．
     *
     * @~english  Update caches
     *
     * Move the cache registered in _componentsAdded and
     * _componentsReordered in the previous step to _components.
     */
    void renewRouteCache();

    /**
     * @~japanese 動的なキャッシュを消去する
     *
     * 道路ネットワークのコスト更新時に呼び出される．
     *
     * @~english  Remove dynamic caches
     *
     * Called when the road network cost is updated.
     */
    void removeDynamicComponent();

    /**
     * @~japanese 格納しているキャッシュを@p out に出力する
     * @~english  Output stored caches to @p out
     */
    void print(std::ostream& out) const;

    /**
     * @~japanese
     * 格納しているキャッシュのうち静的なキャッシュを@p out に出力する
     *
     * @~english
     * Output static caches out of stored caches to @p out
     */
    void printStatic(std::ostream& out) const;

    //==================================================================
private:
    /**
     * @~japanese このコンテナを所有する交差点
     * @~english  Intersection that owns this container
     */
    Intersection* _intersection;

    /**
     * @~japanese 経路のキャッシュ
     *
     * @note
     * キャッシュはデストラクタ内でdeleteされる．
     *
     * @~english  Route caches
     *
     * Caches are Deleted in the destructor.
     */
    std::deque<RouteCache*> _components;

    /**
     * @~japanese
     * 追加すべきキャッシュを一時的に格納するコンテナ
     *
     * @~english
     * Container that temporarily stores caches to be added
     */
    std::map<std::string, RouteCache*, std::less<std::string> >
        _componentsAdded;

    /**
     * @~japanese
     * 再ソートすべきキャッシュを一時的に格納するコンテナ
     *
     * @~english
     * Container that temporarily stores caches to be re-sorted
     */
    std::map<std::string, RouteCache*, std::less<std::string> >
        _componentsReordered;

    /**
     * @~japanese 乱数生成器
     * @~english  Random number generator
     */
    RandomNumberGenerator _rng;

    /**
     * @~japanese 前ステップで使用されたかどうか
     *
     * 使用されたコンテナのみを対象にして renewRouteCache() を呼び出す
     *
     * @~english  Whether it was used in the previous step
     *
     * Call renewRouteCache() only for used containers.
     */
    bool _isUsed;

#ifdef _OPENMP
    /**
     * @~japanese ロック変数
     * @~english  Lock variable
     */
    omp_lock_t _lock;
#endif //_OPENMP

    //==================================================================
    /**
     * @~japanese @name アクセッサ
     * @~english  @name Accessor
     */
    ///@{
public:
    Intersection* intersection() const
    {
        return _intersection;
    }

    void serIntersection(Intersection* inter)
    {
        _intersection = inter;
    }

    ///@}
};

#endif //__ROUTE_CACHE_CONTAINER_HPP__
