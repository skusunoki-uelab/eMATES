/* **************************************************************************
 * Copyright 2014-2024 ADVENTURE Project
 * All Rights Reserved
 * ************************************************************************** */
/**
 * @file RouterAStarHierarchy.hpp
 */
#ifndef __ROUTER_ASTAR_HIERARCHY_HPP__
#define __ROUTER_ASTAR_HIERARCHY_HPP__
#include "RouterAStar.hpp"
#include "RouterBase.hpp"
//#include <AmuPriorityQueue.hpp>
#include <queue>
#include <map>
#include <string>

//##############################################################################
/**
 * @~japanese 階層ネットワーク上でA-star(A*)法を用いる経路探索器
 * @~english  Router using A-star(A*) algorithm on hierarchical network
 * @~ @ingroup Routing
 */
class RouterAStarHierarchy : public RouterAStar
{
public:
    //==========================================================================
    /**
     * @~japanese 探索中のノードの状態を記録する構造体
     * @~english  Struct to record the note status being searched
     */
    class NodeStatusAStarHierarchy : public RouterAStar::NodeStatusAStar
    {
    public:
        NodeStatusAStarHierarchy() {}
        explicit NodeStatusAStarHierarchy(RoutingNode* node)
            : NodeStatusAStar(node)
        {
        }
        virtual ~NodeStatusAStarHierarchy() {};

    protected:
        /**
         * @~japanese 上位ノードの識別番号
         * @~english  ID number of upper node
         */
        std::string _upperId;

        /**
         * @~japanese 下位ノードの識別番号
         * @~english  ID number of lower node
         */
        std::string _lowerId;

        /**
         * @~japanese @name アクセッサ
         * @~english  @name Accessor
         */
        ///@{
    public:
        std::string upperId() const
        {
            return _upperId;
        }

        void setUpperId(const std::string& upperId)
        {
            _upperId = upperId;
        }

        std::string lowerId() const
        {
            return _lowerId;
        }

        void setLowerId(const std::string& lowerId)
        {
            _lowerId = lowerId;
        }
    };

    //==========================================================================
public:
    RouterAStarHierarchy()
    {
        _maxNetworkRank       = 1;
        _preferredNetworkRank = 1;
    }
    virtual ~RouterAStarHierarchy() {}

    /**
     * @~japanese @name 親クラスの関数のオーバーライド
     * @~english  @name Override functions of parent class
     */
    ///@{
public:
    virtual void initialize(const RoutingNetwork* network) override;

    virtual bool generateRouteCache(
        RoadMap* roadMap, std::vector<std::string>& tokens) const override;

protected:
    virtual bool _loadCache(
        RouteCacheContainer* container, const Intersection* from,
        const Intersection* next, const Intersection* goal,
        Route& result_route) const override;

    virtual void _saveCache(
        RouteCacheContainer* container, const Intersection* from,
        const Intersection* next, const Intersection* goal,
        const Route& route) const override;

    virtual double _calcEstimatedCost(
        double fixedCost, RouterAStar::NodeStatusAStar* nextNode,
        const RoutingLink* link) override;

    virtual bool _canExpandHorizontally(
        const RoutingNode* node, const RoutingLink* link) const override;

    virtual bool _canExpandUpward(const RoutingNode* node) const override;

    virtual void _visitNode(
        RouterAStar::NodeStatusAStar* nextNode, const RoutingLink* nextLink,
        std::string prevId, double dist, double estimatedDist, double cost,
        double estimatedCost,
        std::priority_queue<
            RouterAStar::NodeStatusAStar*, std::vector<NodeStatusAStar*>,
            bool (*)(NodeStatusAStar*, NodeStatusAStar*)>& pq) override;

    virtual void _resetRouting() override;

    virtual void _resetStatus() override;

    ///@}

protected:
    /**
     * @~japanese ネットワークランクの上限
     * @~english  Max network rank
     */
    int _maxNetworkRank;

    /**
     * @~japanese 選好するネットワークランク
     *
     * 指定されたランク未満のランクの探索により大きなコストを付与することで，
     * 上位ネットワークを優先的に探索する．
     *
     * @~english  Preferred network rank
     *
     * By giving a higher cost on searches of ranks less than the specified
     * rank, preferentially search higher network.
     */
    int _preferredNetworkRank;

    /**
     * @~japanese @name アクセッサ
     * @~english  @name Accessor
     */
    ///@{
public:
    int preferredNetworkRank() const
    {
        return _preferredNetworkRank;
    }

    void setPreferredNetworkRank(int networkRank)
    {
        _preferredNetworkRank = networkRank;
    }

    ///@}
};

#endif //__ROUTER_ASTAR_HIERARCHY_HPP__
