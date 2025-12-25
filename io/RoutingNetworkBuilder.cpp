/* ******************************************************************
 * Copyright 2014-2024 ADVENTURE Project
 * All Rights Reserved
 * ****************************************************************** */
/**
 * @file RoutingNetworkBuilder.cpp
 */
#include "RoutingNetworkBuilder.hpp"
#include "../AppMates.hpp"
#include "../CustomMessage.hpp"
#include "../Intersection.hpp"
#include "../RoutingLink.hpp"
#include "../RoutingNetwork.hpp"
#include "../RoutingNode.hpp"
#include "../Section.hpp"
#include <AmuConverter.hpp>
#include <iostream>
#include <cassert>
#include <climits>
#include <sstream>
#include <unordered_set>

using namespace std;
using namespace amu::converter;
using RoutingLinkType = RoutingLink::RoutingLinkType;

//======================================================================
bool RoutingNetworkBuilder::generateRoutingNetworks()
{
    RouterManager& manager = AppMates::getRouterManager();
    /*
     * ランク0の経路探索用ネットワークはRoadMapの線グラフ．
     *   Intersectionがリンク，Sectionがノードに相当する．ランク0の
     *   ネットワークでは縮約を行わない．
     *
     * The network of rank 0 for routing is a line graph of RoadMap.
     *    Intersection and section correspond to a link and a node,
     *    respectively. No contraction in the network with rank 0.
     */
    ostringstream ss0;
    ss0 << "build RoutingNetworks[0] ... ";
    amu::msg::status(cout, ss0.str());

    manager.addRoutingNetwork(_buildRoutingLineGraph());
    unsigned int highestNodeRank = _networks[0]->highestNodeRank();
    ss0 << "done";
    amu::msg::status(cout, ss0.str());

    // ランク1以上のネットワーク
    // Networks of rank 1 or higher
    for (unsigned int i = 1; i <= highestNodeRank; i++)
    {
        ostringstream ss;
        ss << "build RoutingNetworks[" << i << "] ... ";
        amu::msg::status(cout, ss.str());

        manager.addRoutingNetwork(_buildRoutingHigherGraph(i));
        _networks[i - 1]->setUpperNetwork(_networks[i]);
        _networks[i]->setLowerNetwork(_networks[i - 1]);
        ss << "done";
        amu::msg::status(cout, ss.str());
    }

    // ランク間リンクを生成する
    // Generate inter-rank links
    for (unsigned int i = 0; i <= highestNodeRank; i++)
    {
        _generateInterlayerLinks(i);
    }

    // RoutingNode, RoutingLinkの属性を付与する
    // Give properties of RoutingNodes and RoutingLinks
    for (unsigned int i = 0; i <= highestNodeRank; i++)
    {
        _setNodeProperties(i);
        _setLinkProperties(i);
    }

    return true;
}

//======================================================================
RoutingNetwork* RoutingNetworkBuilder::_buildRoutingLineGraph()
{
    RoutingNetwork* network = new RoutingNetwork();
    network->setNetworkRank(0);
    _networks.emplace_back(network);

    // ノード生成
    // Node generation
    if (!_generateRoutingBottomNodes())
    {
        cerr << "ERROR: create routing nodes failed." << endl;
        exit(EXIT_FAILURE);
    }

    // リンク生成
    // Link generation
    if (!_generateRoutingBottomLinks())
    {
        cerr << "ERROR: create routing links failed." << endl;
        exit(EXIT_FAILURE);
    }

    return network;
}

//======================================================================
RoutingNetwork* RoutingNetworkBuilder::_buildRoutingHigherGraph(
    unsigned int networkRank)
{
    RoutingNetwork* network = new RoutingNetwork();
    network->setNetworkRank(networkRank);
    _networks.emplace_back(network);

    // ノード生成
    // Generate nodes
    if (!_generateRoutingHigherNodes(networkRank))
    {
        cerr << "ERROR: create routing nodes failed." << endl;
        exit(EXIT_FAILURE);
    }

    // リンク生成
    // Generate links
    if (!_generateRoutingHigherLinks(networkRank))
    {
        cerr << "ERROR: create routing links failed." << endl;
        exit(EXIT_FAILURE);
    }

    /*
     * ノード昇格
     *   ネットワーク情報をもとに1つ上位のネットワークに含めるべき
     *   ノードを求め，rankを上げる
     *
     * Node promotion
     *   Based on the network information, find nodes that should be
     *   included in the network of higher rank, and raise the rank.
     */
    if (!_promoteRoutingNodes(networkRank))
    {
        cerr << "ERROR: promote nodes failed." << endl;
        exit(EXIT_FAILURE);
    }

    return network;
}

