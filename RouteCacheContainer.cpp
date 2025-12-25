/* ******************************************************************
 * Copyright 2014-2024 ADVENTURE Project
 * All Rights Reserved
 * ****************************************************************** */
/**
 * @file RouteCacheContainer.cpp
 */
#include "RouteCacheContainer.hpp"
#include "AppMates.hpp"
#include "RouteCache.hpp"
#include "RouteCacheManager.hpp"
#include "RouteKeyBase.hpp"
#include "Intersection.hpp"
#include "GVManager.hpp"
#include <algorithm>
#include <iostream>
#include <fstream>
#include <cassert>
#include <cstdlib>
#include <algorithm>
#include <climits>

using namespace std;

//======================================================================
RouteCacheContainer::RouteCacheContainer()
{
    _intersection = nullptr;
    _components.clear();
    _componentsAdded.clear();
    _componentsReordered.clear();
    _rng.reset();
    _isUsed = false;
#ifdef _OPENMP
    omp_init_lock(&_lock);
#endif //_OPENMP
}

//======================================================================
RouteCacheContainer::RouteCacheContainer(Intersection* inter)
    : RouteCacheContainer()
{
    _intersection = inter;
}

//======================================================================
RouteCacheContainer::~RouteCacheContainer()
{
    for (auto itr : _components)
    {
        if (itr)
        {
            delete itr;
        }
    }
    _components.clear();

#ifdef _OPENMP
    omp_destroy_lock(&_lock);
#endif //_OPENMP
}

//======================================================================
void RouteCacheContainer::addRouteCacheDirectly(
    const RouteKeyBase* key, const Route& route, int count)
{
    RouteCache* component = new RouteCache();
    component->makeCache(key, route);
    component->setCount(count);

    /*
     * コンテナサイズを固定とするため，新たな要素をpushする前にリストの
     * 先頭の要素をpopする．
     *
     * To fix the container size, pop the head component of the list
     * before pushing a new component.
     */
    if (_components.size() > static_cast<unsigned int>(
            AppMates::getGVManager().getNumeric("VEHICLE_CACHE_ROUTING_SIZE")))
    {
        delete _components[0];
        _components.pop_front();
    }

    // 新たな要素を末尾にpushする
    // Push a new component to the end
    _components.emplace_back(component);
}

//======================================================================
void RouteCacheContainer::addRouteCache(
    const std::string& userId, const RouteKeyBase* key, const Route& route)
{
    // 同じ内容が既に格納されていないかチェックする．
    // Check if the same cache is already stored.
    for (auto itr : _components)
    {
        if (itr->equals(const_cast<RouteKeyBase*>(key), route))
        {
            // 見つかった場合，使用回数をカウントアップする．
            // If found, count up the number of uses.
            itr->countUp();
            return;
        }
    }

    // 見つからなかった場合，新たに登録する
    // If not found, register new cache.
    RouteCache* component = new RouteCache();
    component->makeCache(key, route);
    component->setCount(1);

#ifdef _OPENMP
    omp_set_lock(&_lock);
#endif //_OPENMP

    /*
     * いったん_componentsAddedに格納しソートする．
     *   次ステップのrenewRouteCacheでまとめて更新する．
     *
     * Stored in _componentsAdded and sorted first.
     *   Update all at once with renewRouteCache in the next step.
     */
    _componentsAdded.insert(make_pair(userId, component));
    if (!_isUsed)
    {
        _isUsed = true;
        AppMates::getRouteCacheManager().addUsedContainer(this);
    }

#ifdef _OPENMP
    omp_unset_lock(&_lock);
#endif //_OPENMP
}

