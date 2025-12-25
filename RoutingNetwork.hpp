/* ******************************************************************
 * Copyright 2014-2024 ADVENTURE Project
 * All Rights Reserved
 * ****************************************************************** */
/**
 * @file RoutingNetwork.hpp
 */
#ifndef __ROUTING_NETWORK_HPP__
#define __ROUTING_NETWORK_HPP__
#include "Config.hpp"
#include "RoutingLink.hpp"
#include "RoutingNode.hpp"
#include <vector>
#include <map>
#include <unordered_map>
#include <string>

class Section;
class Intersection;

//######################################################################
/**
 * @~japanese 経路探索用ネットワーク
 *
 * @note
 * 元のグラフ G のリンクをノード，ノードをリンクに変換したグラフ L(G) を
 * Gに対する線グラフあるいは線双対グラフと呼ぶ．経路探索用ネットワークは
 * Intersection をノード，Section をリンクとする RoadMap に対する
 * 線グラフである．RoadMap における Section - Intersection - Section の
 * 接続関係を RoutingNode - RoutingLink - RoutingNode に変換することで，
 * RoutingLink に右左折コストを与え，経路探索に利用することができる．
 *
 * @note
 * 階層型のネットワークを構成する．_networkRank が低いネットワークを
 * 下位層とする．下位層のネットワークがより現実の道路ネットワークに近く，
 * 上位層のネットワークは簡略度が高い．
 *
 * @~english  Network for routing
 *
 * @note
 * A graph L(G), whose links correspond to the nodes of the original
 * graph G and whose nodes correspond to the links of G, is called a
 * line graph or a line dual graph for G. The routing network is a line
 * graph for the RoadMap with Intersections as nodes and Sections as
 * links. By converting Section - Intersection - Section connection
 * in RoadMap to RoutingNode - RoutingLink - RoutingNode connection,
 * RoutingLink can be given a right/left turn cost and used for routing.
 *
 * @note
 * Construct a hierarchical network. A network with a low rank is a
 * lower layer network. The lower layer network is closer to the real
 * road network. The higher network has a higher degree of simplicity.
 *
 * @~ @ingroup Routing
 */
class RoutingNetwork
{
public:
    RoutingNetwork();
    ~RoutingNetwork();

    /**
     * @~japanese ノード @p node をコンテナに追加する
     * @~english  Add @p node to the container
     */
    void addNode(RoutingNode* node)
    {
        _nodes.insert(std::make_pair(node->id(), node));
    }

    /**
     * @~japanese 識別番号 @p id のノードを戻す 
     * @~english  Return the node with ID number @p id
     */
    RoutingNode* node(std::string& id) const
    {
        auto itr = _nodes.find(id);
        if (itr != _nodes.end())
        {
            return (*itr).second;
        }
        else
        {
            return nullptr;
        }
    }

    /**
     * @~japanese 単路部 @p section に該当するノードを戻す
     * @~english  Return the node corresponding to @p section
     */
    RoutingNode* convertS2N(const Section* sect, bool isUp) const;

    /**
     * @~japanese リンク @p link をコンテナに追加する 
     * @~english  Add @p link to the container
     */
    void addLink(RoutingLink* link)
    {
        _links.push_back(link);
    }

    /**
     * @~japanese 識別番号 @p id のリンクを戻す
     * @~english  Return the link with ID number @p id
     */
    RoutingLink* link(const std::string& id) const
    {
        for (auto itr : _links)
        {
            if (itr->id() == id)
            {
                return itr;
            }
        }
        return nullptr;
    }

    /**
     * @~japanese
     * 始点 @p begin , 終点 @p end を持つリンクを戻す
     *
     * @~english
     * Return the link with begin point @p begin and end point @p end .
     */
    RoutingLink* link(const RoutingNode* begin, const RoutingNode* end) const
    {
        for (auto itr : _links)
        {
            if (itr->beginNode() == begin && itr->endNode() == end)
            {
                return itr;
            }
        }
        return nullptr;
    }