//======================================================================
bool RoutingNetworkBuilder::_generateRoutingBottomNodes()
{
    RoutingNetwork* network = _networks[0];
    assert(network);

    unsigned int numSections  = _roadMap->sections().size();
    unsigned int numProcessed = 0;

    for (auto itr : _roadMap->sections())
    {
        if (_numNodes == ULONG_MAX)
        {
            cerr << "ERROR: number of routing nodes reached ULONG_MAX("
                 << ULONG_MAX << ")" << endl;
            exit(EXIT_FAILURE);
        }

        // 上り方向
        // Up direction
        if (itr.second->lanesWithDirection(true).size())
        {
            RoutingNode* node = new RoutingNode(itr.second, true);
            node->setNetworkRank(0);
            node->setLowestNode(node);
            network->addNode(node);
            itr.second->addRoutingNode(true, node);
            _numNodes++;
        }

        // 下り方向
        // Down direction
        if (itr.second->lanesWithDirection(false).size())
        {
            RoutingNode* node = new RoutingNode(itr.second, false);
            node->setNetworkRank(0);
            node->setLowestNode(node);
            network->addNode(node);
            itr.second->addRoutingNode(false, node);
            _numNodes++;
        }
        numProcessed++;
        if (numProcessed % 1000 == 0)
        {
            ostringstream ss;
            ss << "build RoutingNetworks[0] ... processed " << numProcessed
               << "/" << numSections << " nodes";
            amu::msg::status(cout, ss.str());
        }
    }
    return true;
}
//======================================================================
bool RoutingNetworkBuilder::_generateRoutingHigherNodes(
    unsigned int networkRank)
{
    assert(networkRank > 0);
    RoutingNetwork* network      = _networks[networkRank];
    RoutingNetwork* lowerNetwork = _networks[networkRank - 1];
    assert(network && lowerNetwork);

    /*
     * 指定したランクより1だけ低いランクのノードの集合から，条件を満たす
     * ノードを抽出し指定したランクに登録する
     *
     * Extract nodes that meet the criteria from the set of nodes with
     * a rank lower by 1 than the specified rank and register them at
     * the specified rank.
     */
    for (auto itr : lowerNetwork->nodes())
    {
        RoutingNode* targetNode = itr.second;

        /*
         * 対象ノードのランクが低い場合は上位ネットワークに追加しない
         *
         * If the rank of the target node is lower, not add it to the
         * higher network
         */
        if (targetNode->rank() < networkRank)
        {
            continue;
        }

        // 指定ランク以上のノードからの入次数
        // In-degree from nodes of specified rank or higher
        int indeg = targetNode->indegree(networkRank);

        // 指定ランク以上のノードへの出次数
        // Out-degree to nodes of specified rank or higher
        int outdeg = targetNode->outdegree(networkRank);

        /*
         * 入次数，出次数がともに0の場合は対象ノードを上位ネットワークに
         * 追加しない
         *
         * In both the in-degree and out-degree are 0, the target node 
         * is not added to the higher network.
         */
        if (indeg == 0 && outdeg == 0)
        {
            continue;
        }

        /*
         * 入次数，出次数がともに1であり，かつ対象ノードを上流の出次数，
         * 下流の入次数もともに1である場合は上位ネットワークに追加しない
         *
         * In both the in-degree and out-degree are 1, and the out-
         * degree of the upstream and the in-degree of the downstream
         * node are also 1, the target node is not added to the higher
         * network.
         */
        if (indeg == 1 && outdeg == 1)
        {
            vector<const RoutingNode*> upNodes;
            targetNode->getInNodes(upNodes, networkRank);
            assert(upNodes.size() == 1);
            int upOutdeg = upNodes[0]->outdegree(networkRank);

            vector<const RoutingNode*> downNodes;
            targetNode->getOutNodes(downNodes, networkRank);
            assert(downNodes.size() == 1);
            int downIndeg = downNodes[0]->indegree(networkRank);

            if (upOutdeg == 1 && downIndeg == 1)
            {
                continue;
            }
        }

        // 対象ノードを上位のネットワークに登録する
        // Register the target node to the higher network
        RoutingNode* upperNode
            = new RoutingNode(targetNode->section(), targetNode->isUp());
        upperNode->setNetworkRank(networkRank);

        // 上位のノードに下位のノードを登録する
        // Register the lower and the lowest nodes to the higher node
        upperNode->setLowerNode(targetNode);
        upperNode->setLowestNode(targetNode->lowestNode());

        // 下層のノードに上層のノードを登録する
        // Register the higher node to the lower node
        targetNode->setUpperNode(upperNode);

        network->addNode(upperNode);
    }

    return true;
}

