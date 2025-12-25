/* ******************************************************************
 * Copyright 2014-2024 ADVENTURE Project
 * All Rights Reserved
 * ****************************************************************** */
/**
 * @file RouterAStar.cpp
 */
#include "RouterAStar.hpp"
#include "AppMates.hpp"
#include "Config.hpp"
#include "RoutingRecorder.hpp"
#include "RouterManager.hpp"
#include "RoutingNetwork.hpp"
#include "RoutingNode.hpp"
#include "RoutingLink.hpp"
#include "RandomNumberGenerator.hpp"
#include <AmuConverter.hpp>
#include <cfloat>
#include <algorithm>
#include <iostream>

using namespace std;
using namespace amu::converter;
using NodeStatus    = RouterAStar::NodeStatusAStar;
using Label         = RouterBase::NodeStatusBase::NodeLabel;
using PriorityQueue = std::priority_queue<
    NodeStatus*, std::vector<NodeStatus*>, bool (*)(NodeStatus*, NodeStatus*)>;

//======================================================================
RouterAStar::~RouterAStar()
{
    for (auto itr : _snodes)
    {
        delete itr.second;
    }
}

//======================================================================
void RouterAStar::initialize(const RoutingNetwork* network)
{
    // _snodesの設定
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
            targetNetwork = network->upperNetwork();
        }
        else
        {
            break;
        }
    }
}

//======================================================================
double RouterAStar::_search(vector<RoutingNode*>& result_nodes)
{
    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    // 個々の探索のための初期化
    // Initialization for individual routing
    _isInUse = true;
    result_nodes.clear();

    /*
     * 探索中の状況を保持する優先度付きキュー
     *   中身は NodeStatus* の配列で，推定コスト最小の要素が先頭になる．
     *
     * Priority queue that save the status of the routing
     *   Content is an array of NodeStatus*, and the lowest-estimated-
     *   cost element is at the head.
     */
    bool (*compare)(NodeStatus*, NodeStatus*)
        = [](NodeStatus* lhs, NodeStatus* rhs)
    {
        return (lhs->estimatedCost() > rhs->estimatedCost());
    };
    PriorityQueue nodeQueue(compare);

    /*
     * ロガーに静的な情報を記録
     *   各経路探索器でIDの付け方が異なる可能性があるため，毎回クリアし
     *   登録する
     *
     * Record static information in logger
     *   Clear and register every time because the method of assigning
     *   IDs may differ for each router.
     */
    if (_recorder)
    {
        _recorder->clearNodeStatusId2RoutingNode();

        map<string, NodeStatus*, less<string> >::iterator itn;
        for (itn = _snodes.begin(); itn != _snodes.end(); itn++)
        {
            _recorder->addRoutingNode(
                (*itn).second->id(), (*itn).second->routingNode());
        }
    }

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
    while (!nodeQueue.empty())
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
         * 実コストは推定コストよりも必ず大きいため，推定コストが
         * 暫定ゴールのコストよりも大きいpresentNodeは探索の対象から
         * 除外する．
         *
         * Since the actual cost is greater than the estimated one,
         * the presentNode whose estimated cost is greater than the
         * cost of the provisional goal is excluded from the target.
         */
        if (goal && presentNode->estimatedCost() > goal->cost())
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
            // downwardLink
            if (link == presentNode->routingNode()->downwardLink())
            {
                if (!_canExpandDownward(presentNode->routingNode(), link))
                {
                    continue;
                }
            }

            // upwardLink
            else if (link == presentNode->routingNode()->upwardLink())
            {
                if (!_canExpandUpward(presentNode->routingNode()))
                {
                    continue;
                }
            }

            // same rank
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
            NodeStatus* nextNode         = _snodes[link->endNode()->id()];
            double      nextNodeDistance = _calcDistance(presentNode, link);
            double      nextNodeEstimatedDistance
                = _calcEstimatedDistance(nextNodeDistance, nextNode);
            double nextNodeCost = _calcCost(presentNode, link);
            double nextNodeEstimatedCost
                = _calcEstimatedCost(nextNodeCost, nextNode, link);

            /*
             * 推定コストが暫定ゴールのコストよりも大きいnextNodeは
             * 探索の対象から除外する．コスト推定時と暫定ゴール更新時の
             * 判定が必要．
             *
             * The nextNode whose estimated cost is greater than the
             * cost of the provisional goal is excluded from the target.
             * Judgment is required twice when calculating estimated
             * cost and when updating the provisional goal.
             */
            if (goal && nextNodeEstimatedCost > goal->cost())
            {
                continue;
            }

            _visitNode(
                nextNode, link, presentNode->id(), nextNodeDistance,
                nextNodeEstimatedDistance, nextNodeCost, nextNodeEstimatedCost,
                nodeQueue);
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
        routeCost = goal->cost(); // [eMATES]
        _extractSearchResult(result_nodes, start, goal);
    }

    _resetRouting();
    return routeCost; // [eMATES]
}

//======================================================================
double RouterAStar::_calcCost(NodeStatus* presentNode, const RoutingLink* link)
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
double RouterAStar::_calcDistance(
    NodeStatus* presentNode, const RoutingLink* link)
{
    double distance = presentNode->distance();
    distance += link->cost(toUnderlying(RoutingParamIndex::DISTANCE));
    return distance;
}

