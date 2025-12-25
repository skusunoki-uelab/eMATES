/* ******************************************************************
 * Copyright 2014-2024 ADVENTURE Project
 * All Rights Reserved
 * ****************************************************************** */
/**
 * @file RouterAStarHierarchy.cpp
 */
#include "RouterAStarHierarchy.hpp"
#include "AppMates.hpp"
#include "RouteCacheContainer.hpp"
#include "RouteKeyHierarchy.hpp"
#include "RoutingNetwork.hpp"
#include "RoutingNode.hpp"
#include "RoutingLink.hpp"
#include "RoadMap.hpp"
#include "Intersection.hpp"
#include <cfloat>
#include <algorithm>
#include <iostream>

using namespace std;
using BaseStatus    = RouterAStar::NodeStatusAStar;
using NodeStatus    = RouterAStarHierarchy::NodeStatusAStarHierarchy;
using Label         = RouterBase::NodeStatusBase::NodeLabel;
using PriorityQueue = priority_queue<
    BaseStatus*, std::vector<BaseStatus*>, bool (*)(BaseStatus*, BaseStatus*)>;

//======================================================================
void RouterAStarHierarchy::initialize(const RoutingNetwork* network)
{
    // _snodesの設定と_maxNetworkRankの決定
    // Set _snodes and determine _maxNetworkRank
    _snodes.clear();
    _maxNetworkRank                     = 0;
    const RoutingNetwork* targetNetwork = network;

    while (true)
    {
        for (auto itr : targetNetwork->nodes())
        {
            NodeStatus* anode = new NodeStatus(itr.second);
            if (itr.second->upperNode()
                && itr.second->upperNode()->downwardLink())
            {
                // 自分に流入するdownwardLinkがある
                // Has a downwardLink that flows into this node
                anode->setUpperId(itr.second->upperNode()->id());
            }
            else
            {
                anode->setUpperId("-1");
            }
            if (itr.second->downwardLink())
            {
                // 自分から流出するdownwardLinkがある
                // Has a downwardLink that flows out from this node
                anode->setLowerId(itr.second->lowerNode()->id());
            }
            else
            {
                anode->setLowerId("-1");
            }
            _snodes.insert(make_pair(anode->id(), anode));
        }
        if (targetNetwork->upperNetwork())
        {
            targetNetwork = targetNetwork->upperNetwork();
            _maxNetworkRank++;
        }
        else
        {
            break;
        }
    }

    _preferredNetworkRank = _maxNetworkRank;
}

//======================================================================
bool RouterAStarHierarchy::generateRouteCache(
    RoadMap* roadMap, vector<string>& tokens) const
{
    assert(tokens.size() > 5);

    bool         isValidLine = true;
    unsigned int index       = 0;

    // 第1カラムは使用回数
    // First column is number of use
    int count = stoi(tokens[index]);
    index++;

    // 第2カラムは車種ID
    // Second column is vehicle type
    VehicleType type(tokens[index]);
    index++;

    // 第3カラムは選好ネットワークランク
    // Third column is preferred network rank
    unsigned int prefRank = static_cast<unsigned int>(stoi(tokens[index]));
    index++;

    // 第4カラムからは経路探索パラメータ
    // Routing parameters are from the forth column
    double weights[VEHICLE_ROUTING_PARAMETER_SIZE];
    for (unsigned int i = 0; i < VEHICLE_ROUTING_PARAMETER_SIZE; i++)
    {
        weights[i] = stod(tokens[index]);
        index++;
    }

    /*
     * 次のカラムは出発地に至る交差点の識別番号
     *
     * The next column is the Intersection ID number leading to the
     * start point
     */
    const Intersection* past = roadMap->intersection(tokens[index]);
    index++;

    // 次のカラムは出発交差点の識別番号
    // The next column is the start Intersection ID number
    const Intersection* start = roadMap->intersection(tokens[index]);
    index++;

    // 次のカラムは目的交差点の識別番号
    // The next column is the goal Intersection ID number
    const Intersection* goal = roadMap->intersection(tokens[index]);
    index++;

    // 以降のカラムは経路
    // Subsequent columns are route
    Route route;
    for (; index < tokens.size(); index++)
    {
        const Intersection* inter = roadMap->intersection(tokens[index]);
        if (!inter)
        {
            isValidLine = false;
            break;
        }
        route.addIntersection(inter);
    }

    if (!(past && start && goal && isValidLine))
    {
        return false;
    }

    RouteKeyBase* key
        = new RouteKeyHierarchy(type, prefRank, weights, past, start, goal);

    const_cast<Intersection*>(past)
        ->routeCacheContainer()
        ->addRouteCacheDirectly(key, route, count);

    return true;
}

//======================================================================
bool RouterAStarHierarchy::_loadCache(
    RouteCacheContainer* container, const Intersection* from,
    const Intersection* next, const Intersection* goal,
    Route& result_route) const
{
    RouteKeyHierarchy key(
        *_vehicleType, _preferredNetworkRank, _weights, from, next, goal);
    return container->searchRouteCache(_id, &key, result_route);
}

//======================================================================
void RouterAStarHierarchy::_saveCache(
    RouteCacheContainer* container, const Intersection* from,
    const Intersection* next, const Intersection* goal,
    const Route& route) const
{
    RouteKeyHierarchy* key = new RouteKeyHierarchy(
        *_vehicleType, _preferredNetworkRank, _weights, from, next, goal);
    container->addRouteCache(_id, key, route);
}

