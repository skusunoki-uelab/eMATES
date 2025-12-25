/* ******************************************************************
 * Copyright 2014-2024 ADVENTURE Project
 * All Rights Reserved
 * ****************************************************************** */
/**
 * @file RouterDijkstra.hpp
 */
#ifndef __ROUTER_DIJKSTRA_HPP__
#define __ROUTER_DIJKSTRA_HPP__
#include "RouterBase.hpp"
#include <map>
#include <queue>
#include <string>

class RouterManager;

//######################################################################
/**
 * @~japanese ダイクストラ法を用いる経路探索器
 * @~english  Router using Dijkstra's algorithm
 * @~ @ingroup Routing
 */
class RouterDijkstra : public RouterBase
{
    //==================================================================
    /**
     * @~japanese 探索中のノードの状態を記録する構造体
     * @~english  Struct to record the note status being searched
     */
public:
    struct NodeStatusDijkstra : public RouterBase::NodeStatusBase
    {
    public:
        NodeStatusDijkstra() {}
        NodeStatusDijkstra(RoutingNode* node) : NodeStatusBase(node) {}
        virtual ~NodeStatusDijkstra() {}
    };

    //==================================================================
public:
    RouterDijkstra() {}
    virtual ~RouterDijkstra();

    /**
     * @~japanese @name 親クラスの関数のオーバーライド
     * @~english  @name Override functions of parent class
     */
    ///@{
public:
    virtual void initialize(const RoutingNetwork* network) override;

protected:
    virtual double _search(
        std::vector<RoutingNode*>& result_nodes) override;

    ///@}

protected:
    /**
     * @~japanese
     * ノード @p presentNode からリンク @p link を辿って到達する次の
     * ノードのコストを計算する
     *
     * @~english
     * Calculate the cost of the next node reached by following
     * the link @p link from the node @p presentNode
     */
    virtual double _calcCost(
        NodeStatusDijkstra* presentNode, const RoutingLink* link);

    /**
     * @~japanese
     * 同ランクのリンク @p link を辿ってノード @p node を展開可能か
     *
     * @~english
     * Return whether node @p node can be expanded by following
     * the same rank link @p link
     */
    virtual bool _canExpandHorizontally(
        const RoutingNode* node, const RoutingLink* link) const;

    /**
     * @~japanese 次のノード @p nextNode を訪問する
     *
     * ラベルおよび既知のコストを参照し，必要に応じて _updateNode を
     * 呼び出す．
     *
     * @~english  Visit the next node @p nextNode
     *
     * Look up labels and known costs, and call _updateNode as needed.
     */
    virtual void _visitNode(
        NodeStatusDijkstra* nextNode, const RoutingLink* nextLink,
        std::string prevId, double cost,
        std::priority_queue<
            NodeStatusDijkstra*, std::vector<NodeStatusDijkstra*>,
            bool (*)(NodeStatusDijkstra*, NodeStatusDijkstra*)>& pq);

    /**
     * @~japanese @p nextNode を更新する
     * @~english  Update @p nextNode
     */
    virtual void _updateNode(
        NodeStatusDijkstra* nextNode, std::string prevId, double cost,
        std::priority_queue<
            NodeStatusDijkstra*, std::vector<NodeStatusDijkstra*>,
            bool (*)(NodeStatusDijkstra*, NodeStatusDijkstra*)>& pq);

    /**
     * @~japanese
     * 探索後の _snodes から経路探索解を抽出する
     *
     * @~english
     * Extract route solution from _snodes after finish routing
     */
    virtual void _extractSearchResult(
        std::vector<RoutingNode*>& result_nodes,
        const NodeStatusDijkstra*  start,
        const NodeStatusDijkstra*  goal);

    /**
     * @~japanese 探索状態を元に戻す
     *
     * @attention
     * 探索器そのものを元に戻す RouterBase::_resetStatus() とは異なる．
     *
     * @~english  Reset routing state
     *
     * @attention
     * Different from RouterBase::_resetStatus() which resets the router
     * itself.
     */
    virtual void _resetRouting();

private:
    /**
     * @~japanese NodeStatus のコンテナ
     * @~english  Container for NodeStatus
     */
    std::map<std::string, NodeStatusDijkstra*, std::less<std::string> >
        _snodes;
};

#endif //__ROUTER_DIJKSTRA_HPP__