    /**
     * @~japanese 
     * 交差点 @p via に該当するリンクを戻す
     *
     * 交差点は複数のリンクに変換されるため，上流 @p from と 下流
     * @p to を指定してリンクを特定する．
     *
     * @~english
     * Return the link corresponding to the intersection @p via
     *
     * Because intersections are converted into multiple links, specify
     * upstream @p from and downstream @p to in order to identify the
     * link.
     */
    RoutingLink* convertI2L(
        const Intersection* via, const Intersection* from,
        const Intersection* to) const;

    /**
     * @~japanese 初期リンクコストを付与する
     * @~english  Give initial link costs
     */
    void setInitialCosts()
    {
        for (auto itr : _links)
        {
            itr->setInitialCosts();
        }
    }

    /**
     * @~japanese リンクコストを更新する
     * @~english  Update link costs
     */
    void renewCosts()
    {
        for (auto itr : _links)
        {
            itr->renewDynamicCosts();
        }
    }

private:
    /**
     * @~japanese
     * ネットワークに含まれるノードの最高ランクを調査する
     *
     * @~english
     * Investigate the highest rank of nodes included in the network
     */
    void _checkHighestNodeRank()
    {
        for (auto itr : _nodes)
        {
            if (itr.second->rank() > _highestNodeRank)
            {
                _highestNodeRank = itr.second->rank();
            }
        }
    }

public:
    /**
     * @~japanese 経路探索用ネットワークの情報を表示する
     * @~english  Display routing network information
     */
    void print() const;

private:
    /**
     * @~japanese 経路探索用ノードのコンテナ
     *
     * @attention
     * 階層化する場合は最下層のネットワークがノードを所有する．上位層の
     * ネットワークで new & delete してはならない
     *
     * @~english  Container of network nodes for routing
     *
     * @attention
     * When layered, the lowest layer network owns the nodes. Don't new
     * and delete them in the upper layer network.
     */
    std::unordered_map<std::string, RoutingNode*> _nodes;

    /**
     * @~japanese 経路探索用リンクのコンテナ
     *
     * @note
     * ノードとは違いリンクは各層で異なるため，各層で new & delete する
     *
     * @~english  Container of network link for routing
     *
     * @note
     * Unlike nodes, links are different in each layer, so new & delete
     * them in each layer.
     */
    std::vector<RoutingLink*> _links;

    /**
     * @~japanese ネットワークランク
     * @~english  Network rank
     */
    unsigned int _networkRank;

    /**
     * @~japanese 上位ネットワークへのポインタ
     * @~english  Pointer to the upper network
     */
    RoutingNetwork* _upperNetwork;

    /**
     * @~japanese 下位ネットワークへのポインタ
     * @~english  Pointer to the lower network
     */
    RoutingNetwork* _lowerNetwork;

    /**
     * @~japanese ノードの最高ランク
     *
     * @attention
     * ネットワークに含まれるノードのランクは，ネットワークのランクとは
     * 異なる．
     *
     * @~english  Highest rank of nodes
     *
     * @attention
     * The rank of the nodes included in the network differs from the
     * rank of the network.
     */
    unsigned int _highestNodeRank;

    //==================================================================
    /**
     * @~japanese @name アクセッサ
     * @~english  @name Accessor
     */
    ///@{
public:
    const std::unordered_map<std::string, RoutingNode*>& nodes() const
    {
        return _nodes;
    }

    const std::vector<RoutingLink*>& links() const
    {
        return _links;
    }

    void setNetworkRank(unsigned int rank)
    {
        _networkRank = rank;
    }

    unsigned int networkRank() const
    {
        return _networkRank;
    }

    void setUpperNetwork(RoutingNetwork* upperNetwork)
    {
        _upperNetwork = upperNetwork;
    }

    RoutingNetwork* upperNetwork() const
    {
        return _upperNetwork;
    }

    void setLowerNetwork(RoutingNetwork* lowerNetwork)
    {
        _lowerNetwork = lowerNetwork;
    }

    RoutingNetwork* lowerNetwork() const
    {
        return _lowerNetwork;
    }

    unsigned int highestNodeRank()
    {
        if (_highestNodeRank > 0)
        {
            // _highestNodeRankは調査済み
            // _highestNodeRank has been investigated
        }
        else
        {
            // _highestNodeRankをここで調査する
            // Investigate _highestNodeRank here
            _checkHighestNodeRank();
        }
        return _highestNodeRank;
    }

    ///@}
};

#endif //__ROUTING_NETWORK_HPP__
