/* ******************************************************************
 * Copyright 2014-2024 ADVENTURE Project
 * All Rights Reserved
 * ****************************************************************** */
/**
 * @file RouterDijkstra.cpp
 */
#include "RouterDijkstra.hpp"
#include "AppMates.hpp"
#include "RoutingRecorder.hpp"
#include "RandomNumberGenerator.hpp"
#include "RouterManager.hpp"
#include "RoutingNetwork.hpp"
#include "RoutingNode.hpp"
#include "RoutingLink.hpp"
#include <algorithm>
#include <cfloat>
#include <iostream>

using namespace std;
using NodeStatus    = RouterDijkstra::NodeStatusDijkstra;
using Label         = RouterBase::NodeStatusBase::NodeLabel;
using PriorityQueue = std::priority_queue<
    NodeStatus*, std::vector<NodeStatus*>, bool (*)(NodeStatus*, NodeStatus*)>;

//======================================================================
RouterDijkstra::~RouterDijkstra()
{
    for (auto itr : _snodes)
    {
        delete itr.second;
    }
}

//======================================================================
void RouterDijkstra::initialize(const RoutingNetwork* network)
{
    // _snodes の設定
    // Set _snodes
    _snodes.clear();
    const RoutingNetwork* targetNetwork = network;

    while (true)
    {
        for (auto itr : targetNetwork->nodes())
        {
            NodeStatus* anode = new NodeStatus(itr.second);
            _snodes.insert(make_pair(anode->id(), anode));
        }
        if (targetNetwork->upperNetwork())
        {
            targetNetwork = targetNetwork->upperNetwork();
        }
        else
        {
            break;
        }
    }
}

//======================================================================
double RouterDijkstra::_search(std::vector<RoutingNode*>& result_nodes)
{
    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    // 個々の探索のための初期化
    // Initialization for individual routing
    _isInUse = true;
    result_nodes.clear();

    /*
     * 探索中の状況を保持する優先度付きキュー
     *   中身は NodeStatus* の配列で，コスト最小の要素が先頭になる．
     *
     * Priority queue that save the status of the routing
     *   Content is an array of NodeStatus*, and the lowest-cost element
     *   is at the head.
     */
    bool (*compare)(NodeStatus*, NodeStatus*)
        = [](NodeStatus* lhs, NodeStatus* rhs)
    {
        return (lhs->cost() < rhs->cost());
    };
    PriorityQueue nodeQueue(compare);

    // スタートノードをキューにpush
    // Push start node to queue
    NodeStatus* start = _snodes[_startNode->id()];
    start->setDistance(0);
    start->setCost(0);
    nodeQueue.emplace(start);
    if (_recorder)
    {
        _recorder->addLog(start);
    }

    // 複数のゴールの中から到達したものをあとで代入する
    // Assign one reached goal among multiple goals later
    NodeStatus* goal = nullptr;

    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    while (true)
    {
        NodeStatus* presentNode = nodeQueue.top();
        nodeQueue.pop();

        //--------------------------------------------------------------
        /*
         * 到達できたコスト最小のゴールを暫定のゴールとする
         *
         * The lowest-cost goal that has been reached is regarded as the
         * provisional goal.
         */
        if (_isNodeIdIncluded(
                presentNode->routingNode()->lowestNode(), _goalNodes))
        {
            if (!goal || presentNode->cost() < goal->cost())
            {
                goal = presentNode;
            }
        }

        /*
         * コストが暫定ゴールのコストよりも大きいpresentNodeは探索の対象
         * から除外する．
         *
         * The presentNode whose cost is greater than that of the
         * provisional goal is excluded from the target.
         */
        if (goal && presentNode->cost() > goal->cost())
        {
            continue;
        }

        //--------------------------------------------------------------
        // 先頭要素を始点とするリンクを辿る
        // Follow a link starting from the head element
        for (auto itr : presentNode->routingNode()->outLinks())
        {
            const RoutingLink* link = itr;

            //..........................................................
            /*
             * [暫定措置] upwardLinkは無視する
             *   upwardLinkを無視するのでdownwardLinkは自動的に無視
             *
             * [Provisional measure] Ignore upwardLink
             *   Since upward link is ignored, downwardLink is
             *   automatically ignored.
             */
            if (link == presentNode->routingNode()->upwardLink())
            {
                continue;
            }

            // Links with same rank
            else
            {
                if (!_canExpandHorizontally(presentNode->routingNode(), link))
                {
                    continue;
                }
            }

            //..........................................................
            // 次のノードの処理
            // Processing the next node
            NodeStatus* nextNode     = _snodes[link->endNode()->id()];
            double      nextNodeCost = _calcCost(presentNode, link);

            /*
             * コストが暫定ゴールのコストよりも大きいnextNodeは探索の
             * 対象から除外する．コスト算出時と暫定ゴール更新時の判定が
             * 必要．
             *
             * The nextNode whose cost is greater than that of the
             * provisional goal is excluded from the target. Judgment
             * is required twice when calculating ost and when updating
             * the provisional goal.
             */
            if (goal && nextNodeCost > goal->cost())
            {
                continue;
            }

            _visitNode(
                nextNode, link, presentNode->id(), nextNodeCost, nodeQueue);
        }

        //--------------------------------------------------------------
        /*
         * このノードを始点とするリンクをすべて辿り終えたら，ラベルを
         * SCANNED に変更
         *
         * After finished following all links starting from the node,
         * change its label to SCANNED.
         */
        presentNode->setLabel(Label::SCANNED);
    }

    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    // 探索に成功したら探索結果を格納
    // Store the routing result if the search is successful
    double routeCost = DBL_MAX; // [eMATES]
    if (goal)
    {
        _extractSearchResult(result_nodes, start, goal);
        routeCost = goal->cost(); // [eMATES]
    }

    _resetRouting();
    return routeCost; // [eMATES]
}

