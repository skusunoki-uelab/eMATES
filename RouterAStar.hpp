/* ******************************************************************
 * Copyright 2014-2024 ADVENTURE Project
 * All Rights Reserved
 * ****************************************************************** */
/**
 * @file RouterAStar.hpp
 */
#ifndef __ROUTER_ASTAR_HPP__
#define __ROUTER_ASTAR_HPP__
#include "RouterBase.hpp"
#include "RoutingNode.hpp"
//#include <AmuPriorityQueue.hpp>
#include <cfloat>
#include <map>
#include <queue>
#include <string>

//######################################################################
/**
 * @~japanese A-star(A*)アルゴリズム法を用いる経路探索器
 *
 * @todo
 * 従来型のA*アルゴリズムを実装したいが，現在の実装ではあらゆる経路探索
 * アルゴリズムが階層型ネットワークを処理しなければならず，upwardLink，
 * downwardLinkの処理方法を記述しなければならない．A*アルゴリズムや
 * Dijkstraアルゴリズムは本来単層のネットワーク（あるいはランク1までの
 * ネットワーク）だけを対象にすべき．つまり，経路探索アルゴリズムごとに
 * 専用のRoutingNetworkを持つべき．
 *
 * @~english  Router using A-star(A*) algorithm
 * @~ @ingroup Routing
 */
class RouterAStar : public RouterBase
{
public:
    //==================================================================
    /**
     * @~japanese 探索中のノードの状態を記録する構造体
     * @~english  Struct to record the note status being searched
     */
    struct NodeStatusAStar : public RouterBase::NodeStatusBase
    {
    public:
        NodeStatusAStar() {};
        explicit NodeStatusAStar(RoutingNode* node) : NodeStatusBase(node)
        {
            _estimatedDistance = DBL_MAX;
            _estimatedCost     = DBL_MAX;
        }
        virtual ~NodeStatusAStar() {};

    protected:
        /**
         * @~japanese スタートからゴールまでの推定コスト
         *
         * f(n)=g(n)+h(n) における f(n) に相当する．スタートからこの
         * ノードまでの実コストは g(n) であり，親クラスで定義された
         * _cost に値が格納されている．
         *
         * @~english  Estimated cost from start to goal
         *
         * Equivalent to f(n) in f(n)=g(n)+h(n). The actual cost from
         * the start to this node is g(n), and the value is stored in
         * _cost defined in the parent class.
         */
        double _estimatedCost;

        /**
         * @~japanese スタートからゴールまでの推定距離
         *
         * 推定コストの距離成分のみ別に保存する．
         *
         * @~english  Estimated distance from start to goal
         *
         * Store the distance component of estimated cost separately.
         */
        double _estimatedDistance;

        /**
         * @~japanese @name アクセッサ
         * @~english  @name Accessor
         */
        ///@{
    public:
        double estimatedCost() const
        {
            return _estimatedCost;
        }

        void setEstimatedCost(double estimatedCost)
        {
            _estimatedCost = estimatedCost;
        }

        double estimatedDistance() const
        {
            return _estimatedDistance;
        }

        void setEstimatedDistance(double estimatedDistance)
        {
            _estimatedDistance = estimatedDistance;
        }

        ///@}
    };

    //==================================================================
public:
    RouterAStar() {}
    virtual ~RouterAStar();

    /**
     * @~japanese @name 親クラスの関数のオーバーライド
     * @~english  @name Override functions of parent class
     */
    ///@{
public:
    virtual void initialize(const RoutingNetwork* network) override;

protected:
    virtual double _search(std::vector<RoutingNode*>& result_nodes) override;

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
        NodeStatusAStar* presentNode, const RoutingLink* link);

    /**
     * @~japanese
     * リンク @p link を辿って到達する，確定コスト @p fixedCost を持つ
     * ノード @p nextNode からゴールまでの推定コストを計算する
     *
     * @~english
     * Calculate the estimated cost from the node @p nextNode to the
     * goal, with fixed cost @p fixedCost, reached by following the
     * link @p link
     */
    virtual double _calcEstimatedCost(
        double fixedCost, NodeStatusAStar* nextNode, const RoutingLink* link);

    /**
     * @~japanese
     * ノード @p presentNode からリンク @p link を辿って到達する次の
     * ノードの距離を計算する
     *
     * @~english
     * Calculate the distance of the next node reached by following
     * the link @p link from the node @p presentNode
     */
    virtual double _calcDistance(
        NodeStatusAStar* presentNode, const RoutingLink* link);

    /**
     * @~japanese
     * 確定距離 @p fixedDist を持つノード @p nextNode からゴールまでの
     * 推定距離を計算する
     *
     * @~english
     * Calculate the estimated distance from the node @p nextNode to
     * the goal, with fixed distance @p fixedDist
     */
    virtual double _calcEstimatedDistance(
        double fixedDist, NodeStatusAStar* nextNode);

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
     * @~japanese
     * 上位向きリンク @p link を辿ってノード @p lowerNode を展開可能か
     *
     * @~english
     * Return whether node @p lowerNode can be expanded by following
     * the upward link @p link
     */
    virtual bool _canExpandUpward(const RoutingNode* lowerNode) const;

    /**
     * @~japanese
     * 下位向きリンク @p link を辿ってノード @p node を展開可能か
     *
     * @~english
     * Return whether node @p node can be expanded by following
     * the downward link @p link
     */
    virtual bool _canExpandDownward(
        const RoutingNode* upperNode, const RoutingLink* downwardLink) const;

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
        NodeStatusAStar* nextNode, const RoutingLink* nextLink,
        std::string prevId, double dist, double estimatedDist, double cost,
        double estimatedCost,
        std::priority_queue<
            NodeStatusAStar*, std::vector<NodeStatusAStar*>,
            bool (*)(NodeStatusAStar*, NodeStatusAStar*)>& pq);

    /**
     * @~japanese @p nextNode を更新する
     * @~english  Update @p nextNode
     */
    virtual void _updateNode(
        NodeStatusAStar* nextNode, std::string prevId, double dist,
        double estimatedDist, double cost, double estimatedCost,
        std::priority_queue<
            NodeStatusAStar*, std::vector<NodeStatusAStar*>,
            bool (*)(NodeStatusAStar*, NodeStatusAStar*)>& pq);

    /**
     * @~japanese
     * 探索後の _snodes から経路探索解を抽出する
     *
     * @~english
     * Extract route solution from _snodes after finish routing
     */
    virtual void _extractSearchResult(
        std::vector<RoutingNode*>& result_nodes, const NodeStatusAStar* start,
        const NodeStatusAStar* goal);

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

    /**
     * @~japanese ゴールまでの直線距離の最小値を戻す
     * @note ゴールは複数存在する可能性がある．
     *
     * @~english  Returns the minimum direct distance to the goal
     * @note There can be multiple goals.
     */
    virtual double _minDistToGoals(const RoutingNode* node);

protected:
    /**
     * @~japanese NodeStatus のコンテナ
     * @~english  Container for NodeStatus
     */
    std::map<std::string, NodeStatusAStar*, std::less<std::string> > _snodes;
};

#endif //__PF_ASTAR_H__
