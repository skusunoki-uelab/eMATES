/* **************************************************************************
 * Copyright 2014-2024 ADVENTURE Project
 * All Rights Reserved
 * ************************************************************************** */
/**
 * @file RouterManager.hpp
 */
#ifndef __ROUTER_MANAGER_HPP__
#define __ROUTER_MANAGER_HPP__
#include "Intersection.hpp"
#include "ManagerBase.hpp"
#include "RoadMap.hpp"
#include "RoutingNetwork.hpp"
#include "RoutingNode.hpp"
#include "RouterBase.hpp"
#include <iostream>
#include <unordered_map>
#include <vector>
#ifdef _OPENMP
#include <omp.h>
#endif //_OPENMP

/**
 * @defgroup Routing
 * @~japanese
 * 経路と大域的な経路探索機能を提供する
 *
 * @~english
 * Provides routes and global route searching
 */

//##############################################################################
/**
 * @~japanese 経路探索器と経路探索用ネットワークを管理する
 * @~english  Manage routers and network for routing
 * @~ @ingroup Manager Routing
 */
class RouterManager : public ManagerBase
{
    friend class ManagerPool;

private:
    RouterManager();
    ~RouterManager();

    /**
     * @~japanese 親クラスの関数のオーバーライド
     * @~english  Override functions of parent class
     */
    void _finalizeInside() override {};

    //==========================================================================
public:
    /**
     * @~japanese 管理するオブジェクトをすべて削除する
     * @~english  Delete all managed objects
     */
    void deleteAll()
    {
        for (auto itr : _routingNetworks)
        {
            delete itr;
        }
        _routingNetworks.clear();
        deleteAllRouters();
    }

    //==========================================================================
    /**
     * @~japanese @name 経路探索用ネットワークに関する操作
     * @~english  @name Operations for routing networks 
     */
    ///@{

    /**
     * @~japanese 経路探索用ネットワークを作成する
     * @~english  Generate RoutingMap
     */
    bool getReadyRoutingNetworks();

    /**
     * @~japanese コストの更新が必要なリンクとして @p link を登録する
     * @~english  Register @p link as a link whose cost needs to be updated
     */
    void addUpdateRequiredLink(const RoutingLink* link);

    /**
     * @~japanese 初期リンクコストを付与する
     * @~english  Set initial link costs
     */
    bool setInitialCosts();

    /**
     * @~japanese リンクコストを更新する
     * @~english  Update link costs
     */
    bool renewCosts();

    /**
     * @~japanese 経路探索用ネットワーク @p network を追加する
     * @~english  Add routing network @p network.
     */
    void addRoutingNetwork(RoutingNetwork* network)
    {
        _routingNetworks.emplace_back(network);
    }

    /**
     * @~japanese 経路探索用ネットワークの最高ランクを返す
     * @~english  Return highest rank of routing networks
     */
    unsigned int highestNetworkRank() const
    {
        return _highestNetworkRank;
    }

    ///@}

    //==========================================================================
    /**
     * @~japanese @name 経路探索器に関する操作
     * @~english  @name Operations for router
     */
    ///@{

    /**
     * @~japanese 利用可能な経路探索器を返す
     * @~english  Return available router
     */
    RouterBase* assignRouter();

    /**
     * @~japanese 経路探索器 @p router を利用状態から開放する
     * @~english  Release route searcher @p router from use
     */
    void releaseRouter(RouterBase* router);

    /**
     * @~japanese すべての経路探索器を削除する
     * @~english  Delete all routers
     */
    void deleteAllRouters()
    {
        for (auto itr : _routers)
        {
            delete itr;
        }
        _routers.clear();
    }

    ///@}

    //==========================================================================
public:
    /**
     * @~japanese 経路探索洋ネットワークの情報を @p out に出力する
     * @~english  Output map for route search to @p out
     */
    void printRoutingNetwork(std::ostream& out) const;

    //==========================================================================
private:
    /**
     * @~japanese 道路地図
     * @~english  Road map
     */
    RoadMap* _roadMap;

    /**
     * @~japanese 経路探索用ネットワーク
     *
     * @note
     * 階層化するためvectorで保持する．階層化した場合のランクとvectorの
     * インデックスを一致させる
     *
     * @~english  Map for route search
     *
     * @note
     * It is stored by using vector in order to be layered. Be sure to
     * match the rank and the index of the vector when layered.
     */
    std::vector<RoutingNetwork*> _routingNetworks;

    /**
     * @~japanese 次のステップでコストを更新すべきリンク
     * @todo
     * 高レベルの RoutingLink のIDが一意でないので，IDをキーにした
     * unordered_mapを使おうと思ってもできない
     * 
     * @~english  Link to update cost in next step
     */
    std::unordered_map<std::string, RoutingLink*> _updateRequiredLinks;

    /**
     * @~japanese 経路探索用ネットワークの最高ランク
     * @~english  Highest rank of map for route search
     */
    unsigned int _highestNetworkRank;

    /**
     * @~japanese 経路探索器のプール
     *
     * @attention
     * 経路探索器は使用時に貸し出され，使用後は再利用される．必要に応じ
     * newされるため最後にdeleteしなければならない
     *
     * @~english  Route searcher pool
     *
     * @attention
     * Route searchers are lent when used and reused after use. Must be
     * deleted at the end because of being newed as needed.
     */
    std::vector<RouterBase*> _routers;

#ifdef _OPENMP
    /**
     * @~japanese ロック変数
     * @~english  Lock variable
     */
    omp_lock_t _lock;
#endif //_OPENMP

    //==========================================================================
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

    /**
     * @~japanese 最下位のRoutingNetworkを返す
     * @~english  Return lowest rank RoutingNetwork
     */
    RoutingNetwork* routingNetwork()
    {
        return _routingNetworks[0];
    }

    /**
     * @~japanese ランク @p rank のRoutingNetworkを返す
     * @~english  Return RoutingNetwork of rank @p rank
     */
    RoutingNetwork* routingNetwork(unsigned int rank)
    {
        if (rank >= _routingNetworks.size())
        {
            return nullptr;
        }
        return _routingNetworks[rank];
    }

    ///@}
};

#endif //__ROUTER_MANAGER_HPP__