//======================================================================
double RouterDijkstra::_calcCost(
    NodeStatus* presentNode, const RoutingLink* link)
{
    double cost = presentNode->cost();
    for (unsigned int i = 0; i < VEHICLE_ROUTING_PARAMETER_SIZE; i++)
    {
        cost += _weights[i] * link->cost(i);
    }
    cost += link->csCost(); // [eMATES] 2025/5/30 by abe
    return cost;
}

//======================================================================
bool RouterDijkstra::_canExpandHorizontally(
    const RoutingNode* node, const RoutingLink* link) const
{
    /*
     * 展開不可（リンクが上位ネットワークで同じノードに接続）
     *
     * Cannot expand since the link connects to the same node in the
     * upper network
     */
    if (node->networkRank() == 0)
    {
        if (node->upperNode() && link->endNode()->upperNode())
        {
            return false;
        }
        return true;
    }

    // 展開不可 (通行規制による)
    // Cannot expand due to traffic regulation
    if (!(node->permitsPassing(*_vehicleType))
        || !(link->permitsPassing(*_vehicleType)))
    {
        return false;
    }

    // 展開不可 (探索確率による)
    // Cannot be expand due to routing probability
    if (_rng->uniform() > min(
            node->probability(*_vehicleType), link->probability(*_vehicleType)))
    {
        return false;
    }

    return true;
}

//======================================================================
void RouterDijkstra::_visitNode(
    NodeStatus* nextNode, const RoutingLink*, string prevId, double cost,
    PriorityQueue& pq)
{
    switch (nextNode->label())
    {
    case Label::UNREACHED:
        // 必ず更新する
        // Always update
        _updateNode(nextNode, prevId, cost, pq);
        nextNode->setLabel(Label::LABELED);
        pq.emplace(nextNode);
        break;

    case Label::LABELED:
        // コストが以前の値を下回らなければ更新しない
        // Not update unless cost drops below previous value
        if (cost >= nextNode->cost())
        {
            return;
        }
        _updateNode(nextNode, prevId, cost, pq);
        pq.emplace(nextNode);
        break;

    case Label::UPPERLABELED:
        // 必ず更新する
        // Always update
        _updateNode(nextNode, prevId, cost, pq);
        nextNode->setLabel(Label::LABELED);
        pq.emplace(nextNode);
        break;

    case Label::SCANNED:
        // 何もしない
        // Do nothing
        return;

    default:
        return;
    }
}

//======================================================================
void RouterDijkstra::_updateNode(
    NodeStatus* nextNode, string prevId, double cost, PriorityQueue&)
{
    nextNode->setPrevId(prevId);
    nextNode->setCost(cost);
}

//======================================================================
void RouterDijkstra::_extractSearchResult(
    vector<RoutingNode*>& result_nodes, const NodeStatus* start,
    const NodeStatus* goal)
{
    // ゴールからスタートまで，_prevIdを辿って探索結果を格納する．
    // Store the result from the goal to the start, following _prevId.
    NodeStatus* scan = const_cast<NodeStatus*>(goal);

    while (true)
    {
        RoutingNode* node = scan->routingNode();
        if (scan->id() == start->id())
        {
            // 最後の要素を格納して終了
            // Store last element and exit
            result_nodes.push_back(node);
            break;
        }

        RoutingNode* prevNode = _snodes[scan->prevId()]->routingNode();
        RoutingLink* inLink   = node->inLink(prevNode);

        /*
         * prevNode->node が upwardLink や downwardLink の場合は格納不要
         *
         * No need to store if prevNode->node is upwardLink or
         * downwardLink.
         */
        if (inLink == prevNode->upwardLink()
            || inLink == prevNode->downwardLink())
        {
            scan = _snodes[scan->prevId()];
            continue;
        }

        // NodeStatus に対応する RoutingNode を格納
        // Store RoutingNode corresponding to NodeStatus
        result_nodes.push_back(node->lowestNode());

        // inLinkが集約リンクである場合，展開する必要がある
        // If inLink is an aggregate link, it must be expanded.
        if (inLink->rank() >= 1 && inLink->isAggregatedLink())
        {
            const vector<const RoutingNode*> abbrNodes
                = inLink->includedLowestNodes();
            for (int i = abbrNodes.size() - 1; i >= 0; i--)
            {
                result_nodes.push_back(const_cast<RoutingNode*>(abbrNodes[i]));
            }
        }
        scan = _snodes[scan->prevId()];
    }

    reverse(result_nodes.begin(), result_nodes.end());
}

//======================================================================
void RouterDijkstra::_resetRouting()
{
    for (auto itr : _snodes)
    {
        itr.second->setPrevId("-1");
        itr.second->setDistance(DBL_MAX);
        itr.second->setCost(DBL_MAX);
        itr.second->setLabel(Label::UNREACHED);
    }
    _startNode = nullptr;
    _goalNodes.clear();
}