//======================================================================
double RouterAStar::_calcEstimatedCost(
    double fixedCost, NodeStatus* nextNode, const RoutingLink*)
{
    double estimatedCost  = fixedCost;
    double distanceToGoal = _minDistToGoals(nextNode->routingNode());

    // 距離のヒューリスティクスh(n) [m] を加算
    // Add distance heuristics h(n) [m]
    estimatedCost += distanceToGoal * _weights[0];

    // 時間のヒューリスティクスh(n) [s] を加算
    // Add time heuristics h(n) [s]
    //   180 [km/h] = 3000 [m/min] = 50 [m/s]
    estimatedCost += distanceToGoal * _weights[1] * 6.0 / 100;

    // 他のヒューリスティクスは0
    // Other heuristics are 0.

    return estimatedCost;
}
//======================================================================
double RouterAStar::_calcEstimatedDistance(
    double fixedDist, NodeStatus* nextNode)
{
    return fixedDist + _minDistToGoals(nextNode->routingNode());
}

//======================================================================
bool RouterAStar::_canExpandHorizontally(
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
bool RouterAStar::_canExpandUpward(const RoutingNode* node) const
{
    // ランク0->1のupwardLinkを持つノードのみ展開可能
    // Only the node with an upwardLink of rank 0->1 can be expanded.
    if (node->networkRank() == 0)
    {
        return true;
    }
    else
    {
        return false;
    }
}

//======================================================================
bool RouterAStar::_canExpandDownward(
    const RoutingNode* node, const RoutingLink* link) const
{
    // ランク2以上であればつねに展開可能
    // Always expandable if the network rank 2 or higher
    if (node->networkRank() >= 2)
    {
        return true;
    }

    //------------------------------------------------------------------
    // 以下はupperNode->networkRank()==1である場合の展開可能条件
    // The expansion conditions when upperNode->networkRank()==1

    // 展開条件1. 下位ノードがゴールである
    // Expansion condition 1. The lower node is one of goal nodes.
    const RoutingNode* lowestNode = node->lowestNode();
    if (_isNodeIdIncluded(lowestNode, _goalNodes))
    {
        return true;
    }

    // 展開条件2. この先に集約リンクがあり，ゴールを含む
    // Expansion condition 2. Aggregate link ahead includes goals
    for (auto itr_l : node->outLinks())
    {
        if (itr_l == link || !(itr_l->isAggregatedLink()))
        {
            continue;
        }
        const vector<const RoutingNode*>& abbrNodes = itr_l->abbreviatedNodes();
        for (auto itr_n : abbrNodes)
        {
            if (_isNodeIdIncluded(itr_n, _goalNodes))
            {
                return true;
            }
        }
    }
    return false;
}

//======================================================================
void RouterAStar::_visitNode(
    NodeStatus* nextNode, const RoutingLink*, string prevId, double dist,
    double estimatedDist, double cost, double estimatedCost, PriorityQueue& pq)
{
    switch (nextNode->label())
    {
    case Label::UNREACHED:
        // 必ず更新する
        // Always update
        _updateNode(
            nextNode, prevId, dist, estimatedDist, cost, estimatedCost, pq);
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
        _updateNode(
            nextNode, prevId, dist, estimatedDist, cost, estimatedCost, pq);
        pq.emplace(nextNode);
        break;

    case Label::UPPERLABELED:
        // 必ず更新する
        // Always update
        _updateNode(
            nextNode, prevId, dist, estimatedDist, cost, estimatedCost, pq);
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
void RouterAStar::_updateNode(
    NodeStatus* nextNode, string prevId, double dist, double estimatedDist,
    double cost, double estimatedCost, PriorityQueue&)
{
    nextNode->setPrevId(prevId);
    nextNode->setDistance(dist);
    nextNode->setEstimatedDistance(estimatedDist);
    nextNode->setCost(cost);
    nextNode->setEstimatedCost(estimatedCost);

    // ログの記録
    // Recording log
    if (_recorder)
    {
        _recorder->addLog(nextNode);
    }
}

//======================================================================
void RouterAStar::_extractSearchResult(
    vector<RoutingNode*>& result_nodes, const NodeStatus* start,
    const NodeStatus* goal)
{
    NodeStatus* scan = const_cast<NodeStatus*>(goal);

    // ゴールからprevIdを辿ってスタートまで辿り，最後に反転させる
    // Follow prevId from the goal to the start, and finally reverse.
    while (true)
    {
        RoutingNode* node = scan->routingNode();
        if (scan->id() == start->id())
        {
            // 最後の要素(=start)を追加して終了
            // Add last element (=start) and exit
            result_nodes.push_back(node);
            break;
        }

        RoutingNode* prevNode = _snodes[scan->prevId()]->routingNode();
        RoutingLink* inLink   = node->inLink(prevNode);

        /*
         * prevNode->nodeの inLink が upwardLink や downwardLink の場合，
         * result_nodesに追加する必要はない
         *
         * If the inLink of prevNode->node is upwardLink or downwardLink,
         * not need to add to result_nodes
         */
        if (inLink == prevNode->upwardLink()
            || inLink == prevNode->downwardLink())
        {
            scan = _snodes[scan->prevId()];
            continue;
        }

        // 対応する最下位ノードを追加
        // Add corresponding lowest node
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
void RouterAStar::_resetRouting()
{
    for (auto itr : _snodes)
    {
        itr.second->setPrevId("-1");
        itr.second->setDistance(DBL_MAX);
        itr.second->setEstimatedDistance(DBL_MAX);
        itr.second->setCost(DBL_MAX);
        itr.second->setEstimatedCost(DBL_MAX);
        itr.second->setLabel(Label::UNREACHED);
    }

    _startNode = nullptr;
    _goalNodes.clear();
}

//======================================================================
double RouterAStar::_minDistToGoals(const RoutingNode* node)
{
    double minDistance = DBL_MAX;

    for (unsigned int i = 0; i < _goalNodes.size(); i++)
    {
        double distance = node->point().distance(_goalNodes[i]->point());
        if (distance < minDistance)
        {
            minDistance = distance;
        }
    }
    return minDistance;
}
