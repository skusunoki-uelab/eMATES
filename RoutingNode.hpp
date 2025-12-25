/* ******************************************************************
 * Copyright 2014-2024 ADVENTURE Project
 * All Rights Reserved
 * ****************************************************************** */
/**
 * @file RoutingNode.hpp
 */
#ifndef __ROUTING_NODE_HPP__
#define __ROUTING_NODE_HPP__
#include "Config.hpp"
#include "RoutingProbability.hpp"
#include "VehicleRestriction.hpp"
#include "VehicleTypeManager.hpp"
#include <AmuPoint.hpp>
#include <vector>
#include <string>
#include <map>

class Section;
class RoutingLink;

//######################################################################
/**
 * @~japanese 経路探索用ネットワークにおけるノード
 *
 * RoadMap における Section に相当する．
 *
 * @~english  Node in routing networks
 *
 * Corresponds to Section in RoadMap.
 *
 * @~ @ingroup Routing
 */
class RoutingNode
{
public:
    RoutingNode(const Section* section, bool isUp);
    ~RoutingNode();

    /**
     * @~japanese 自身を終点とするリンク @p を追加する
     * @~english  Add link @p link with this node as an end node 
     */
    void addInLink(RoutingLink* link)
    {
        _inlinks.emplace_back(link);
    }

    /**
     * @~japanese ノード @p prev から自身へのリンクを戻す
     * @~english  Return a link from the node @p prev to this
     */
    RoutingLink* inLink(const RoutingNode* prev) const;

    /**
     * @~japanese
     * 流入リンクのうち，ランク @p rank 以上のノードと接続するリンクの
     * 集合を @p result_links に代入する
     *
     * @~english
     * Assign to the set of inflow links connecting to nodes with rank
     * @p rank or higher to @p result_links
     */
    void getInLinks(
        std::vector<const RoutingLink*>& result_links, unsigned int rank) const;

    /**
     * @~japanese
     * 上流ノードのうち，ランク @p rank 以上のノードの集合を
     * @p result_nodes に代入する
     *
     * @~english
     * Assign the set of upstream nodes with rank @p rank or higher
     * to @p result_nodes
     */
    void getInNodes(
        std::vector<const RoutingNode*>& result_nodes, unsigned int rank) const;

    /**
     * @~japanese
     * 上流ノードのうち，なす角がもっとも小さなノードを戻す
     * 
     * @~english
     * Returns the node with the smallest angle among upstream nodes
     */
    const RoutingNode* straightInNode() const;

    /**
     * @~japanese
     * ランク @p rank のノードからの入次数を戻す
     *
     * @~english
     * Return the indegree from the node with rank @p rank
     */
    int indegree(unsigned int rank) const;

    /**
     * @~japanese 同じ階層のノードからの入次数を戻す
     * @~english  Return indegree from nodes in same network rank
     */
    int indegreeInSameNetworkRank() const;

    /**
     * @~japanese 自分を始点とするリンク @p link を追加する
     * @~english  Add link @p link with this node as a begin node 
     */
    void addOutLink(RoutingLink* link)
    {
        _outlinks.emplace_back(link);
    }

    /**
     * @~japanese 自身からノード @p next へのリンクを戻す
     * @~english  Return a link from this to the node @p next
     */
    RoutingLink* outLink(const RoutingNode* next) const;

    /**
     * @~japanese
     * 流出リンクのうち，ランク @p rank 以上のノードと接続するリンクの
     * 集合を @p result_links に代入する
     *
     * @~english
     * Assign the set of outflow links connecting to nodes with rank
     * @p rank or higher to @p result_links
     */
    void getOutLinks(
        std::vector<const RoutingLink*>& result_links, unsigned int rank) const;

    /**
     * @~japanese
     * 下流ノードのうち，ランク @p rank 以上のノードの集合を
     * @p result_nodes に代入する
     *
     * @~english
     * Assign the set of downstream nodes with rank @p rank or higher
     * to @p result_nodes
     */
    void getOutNodes(
        std::vector<const RoutingNode*>& result_nodes, unsigned int rank) const;

    /**
     * @~japanese
     * 下流ノードのうち，なす角がもっとも小さなノードを戻す
     * 
     * @~english
     * Returns the node with the smallest angle among downstream nodes
     */
    const RoutingNode* straightOutNode() const;

    /**
     * @~japanese
     * ランク @p rank のノードへの出次数を戻す
     *
     * @~english
     * Return the outdegree to the node with rank @p rank
     */
    int outdegree(unsigned int rank) const;

    /**
     * @~japanese 同じ階層のノードへの出次数を戻す
     * @~english  Return outdegree to nodes in same network rank
     */
    int outdegreeInSameNetworkRank() const;

    /**
     * @~japanese リンク属性を設定する
     * @~english  Set node property
     */
    void setProperty();

    /**
     * @~japanese 車種 @p type の通行を許可するか
     * @~english  Whether to permit vehicles of type @p type to pass
     */
    bool permitsPassing(const VehicleType& type) const
    {
        return _restriction.permitsPassing(type);
    }

    /**
     * @~japanese
     * 車種 @p type の車両の経路選択確率を戻す
     *
     * @~english
     * Return routing probability for vehicles of type @p type
     */
    double probability(const VehicleType& type) const
    {
        return _probability.probability(type);
    }

    /**
     * @~japanese ノードの属性を @p out に出力する
     * @~english  Output node properties to @p out
     */
    void print(std::ostream& out) const;