//======================================================================
bool RoutingNetworkBuilder::_generateRoutingBottomLinks()
{
    RoutingNetwork* network = _networks[0];
    assert(network);

    int numLinks = 0;

    const unordered_map<string, RoutingNode*>& nodes        = network->nodes();
    unsigned int                               numNodes     = nodes.size();
    unsigned int                               numProcessed = 0;

    for (auto itr : nodes)
    {
        RoutingNode*   beginNode   = itr.second;
        const Section* thisSection = beginNode->section();

        Intersection* via = thisSection->intersection(beginNode->isUp());

        if (via->numNexts() == 1)
        {
            // ODNodeはリンク化しない
            // Not convert an ODNode to a link
            continue;
        }

        /*
         * 次数2以上の場合は交差点"via"の先に必ず経路探索用ノードがある
         *
         * If the degree is 2 or more, there is always a RoutingNode
         * beyond the intersection "via".
         */
        for (int j = 0; j < via->numNexts(); j++)
        {
            Section*            nextSection = via->nextSection(j);
            const Intersection* from = thisSection->anotherIntersection(via);
            const Intersection* to   = nextSection->anotherIntersection(via);
            if (!(via->hasValidPath(from, to)))
            {
                // 交差点内で到達不可能な場合はリンクしない
                // Not link if unreachable in the intersection
                continue;
            }

            bool         isUp    = nextSection->isUp(via, to);
            RoutingNode* endNode = network->convertS2N(nextSection, isUp);

            RoutingLink* link = new RoutingLink(via, beginNode, endNode);
            link->setRank(0);
            link->setLinkType(RoutingLinkType::FLAT);
            via->addRoutingLink(link);

            // サブIDの付与
            // Give sub ID number
            string beginId = beginNode->subId();
            string endId   = endNode->subId();
            string subId   = beginId.substr(0, NUM_FIGURE_FOR_INTERSECTION)
                + endId.substr(
                    NUM_FIGURE_FOR_INTERSECTION, NUM_FIGURE_FOR_INTERSECTION);
            link->setSubId(subId);

            numLinks++;
            network->addLink(link);

            beginNode->addOutLink(link);
            endNode->addInLink(link);
        }
        numProcessed++;
        if (numProcessed % 1000 == 0)
        {
            ostringstream ss;
            ss << "build RoutingNetworks[0] ... processed " << numProcessed
               << "/" << numNodes << " links";
            amu::msg::status(cout, ss.str());
        }
    }
    return true;
}

