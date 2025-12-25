/* ******************************************************************
 * Copyright 2014-2024 ADVENTURE Project
 * All Rights Reserved
 * ****************************************************************** */
/**
 * @file RoutingNetworkBuilder.hpp
 */
#ifndef __ROUTING_NETWORK_BUILDER_HPP__
#define __ROUTING_NETWORK_BUILDER_HPP__
#include "../Config.hpp"
#include "../RoadMap.hpp"
#include "../RoutingNetwork.hpp"
#include <vector>

//######################################################################
/**
 * @~japanese 経路探索用ネットワークを作成する
 * @~english  Build routing network
 * @~ @ingroup Initialization Routing
 */
class RoutingNetworkBuilder
{
public:
    RoutingNetworkBuilder(RoadMap* roadMap)
    {
        _roadMap  = roadMap;
        _numNodes = 0;
        _numLinks = 0;
        _networks.clear();
    }
    ~RoutingNetworkBuilder()
    {
        _networks.clear();
    }

    /**
     * @~japanese 経路探索用ネットワークを生成する
     * @~english  Generate routing networks
     */
    bool generateRoutingNetworks();

private:
    /**
     * @~japanese 経路探索用の線グラフを作成する
     *
     * @note
     * 線双対グラフと呼ばれることもある．グラフGのリンクが線グラフL(G)の
     * ノードであり，Gにおいてノードを共有するリンクについて，対応する
     * L(G)のノード間にリンクを張る．
     *
     * @~english  Generate line graph for routing
     *
     * @note
     * Line graph is sometimes called line dual graph. Links in graph G
     * are nodes in line graph L(G), and for links sharing nodes in G,
     * create links between the corresponding nodes in L(G).
     */
    RoutingNetwork* _buildRoutingLineGraph();

    /**
     * @~japanese 経路探索用の上位ネットワークを作成する
     * @~english  Generate routing network of higher rank 
     */
    RoutingNetwork* _buildRoutingHigherGraph(unsigned int networkRank);

    /**
     * @~japanese ランク0のネットワークに属するノードを生成する
     * @~english  Generate nodes belonging to network of rank zero
     */
    bool _generateRoutingBottomNodes();

    /**
     * @~japanese @p networkRank のネットワークに属するノードを生成する
     * @~english  Generate nodes belonging to network of @p networkRank
     */
    bool _generateRoutingHigherNodes(unsigned int networkRank);

    /// ノードのランクを昇格する
    /**
     * 上位ネットワークを拡張する
     */
    bool _promoteRoutingNodes(unsigned int networkRank);

    /**
     * @~japanese ランク0のネットワークに属するリンクを生成する
     * @~english  Generate links belonging to network of rank zero
     */
    bool _generateRoutingBottomLinks();

    /**
     * @~japanese @p networkRank のネットワークに属するリンクを生成する
     * @~english  Generate links belonging to network of @p networkRank
     */
    bool _generateRoutingHigherLinks(unsigned int networkRank);

    /**
     * @~japanese ランク間のリンクを生成する
     *
     * @note
     * ランク間リンクは始点側のネットワークランクに属するものとする
     *
     * @~english  Generate links between ranks
     *
     * @note
     * Inter-rank links shall belong to the originating rank.
     */
    bool _generateInterlayerLinks(unsigned int networkRank);

    /**
     * @~japanese
     * @p networkRank のネットワークに属するノードの属性を設定する
     *
     * @~english
     * Set properties for nodes belonging to network of @p networkRank
     */
    bool _setNodeProperties(unsigned int networkRank);

    /**
     * @~japanese
     * @p networkRank のネットワークに属するリンクの属性を設定する
     *
     * @~english
     * Set properties for links belonging to network of @p networkRank
     */
    bool _setLinkProperties(unsigned int networkRank);

private:
    /**
     * @~japanese 道路地図オブジェクト
     * @~english  RoadMap object
     */
    RoadMap* _roadMap;

    /**
     * @~japanese これまでに作成した経路探索用ネットワークのコンテナ
     *
     * @attention
     * インデックスとnetworkRankが等しくなければならない
     *
     * @~english  Container for network for routing already generated
     *
     * @attention
     * Index and networkRank must be equal.
     */
    std::vector<RoutingNetwork*> _networks;

    /**
     * @~japanese ノード数
     * @~english  Number of nodes
     */
    ulint _numNodes;

    /**
     * @~japanese リンク数
     * @~english  Number of links
     */
    ulint _numLinks;
};

#endif //__ROUTING_NETWORK_BUILDER_HPP__