//======================================================================
double RouterAStarHierarchy::_calcEstimatedCost(
    double fixedCost, RouterAStar::NodeStatusAStar* nextNode,
    const RoutingLink*)
{
    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    double estimatedCost  = fixedCost;
    double distanceToGoal = _minDistToGoals(nextNode->routingNode());

    // 距離のヒューリスティクスh(n) [m] を加算
    // Add distance heuristics h(n) [m]
    estimatedCost += distanceToGoal * _weights[0];

    // 時間のヒューリスティクスh(n) [s] を加算
    // Add time heuristics h(n) [s]
    //   180 [km/h] = 3000 [m/min] = 50 [m/s]
    estimatedCost += distanceToGoal * _weights[1] / 50.0;

    // 他のヒューリスティクスは0
    // Other heuristics are 0.

    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    // ネットワークのランクに応じてペナルティを課す
    // Penalize according to network rank
    int rankDiff = max(
        0,
        _preferredNetworkRank
            - static_cast<int>(nextNode->routingNode()->networkRank()));
    /*
     * - 選好するランクとの差に応じてペナルティが増大(0-100%)
     * - ゴールまでの推定距離に応じてペナルティが増大(直線距離の実数)
     * - 係数0.001はいい加減で調整の余地がある．大きくすると高ランクの
     *   優先が高まり探索が高速になるが，結果として得られる経路長が
     *   非現実的に長くなる可能性がある．
     *
     * - Penalty increases according to the difference from the
     *   preferred rank (0-100%)
     * - Penalty increases with estimated distance to goal (real number
     *   of straight line distance)
     * - A coefficient of 0.001 is an unreliable value and there is room
     *   for adjustment. Larger values ​​give higher priority to higher
     *   ranks and speed up the routing, but the resulting path length
     *   may be unrealistically long.
     */
    /**
     * @todo distanceToGoalの正規化について検討
     */
    double penalty = 0.001 * distanceToGoal * static_cast<double>(rankDiff)
        / _preferredNetworkRank;

    estimatedCost *= (1 + penalty);
    return estimatedCost;
}

//======================================================================
bool RouterAStarHierarchy::_canExpandHorizontally(
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

    // 上位ネットワークであれば展開可能
    // Can be expanded in upper network
    if (node->networkRank() >= static_cast<unsigned int>(_preferredNetworkRank))
    {
        return true;
    }
    else
    {
        /*
         * 上位ネットワークから到達できるノードへは展開する必要がない
         *
         * Not need to expand to the node reachable from the upper
         * network
         */
        const RoutingNode* nextNode  = link->endNode();
        const RoutingLink* upperLink = link->upperLink();
        if (!upperLink)
        {
            return true;
        }

        if (nextNode->upperNode() && nextNode->upperNode()->downwardLink()
            && upperLink->beginNode()->lowerNode() == node
            && upperLink->endNode()->lowerNode() == nextNode)
        {
            return false;
        }
        else
        {
            return true;
        }
    }
    return true;
}

//======================================================================
bool RouterAStarHierarchy::_canExpandUpward(const RoutingNode* node) const
{
    assert(node->upwardLink());

    /*
     * _preferredNetworkRankより上のランクのネットワークへは展開しない
     *
     * Do not expand to network with a rank higher than
     * _preferredNetworkRank
     */
    if (node->upwardLink()->endNode()->networkRank()
        > static_cast<unsigned int>(_preferredNetworkRank))
    {
        return false;
    }
    return true;
}

//======================================================================
void RouterAStarHierarchy::_visitNode(
    RouterAStar::NodeStatusAStar* nextNode, const RoutingLink* nextLink,
    string prevId, double dist, double estimatedDist, double cost,
    double estimatedCost, PriorityQueue& pq)
{
    bool                      propagatesLabel = false;
    NodeStatusAStarHierarchy* nextNodeASH
        = dynamic_cast<NodeStatusAStarHierarchy*>(nextNode);


    switch (nextNodeASH->label())
    {
    case Label::UNREACHED:
        // 必ず更新する
        // Always update
        _updateNode(
            nextNode, prevId, dist, estimatedDist, cost, estimatedCost, pq);
        nextNode->setLabel(Label::LABELED);
        pq.emplace(nextNode);
        /*
         * 下位ノードがあれば，そのラベルをUPPERLABELEDに変更する
         * 必要がある
         *
         * If nextNode has lower nodes, they should be labeled
         * as UPPERLABELED.
         */
        if (nextNodeASH->lowerId() != "-1"
            && nextLink != _snodes[prevId]->routingNode()->upwardLink())
        {
            propagatesLabel = true;
        }
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
        /**
         * @todo 新たなノードのinsertに変更できないか検討
         */
        pq.emplace(nextNode);
        //pq.insert(nextNode);
        break;

    case Label::UPPERLABELED:
        /*
         * 上位ノードが探索済みであるので，downwardLinkのみ処理する
         *
         * Since the upper node has already been searched, only the
         * downwardLink is processed.
         */
        if (nextLink != nextNode->routingNode()->upperNode()->downwardLink())
        {
            return;
        }
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

    // 下位ノードのラベルを更新
    // Update labels of lower nodes
    if (propagatesLabel)
    {
        NodeStatusAStarHierarchy* target = nextNodeASH;
        while (true)
        {
            target = dynamic_cast<NodeStatusAStarHierarchy*>(
                _snodes[target->lowerId()]);

            target->setLabel(Label::UPPERLABELED);
            if (target->lowerId() == "-1")
            {
                break;
            }
        }
    }
}

//======================================================================
void RouterAStarHierarchy::_resetRouting()
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

    // upperId, lowerId, _preferredNetworkRankはそのままでよい
    // upperId, lowerId, _preferredNetworkRank can be left as they are
}

//======================================================================
void RouterAStarHierarchy::_resetStatus()
{
    RouterBase::_resetStatus();
    _preferredNetworkRank = _maxNetworkRank;
}