//======================================================================
bool RoutingNetworkBuilder::_generateRoutingHigherLinks(
    unsigned int networkRank)
{
    int numLinks = 0;

    assert(networkRank > 0);
    RoutingNetwork* network      = _networks[networkRank];
    RoutingNetwork* lowerNetwork = _networks[networkRank - 1];
    assert(network && lowerNetwork);

    for (auto itr : network->nodes())
    {
        assert(itr.second->lowerNode());

        // inlinkとoutlinkのどちらかのみの処理でよい
        // Either inlink or outlink should be processed
        const vector<RoutingLink*>& lowerOutLinks
            = itr.second->lowerNode()->outLinks();
        if (lowerOutLinks.empty())
        {
            continue;
        }

        for (auto itr_l : itr.second->lowerNode()->outLinks())
        {
            assert(itr_l);
            //RoutingLink* lowerOutLink = itr_l;
            const RoutingNode* nextNode = itr_l->endNode();

            //----------------------------------------------------------
            // 隣接ノードのランクが低い場合は何もしない
            // Do nothing if the rank of the adjacent node is low
            if (nextNode->rank() < networkRank)
            {
                continue;
            }

            //----------------------------------------------------------
            /*
             * 隣接ノード自体のランクは高いが上位ノードがない場合は
             * ノードが縮約されているため，接続先を探す
             *
             * Search for the connected node if the rank of adjacent
             * node is high but there is no higher node, because the
             * node is contracted.
             */
            else if (nextNode->upperNode() == NULL)
            {
                vector<const RoutingNode*> abbrNodes;
                vector<const RoutingLink*> abbrLinks;

                vector<const RoutingLink*> tmpInLinks;
                nextNode->getInLinks(tmpInLinks, networkRank);
                assert(tmpInLinks.size() == 1);
                abbrLinks.push_back(tmpInLinks[0]);

                while (true)
                {
                    if (nextNode->upperNode() != NULL)
                    {
                        // 縮約されていないノードが見つかった
                        // A non-contracted node is found
                        break;
                    }

                    // 縮約されたノードとリンクを追加
                    // Add contracted nodes and links
                    vector<const RoutingLink*> tmpOutLinks;
                    nextNode->getOutLinks(tmpOutLinks, networkRank);
                    assert(tmpOutLinks.size() == 1);
                    abbrNodes.push_back(nextNode);
                    abbrLinks.push_back(tmpOutLinks[0]);

                    for (unsigned int j = 0; j < tmpOutLinks.size(); j++)
                    {
                        // nextLinks->size()は1であるはず
                        // nextLinks->size() must be 1.
                        if (tmpOutLinks[j]->endNode()->rank() >= networkRank)
                        {
                            nextNode = tmpOutLinks[j]->endNode();
                            break;
                        }
                    }
                }
                RoutingLink* link = new RoutingLink(
                    nullptr, itr.second, nextNode->upperNode());
                link->setRank(networkRank);
                link->setLinkType(RoutingLinkType::FLAT);

                // サブIDの付与
                // Give sub ID number
                string beginId = itr.second->subId();
                string endId   = nextNode->subId();
                string subId   = beginId.substr(0, NUM_FIGURE_FOR_INTERSECTION)
                    + endId.substr(
                        NUM_FIGURE_FOR_INTERSECTION,
                        NUM_FIGURE_FOR_INTERSECTION);
                link->setSubId(subId);

                for (unsigned int i = 0; i < abbrNodes.size(); i++)
                {
                    link->addLowerNode(abbrNodes[i]);
                }
                for (unsigned int i = 0; i < abbrLinks.size(); i++)
                {
                    link->addLowerLink(abbrLinks[i]);
                    const_cast<RoutingLink*>(abbrLinks[i])->setUpperLink(link);
                }

                numLinks++;
                network->addLink(link);

                itr.second->addOutLink(link);
                const_cast<RoutingNode*>(nextNode->upperNode())
                    ->addInLink(link);
            }

            //----------------------------------------------------------
            // 隣接ノードのランクが高い場合は直接接続する
            // Directly connect if the rank of adjacent node is higher.
            else
            {
                RoutingLink* link = new RoutingLink(
                    itr_l->intersection(), itr.second, nextNode->upperNode());
                link->setRank(networkRank);
                link->setLinkType(RoutingLinkType::FLAT);

                // サブIDの付与
                // Give sub ID number
                string beginId = itr.second->subId();
                string endId   = nextNode->upperNode()->subId();
                string subId   = beginId.substr(0, NUM_FIGURE_FOR_INTERSECTION)
                    + endId.substr(
                        NUM_FIGURE_FOR_INTERSECTION,
                        NUM_FIGURE_FOR_INTERSECTION);
                link->setSubId(subId);

                link->addLowerLink(itr_l);
                itr_l->setUpperLink(link);

                numLinks++;
                network->addLink(link);

                itr.second->addOutLink(link);
                const_cast<RoutingNode*>(nextNode->upperNode())
                    ->addInLink(link);
            }
        }
    }
    return true;
}