//======================================================================
bool RouteCacheContainer::searchRouteCache(
    const std::string& userId, RouteKeyBase* key, Route& result_route)
{
    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    // コピーする候補を収集する．
    // Collect candidates to copy.
    vector<RouteCache*> candidates;
    vector<int>         candCounts;
    int                 sumCounts = 0;
    candidates.clear();
    candCounts.clear();

    for (auto itr : _components)
    {
        if (itr->hasEqualKey(key))
        {
            candidates.push_back(itr);
            candCounts.push_back(itr->count());
            sumCounts += itr->count();
        }
    }
    if (candidates.empty())
    {
        // 条件を満たす候補が見つからなかった
        // No candidates that meet the criteria
        return false;
    }

    if (sumCounts > INT_MAX / 2)
    {
        sumCounts = 0;
        /*
         * シミュレーションを繰り返すとcountがあふれるため補正する．
         *
         * Correct count because it will overflow when simulation is
         * repeated.
         */
        for (unsigned int i = 0; i < candidates.size(); i++)
        {
            candidates[i]->divideCountBy(2);
            candCounts[i] = candidates[i]->count();
            sumCounts += candidates[i]->count();
        }
    }

    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    // コピーするキャッシュを決定する．
    // Decide which cache to copy.
    int rnd   = _rng.uniform(sumCounts);
    int index = 0;
    while (true)
    {
        if (rnd < candCounts[index])
        {
            break;
        }
        else
        {
            rnd -= candCounts[index];
            index++;
        }
    }
    candidates[index]->countUp();

    // 経路のコピー
    // Copy route
    const Route& route = candidates[index]->route();
    if (route.intersections().size() < 2)
    {
        // 不正な経路
        // Invalid route
        return false;
    }
    result_route.clearIntersections();
    result_route = route;

#ifdef _OPENMP
    omp_set_lock(&_lock);
#endif //_OPENMP

    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    // 選択されたキャッシュを次ステップのrenewRouteCacheで更新する
    // Update selected caches with renewRouteCache in the next step

    /*
     * キャッシュが重複しているかどうかチェックする
     *   同時に経路再探索を行った場合など，同一のポインタが二重に
     *   登録されるのを防ぐ (メモリ二重開放の抑止)
     *
     * Check for duplicate caches
     *   Prevents the same pointer from being registered twice, such as
     *   when rerouting at the same step (suppression of double freeing
     *   memory).
     */
    bool isFound = false;
    for (auto itr : _componentsReordered)
    {
        if (itr.second == candidates[index])
        {
            isFound = true;
            break;
        }
    }
    if (!isFound)
    {
        _componentsReordered.insert(make_pair(userId, candidates[index]));
    }

    // このRouteCacheContainerを次ステップの更新対象にする
    // Make this RouteCacheContainer the update target of the next step
    if (!_isUsed)
    {
        _isUsed = true;
        AppMates::getRouteCacheManager().addUsedContainer(this);
    }

#ifdef _OPENMP
    omp_unset_lock(&_lock);
#endif //_OPENMP

    return true;
}

//======================================================================
void RouteCacheContainer::renewRouteCache()
{
    // componentsReordered，_componentsAddedの順で処理する
    // Process _componentsReordered and _componentsAdded in order.

    deque<RouteCache*> renewComponents;

    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    /*
     * _componentsReorderedの処理
     *   重複を避けつつ新しいコンテナへコピーする．
     *
     * Process _componentsReordered
     *   Copy to new container while avoiding duplication.
     */
    for (auto itr : _components)
    {
        bool isComponentFound = false;
        for (auto itr_r : _componentsReordered)
        {
            if (itr == itr_r.second)
            {
                isComponentFound = true;
                break;
            }
        }

        // _componentReorderedに見つかった場合は後で登録される．
        // Registered later if found in _componentsReordered.
        if (isComponentFound)
        {
        }
        // 見つからなかった場合はこの時点で登録する．
        // Register now if it is not found in _componentsReordered.
        else
        {
            renewComponents.push_back(itr);
        }
    }

    // 続いて_componentsReorderedを追加する
    // Then add _componentsReordered
    for (auto itr_r : _componentsReordered)
    {
        renewComponents.push_back(itr_r.second);
    }

    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    // _componentsAddedの処理
    // Process _componentsAdded
    for (auto itr_a : _componentsAdded)
    {
        renewComponents.push_back(itr_a.second);
    }

    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    // _componentsのサイズが大きくなりすぎたら先頭から削除
    // Delete from the head if the size of _components becomes too large
    while (
        renewComponents.size() > static_cast<unsigned int>(
            AppMates::getGVManager().getNumeric("VEHICLE_CACHE_ROUTING_SIZE")))
    {
        delete renewComponents[0];
        renewComponents.pop_front();
    }

    // 結果をswapして一時的な格納をクリアする．
    // Swap the result and clear the temporary storage.
    _components.swap(renewComponents);
    _componentsAdded.clear();
    _componentsReordered.clear();
    _isUsed = false;
}

//======================================================================
void RouteCacheContainer::removeDynamicComponent()
{
    auto itr = remove_if(
        _components.begin(), _components.end(),
        [](const RouteCache* c)
        {
            return !(c->isStatic());
        });
    _components.erase(itr, _components.end());
    // print();
}

//======================================================================
void RouteCacheContainer::print(ostream& out) const
{
    for (auto itr : _components)
    {
        itr->print(out);
    }
}

//======================================================================
void RouteCacheContainer::printStatic(ostream& out) const
{
    for (auto itr : _components)
    {
        if (itr->isStatic())
        {
            itr->print(out);
        }
    }
}