    //==================================================================
private:
    /**
     * @~japanese 識別番号
     *
     * サブ識別番号にランクを表すprefixを付与した番号を識別番号とする．
     *
     * @~english  ID number
     *
     * An ID number is obtained by adding a prefix representing the rank
     * indicating its rank to the sub ID number.
     */
    std::string _id;

    /**
     * @~japanese サブ識別番号
     *
     * 上流と下流の Intersection ID を結合．Section ID に類似するが，
     * 方向別に違うIDが付与される．
     *
     * @~english  Sub ID number
     *
     * Combination of upstream and downstream Intersection IDs. Similar
     * to Section ID, but different IDs are given for  each direction.
     */
    std::string _subId;

    /**
     * @~japanese ノードのランク
     *
     * 経路探索における重要度に相当する．たとえば，幹線道路に相当する
     * ノードのランクは高い．
     *
     * @~english  Node rank
     *
     * Corresponds to the degree of importance in routing. For example,
     * the rank of the node corresponding to the trunk road is high.
     */
    unsigned int _rank;

    /**
     * @~japanese 経路探索用ネットワークのランク
     *
     * ノードが所属する最上位ネットワークのランク．
     *
     * @attention
     * 基本的に _rank == _networkRankであるが，ノードの次数が2で
     * 接続するリンクが縮約される場合は _rank > _networkRank となる．
     * 
     * @~english  Rank of routing network
     *
     * The rank of the highest network rank to which the node belongs.
     *
     * @attention
     * Basically _rank == _networkRank, but if the degree of node is 2
     * and a connecting link is contracted, then _rank > _networkRank.
     */
    unsigned int _networkRank;

    /**
     * @~japanese 該当する単路
     * @~english  Corresponding section
     */
    const Section* _section;

    /**
     * @~japanese
     * 単路の上り方向にあたるか

     * @~english
     * Whether to correspond the ascending direction of the section
     */
    bool _isUp;

    /**
     * @~japanese 自身を終点とするリンク
     * @~english  Links with this node as an end node 
     */
    std::vector<RoutingLink*> _inlinks;

    /**
     * @~japanese 自身を始点とするリンク
     * @~english  Links with this node as an begin node
     */
    std::vector<RoutingLink*> _outlinks;

    /**
     * @~japanese 自身を始点とする，上位ネットワークへのリンク
     * @~english  A upward link with this node as an begin node
     */
    RoutingLink* _upwardLink;

    /**
     * @~japanese 自身を始点とする，下位ネットワークへのリンク
     * @~english  A downward link with this node as an begin node
     */
    RoutingLink* _downwardLink;

    /**
     * @~japanese 上位ノード
     * @~english  Higher-rank node
     */
    RoutingNode* _upperNode;

    /**
     * @~japanese 下位ノード
     * @~english  Lower-rank node
     */
    RoutingNode* _lowerNode;

    /**
     * @~japanese 最下位ノード
     * @~english  Lowest-rank node
     */
    RoutingNode* _lowestNode;

    /**
     * @~japanese 通行権の設定
     * @~english  Right-of-way property
     */
    VehicleRestriction _restriction;

    /**
     * @~japanese 確率の設定
     * @~english  Probability property
     */
    RoutingProbability _probability;

    /**
     * @~japanese 表示に使用する座標
     * @~english  Coordinates used for display
     */
    amu::geometry::AmuPoint _coord;

    //==================================================================
    /**
     * @~japanese @name アクセッサ
     * @~english  @name Accessor
     */
    ///@{
public:
    std::string id() const
    {
        return _id;
    }

    void setId(const std::string& id)
    {
        _id = id;
    }

    std::string subId() const
    {
        return _subId;
    }

    unsigned int rank() const
    {
        return _rank;
    }

    void setRank(unsigned int rank)
    {
        _rank = rank;
    }

    unsigned int networkRank() const
    {
        return _networkRank;
    }

    void setNetworkRank(unsigned int networkRank)
    {
        _networkRank = networkRank;
        setId(
            amu::converter::formatId(
                std::to_string(_networkRank), NUM_FIGURE_FOR_ROUTING_LAYER)
            + _subId);
    }

    const Section* section() const
    {
        return _section;
    }

    bool isUp() const
    {
        return _isUp;
    }

    const std::vector<RoutingLink*>& inLinks() const
    {
        return _inlinks;
    }

    const std::vector<RoutingLink*>& outLinks() const
    {
        return _outlinks;
    }

    RoutingLink* upwardLink() const
    {
        return _upwardLink;
    }

    void setUpwardLink(RoutingLink* link)
    {
        _upwardLink = link;
    }

    RoutingLink* downwardLink() const
    {
        return _downwardLink;
    }

    void setDownwardLink(RoutingLink* link)
    {
        _downwardLink = link;
    }

    RoutingNode* upperNode() const
    {
        return _upperNode;
    }

    void setUpperNode(RoutingNode* node)
    {
        _upperNode = node;
    }

    RoutingNode* lowerNode() const
    {
        return _lowerNode;
    }

    void setLowerNode(RoutingNode* node)
    {
        _lowerNode = node;
    }

    RoutingNode* lowestNode() const
    {
        return _lowestNode;
    }

    void setLowestNode(RoutingNode* node)
    {
        _lowestNode = node;
    }

    const VehicleRestriction& restriction() const
    {
        return _restriction;
    }

    const RoutingProbability& probability() const
    {
        return _probability;
    }

    const amu::geometry::AmuPoint& point() const
    {
        return _coord;
    }

    ///@}
};

#endif //__ROUTING_NODE_HPP__