//======================================================================
bool RoutingNetworkBuilder::_promoteRoutingNodes(unsigned int networkRank)
{
    // ランク1以上のネットワークが対象
    // For rank 1 and above networks
    if (networkRank < 2)
    {
        return true;
    }

    RoutingNetwork* network = _networks[networkRank];
    assert(network);

    bool                 result = true;
    vector<RoutingNode*> promotedNodes;

    for (auto itr : network->nodes())
    {
        const RoutingNode* target = itr.second;

        /*
         * 上位ネットワークに含まれないノードは起点の対象外
         *
         * Nodes that are not included in the higher network are
         * excluded from the starting points.
        */
        if (target->rank() < networkRank + 1)
        {
            continue;
        }

        //--------------------------------------------------------------
        // 下流方向へ拡張
        // Expand to downstream
        bool canPromote = false;

        if (target->straightOutNode()
            && target->straightOutNode()->rank() <= networkRank)
        {
            /*
             * 直進下流のノードのランクが低い場合は起点の候補
             *
             * If the rank of the straight downstream node is lower
             * than that of the featured node, the featured node is
             * a candidate for the starting point.
             */
            canPromote = true;
        }
        else
        {
            /*
             * 上位ネットワークにおける入次数，出次数
             *
             * In-degree and out-degree of the featured node in 
             * higher-rank network.
             */
            int indeg  = target->indegree(networkRank + 1);
            int outdeg = target->outdegree(networkRank + 1);

            /*
             * 入次数が出次数より高くなければ起点にならない
             *   縮約対象のノードはindeg==outdeg==1であるのでここで除外
             *
             * It can be a starting point only if its in-degree is
             * higher than its out-degree,
             *   Since the node to be contracted is indeg==outdeg==1,
             *   it is excluded here
             */
            if (indeg > outdeg)
            {
                canPromote = true;
            }
        }

        //--------------------------------------------------------------
        /*
         * 対象ノードを通り直進するネットワークのノードを次々に取得し，
         * 指定ホップ数までの間に探索の先頭が始点より上位のランクを持つ
         * ノードに到達したら昇格させる
         *
         * Find the nodes of the network that goes straight through
         * the target node one after another, and promote them when the
         * head of the search reaches a node with a higher rank than
         * the starting point within the given number of hops.
         */
        vector<RoutingNode*> candidates;
        if (canPromote)
        {
            canPromote = false;
            for (int hop = 0; hop < 20; hop++)
            {
                const RoutingNode* nextTarget = target->straightOutNode();

                if (!nextTarget)
                {
                    break;
                }

                /*
                 * 起点自体(hop==0)は上位ノードが存在するため除外
                 *
                 * The starting point itself (hop==0) is excluded
                 * because it has an higher node.
                 */
                if (hop != 0)
                {
                    candidates.push_back(const_cast<RoutingNode*>(target));
                }

                // 上位ノードが存在するノードに到達したら探索を終了
                // End search when reaching a node with a higher node
                if (nextTarget->rank() >= networkRank + 1)
                {
                    canPromote = true;
                    break;
                }

                target = nextTarget;
            }
        }

        if (canPromote)
        {
            // ノード昇格のための登録
            // Registration for node promotion
            for (unsigned int i = 0; i < candidates.size(); i++)
            {
                promotedNodes.push_back(candidates[i]);
            }
        }
    }

    //------------------------------------------------------------------
    // ノードの昇格
    // Node promotion
    for (unsigned int i = 0; i < promotedNodes.size(); i++)
    {
        RoutingNode* prmNode = promotedNodes[i];
        if (prmNode->rank() == networkRank + 1)
        {
            // 既に昇格済み
            // Already promoted
            continue;
        }
        while (prmNode)
        {
            prmNode->setRank(networkRank + 1);
            prmNode = prmNode->lowerNode();
        }
    }

    return result;
}

//======================================================================
bool RoutingNetworkBuilder::_generateInterlayerLinks(unsigned int networkRank)
{
    bool result = true;

    RoutingNetwork* network = _networks[networkRank];
    assert(network);

    const unordered_map<string, RoutingNode*>& nodes = network->nodes();

    //------------------------------------------------------------------
    // 下位→上位へのリンク
    // Upward Link
    for (auto itr : nodes)
    {
        RoutingNode* upperNode = itr.second->upperNode();
        if (upperNode)
        {
            RoutingLink* upwardLink
                = new RoutingLink(nullptr, itr.second, upperNode);
            upwardLink->setRank(networkRank);
            upwardLink->setLinkType(RoutingLinkType::UPWARD);

            // サブIDの付与
            // Give sub ID number
            string beginId = itr.second->subId();
            string endId   = upperNode->subId();
            string subId   = beginId.substr(0, NUM_FIGURE_FOR_INTERSECTION)
                + endId.substr(
                    NUM_FIGURE_FOR_INTERSECTION, NUM_FIGURE_FOR_INTERSECTION);
            upwardLink->setSubId(subId);

            // リンクを登録する
            // Register generated link
            network->addLink(upwardLink);
            itr.second->addOutLink(upwardLink);
            itr.second->setUpwardLink(upwardLink);
            upperNode->addInLink(upwardLink);
        }
    }

    if (networkRank == 0)
    {
        return result;
    }
    RoutingNetwork* lowerNetwork = _networks[networkRank - 1];
    assert(lowerNetwork);

    //------------------------------------------------------------------
    // 上位→下位へのリンク
    // Downward link
    for (auto itr : nodes)
    {
        RoutingNode* lowerNode = itr.second->lowerNode();
        assert(lowerNode);

        /*
         * 以下のいずれかの場合を満たす場合にのみ下位ノードへのリンク
         * 生成フラグを立てる
         * - 下位ノードがさらに下位へのリンクを持つ場合
         * - networkRankとnetworkRank-1で出次数が異なる場合
         * - 下流リンクのうちいずれかが集約リンクである場合
         *
         * Enable flag to generate a link to a lower node if any of
         * the following conditions are satisfied
         * - If the lower node has a link to the further lower node,
         * - If the out-degree of networkRank and networkRank-1 are
         *   different, or
         * - Any of the lower links are aggregated links
         */
        bool buildsLink = false;
        if (!buildsLink)
        {
            if (lowerNode->downwardLink())
            {
                buildsLink = true;
            }
        }
        if (!buildsLink)
        {
            if (itr.second->outdegreeInSameNetworkRank()
                != lowerNode->outdegreeInSameNetworkRank())
            {
                buildsLink = true;
            }
        }
        if (!buildsLink)
        {
            const vector<RoutingLink*>& outlinks = itr.second->outLinks();
            for (unsigned int i = 0; i < outlinks.size(); i++)
            {
                if (outlinks[i]->isAggregatedLink())
                {
                    buildsLink = true;
                    break;
                }
            }
        }

        // フラグが立てられていたらリンクを生成する
        // Generate link if flagged
        if (buildsLink)
        {
            RoutingLink* downwardLink
                = new RoutingLink(nullptr, itr.second, lowerNode);
            downwardLink->setRank(networkRank);
            downwardLink->setLinkType(RoutingLinkType::DOWNWARD);

            // サブIDの付与
            // Give sub ID number
            string beginId = itr.second->subId();
            string endId   = lowerNode->subId();
            string subId   = beginId.substr(0, NUM_FIGURE_FOR_INTERSECTION)
                + endId.substr(
                    NUM_FIGURE_FOR_INTERSECTION, NUM_FIGURE_FOR_INTERSECTION);
            downwardLink->setSubId(subId);

            // リンクを登録する
            // Register generated link
            network->addLink(downwardLink);
            itr.second->addOutLink(downwardLink);
            itr.second->setDownwardLink(downwardLink);
            lowerNode->addInLink(downwardLink);
        }
    }

    return result;
}

//======================================================================
bool RoutingNetworkBuilder::_setNodeProperties(unsigned int networkRank)
{
    const unordered_map<string, RoutingNode*>& nodes
        = _networks[networkRank]->nodes();

    for (auto itr : nodes)
    {
        itr.second->setProperty();
    }
    return true;
}

//======================================================================
bool RoutingNetworkBuilder::_setLinkProperties(unsigned int networkRank)
{
    // サブ識別番号の重複を調べるために使われるコンテナ
    // Container used to check for duplication of sub-ID numbers
    unordered_multiset<string> keys;

    for (auto itr : _networks[networkRank]->links())
    {
        string key = itr->subId();
        string id //
            = formatId(to_string(itr->rank()), NUM_FIGURE_FOR_ROUTING_LAYER)
            + formatId(to_string(keys.count(key)), NUM_FIGURE_FOR_SAME_SUBID)
            + itr->subId();
        keys.insert(key);

        itr->setId(id);
        itr->setProperty();
    }
    return true;
}
