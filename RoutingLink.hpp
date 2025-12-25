/* ******************************************************************
 * Copyright 2014-2024 ADVENTURE Project
 * All Rights Reserved
 * ****************************************************************** */
/**
 * @file RoutingLink.hpp
 */
#ifndef __ROUTING_LINK_HPP__
#define __ROUTING_LINK_HPP__
#include "Config.hpp"
#include "RoutingProbability.hpp"
#include "VehicleRestriction.hpp"
#include "VehicleTypeManager.hpp"
#include <string>
#include <vector>
#include <map>

class Section;
class Intersection;
class RoutingNode;

//##############################################################################
/**
 * @~japanese 経路探索用ネットワークにおけるリンク
 *
 * RoadMap における Intersection に相当する．
 *
 * @note
 * I0(1)->S0->I1(2)->S1->I2(2)->S2->I3(4)(※)のように接続する場合，両端で次数が
 * 2の Intersection と接続する Section は縮約される．つまり [I1(2)->S1->I2(2)]
 * が1つの RoutingLink となる．このときメンバ変数 _inter は上流の Intersection
 * I1 を指す．
 * ※I0(1)とは，Intersection[ID:0]の次数が1であることを表す．
 *
 * @~english  Links in routing networks
 *
 * Corresponds to Intersection in RoadMap.
 *
 * @note
 * When connecting like I0(1)->S0->I1(2)->S1->I2(2)->S2->I3(4)(*), the section
 * connected with intersections of degree 2 at both ends is contracted. In other
 * words, [I1(2)->S1->I2(2)] is treated as one routing link . At this time,
 * member variable _inter points to upstream intersection I1.
 * * Note that I0(1) means that the degree of Intersection[ID:0] is 1.
 *
 * @~ @ingroup Routing
 */
class RoutingLink
{
public:
    /**
     * @~japanese リンクの種別の列挙
     *
     * 順に，同一ランク内のリンク，上位層へのリンク，下位層へのリンクを表す
     *
     * @~english  Enumeration of link types
     *
     * Link within the same rank, link to upper layer, link to lower
     * layer, in order.
     */
    enum class RoutingLinkType : unsigned int
    {
        FLAT,
        UPWARD,
        DOWNWARD
    };

    //==========================================================================
    /**
     * @~japanese コンストラクタ
     * @note IDは後で付ける．
     *
     * @~english  Constructor
     * @note An ID number is assigned later.
     */
    RoutingLink(
        const Intersection* inter, const RoutingNode* begin,
        const RoutingNode* end);
    ~RoutingLink() {}

    /**
     * @~japanese 下位ノード @p node を追加する
     * @~english  Add lower node @p node
     */
    void addLowerNode(const RoutingNode* node)
    {
        _lowerNodes.emplace_back(node);
    }

    /**
     * @~japanese 下位リンク @p link を追加する
     * @~english  Add lower link @p link
     */
    void addLowerLink(const RoutingLink* link)
    {
        _lowerLinks.emplace_back(link);
    }

    /**
     * @~japanese このリンクが集約リンクかどうか
     *
     * 内部に複数のリンクを内包するかどうかで判別する
     *
     * @~english  Whether this link is an aggregate link
     *
     * Determine if multiple links are included inside.
     */
    bool isAggregatedLink() const
    {
        if (_lowerLinks.size() == 0)
        {
            return false;
        }
        else if (_lowerLinks.size() >= 2)
        {
            return true;
        }
        else
        {
            // _lowerLinks.size()==1
            return _lowerLinks[0]->isAggregatedLink();
        }
    }

    /**
     * @~japanese リンク属性を設定する
     * @~english  Set link property
     */
    void setProperty();

    /**
     * @~japanese @name setProperty() から呼ばれる関数群
     * @~english  @name Functions called form setProperty()
     */
    ///@{
private:
    /**
     * @~japanese ランク0のリンク属性を直接求める
     * @~english  Directly obtain link property of rank 0
     */
    void _setPropertyDirectly();

    /**
     * @~japanese 上位ランクのリンク属性は下位リンクから求める
     * @~english  Obtain higher-rank link property from lower-rank link
     */
    void _setPropertyOfHigherRank();


    /**
     * @~japanese 集約リンクを展開して含まれる RoutingNode を保存する
     * @~english  Expand aggregated link and save RoutingNodes it contains
     */
    void _setAbbreviatedNodes();

    ///@}

public:
    /**
     * @~japanese
     * コストの更新が必要なリンクとして自身を RouterManager に登録する
     *
     * @~english
     * Register this link in RouterManager as a link that should be update costs
     */
    void registerToBeUpdated() const;

    /**
     * @~japanese リンクコストを初期化する
     *
     * @note
     * シミュレーション開始時に呼ばれる．シミュレーション中は動的コストのみ更新
     * される．
     *
     * @~english  Initialize all link costs
     *
     * @note
     * Called when the simulation starts. Only the dynamic costs are updated
     * during the simulation.
     */
    void setInitialCosts();

    /**
     * @~japanese 動的コストを更新する
     * @~english  Update dynamic costs
     */
    void renewDynamicCosts();

    /**
     * @~japanese 時間コストを @p time に更新する
     * @~english  Update time costs to @p time
     */
    void renewTimeCost(const double time)
    {
        _costs[amu::converter::toUnderlying(RoutingParamIndex::TIME)] = time;
    }

    /**
     * @~japanese 車種 @p type の通行を許可するか
     * @~english  Whether to permit vehicles of type @p type to pass
     */
    bool permitsPassing(const VehicleType& type) const
    {
        return _restriction.permitsPassing(type);
    }

    /**
     * @~japanese 車種 @p type の車両の経路選択確率を戻す
     * @~english  Return routing probability for vehicles of type @p type
     */
    double probability(const VehicleType& type) const
    {
        return _probability.probability(type);
    }

    /**
     * @~japanese リンクの属性を @p out に出力する
     * @~english  Output link properties to @p out
     */
    void print(std::ostream& out) const;

    //==========================================================================
private:
    /**
     * @~japanese 識別番号
     *
     * サブ識別番号にランクと重複識別用番号を表すprefixを付与した番号をリンクの
     * 識別番号とする．
     *
     * @note
     * リンクの縮約により，高位のネットワークには同じサブ識別番号を持つリンクが
     * できる可能性がある．そこで，サブ識別番号が重複するたびにインクリメント
     * される番号を「重複識別用番号」として付与する．
     *
     * @~english  ID number
     *
     * The link ID number is the sub-ID number with a prefix indicating its rank
     * and the number to distinguish duplication.

     * @note
     * Due to link reduction, there is a possibility that there will be links
     * with the same sub-ID number in the higher-level network. Therefore, a
     * number that increments each time of sub-ID number duplication is assigned
     * as a "number to distinguish duplication."
     */
    std::string _id;

    /**
     * @~japanese サブ識別番号
     *
     * _beginNodeのサブ識別番号の前半+_endNodeのサブ識別番号の後半．すなわち，
     * このリンクを経由して接続することになる Intersectionの識別番号を結合した
     * もの．
     *
     * @~english  Sub-ID number
     *
     * Concatenation of the first half of the _beginNode sub-ID number and the
     * second half of the _endNode sub-ID number. That is, a concatenation of
     * the ID of the intersections that are connected via this link.
     */
    std::string _subId;

    /**
     * @~japanese リンクのランク
     *
     * @note
     * RoutingNodeと異なり各NetworkRankでnewされるため，リンクのランクと
     * ネットワークのランクは必ず等しい
     *
     * @~english  Link rank
     *
     * @note
     * Unlike RoutingNode, this instance is new at each NetworkRank, so the rank
     * of the link and the rank of the network are always equal.
     */
    unsigned int _rank;

    /**
     * @~japanese リンクの種別
     * @~english  Link type
     */
    RoutingLinkType _linkType;

    /**
     * @~japanese 該当する交差点
     * @note 縮約したリンクの場合には nullptr が入る．
     *
     * @~english  Corresponding intersection
     * @note Contains nullptr for contracted links.
     */
    const Intersection* _inter;

    /**
     * @~japanese 上位のリンク
     * @~english  Higher-rank link
     */
    const RoutingLink* _upperLink;

    /**
     * @~japanese 始点
     * @~english  Begin node
     */
    const RoutingNode* _beginNode;

    /**
     * @~japanese 終点
     * @~english  End node
     */
    const RoutingNode* _endNode;

    /**
     * @~japanese 対応する下位のノード
     *
     * 縮約したノードが入る．縮約しない場合は空リスト．
     *
     * @~english  Corresponding lower-rank nodes
     *
     * Contains the contracted node. Empty if not contracted.
     */
    std::vector<const RoutingNode*> _lowerNodes;

    /**
     * @~japanese 対応する下位のリンク
     *
     * 縮約した場合には複数のリンクが入る
     *
     * @~english  Corresponding lower-rank links
     *
     * Multiple links if contracted
     */
    std::vector<const RoutingLink*> _lowerLinks;

    /**
     * @~japanese 対応する最下位のノード
     * @attention _beginNode, _endNode を含まない
     *
     * @~english  Corresponding lowest-rank nodes
     * @attention Not include _beginNode, _endNode.
     */
    std::vector<const RoutingNode*> _includedLowestNodes;

    /**
     * @~japanese すべての集約リンクを展開して得られるノード
     * @~english  All nodes included this link by expanding all aggregate links
     */
    std::vector<const RoutingNode*> _abbreviatedNodes;

    /**
     * @~japanese リンクコスト
     * @~english  Link costs
     * @~ @see RoutingParamIndex
     */
    double _costs[VEHICLE_ROUTING_PARAMETER_SIZE];

    // [eMATES] 2025/5/30 by abe CS経由コスト
    // このRoutingLinkが表現するノードがCSNodeFastの場合、大きなコストを設定し、不必要なCS進入を防ぐ
    double _csCost {0};

    /**
     * @~japanese リンクの長さ
     * @~english  Link length
     */
    double _length;

    /**
     * @~japanese リンクが交差点の直進を含むか
     * @~english  Whether the link include straight through intersection
     */
    bool _includesStraightDriving;

    /**
     * @~japanese リンクが交差点の左折を含むか
     * @~english  Whether the link include left turn at intersection
     */
    bool _includesLeftTurn;

    /**
     * @~japanese リンクが交差点の右折を含むか
     * @~english  Whether the link include right turn at intersection
     */
    bool _includesRightTurn;

    /**
     * @~japanese
     * リンクに対応する単路部の車線数の最小値
     *
     * @~english
     * Minimum number of lanes for sections corresponding to the link
     */
    unsigned int _width;

    /**
     * @~japanese 通行権の設定
     * @~english  Right-op-way property
     */
    VehicleRestriction _restriction;

    /**
     * @~japanese 確率の設定
     * @~english  Probability property
     */
    RoutingProbability _probability;

    //==========================================================================
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

    void setSubId(const std::string& subId)
    {
        _subId = subId;
    }

    unsigned int rank() const
    {
        return _rank;
    }

    void setRank(unsigned int rank)
    {
        _rank = rank;
    }

    void setLinkType(RoutingLinkType type)
    {
        _linkType = type;
    }

    const Intersection* intersection() const
    {
        return _inter;
    }

    const RoutingLink* upperLink() const
    {
        return _upperLink;
    }

    void setUpperLink(const RoutingLink* upperLink)
    {
        _upperLink = upperLink;
    }

    const RoutingNode* beginNode() const
    {
        return _beginNode;
    }

    const RoutingNode* endNode() const
    {
        return _endNode;
    }

    const std::vector<const RoutingNode*>& includedLowestNodes() const
    {
        return _includedLowestNodes;
    }

    const std::vector<const RoutingNode*>& abbreviatedNodes()
    {
        if (_abbreviatedNodes.empty())
        {
            _setAbbreviatedNodes();
        }
        return _abbreviatedNodes;
    }

    const VehicleRestriction& restriction() const
    {
        return _restriction;
    }

    const RoutingProbability& probability() const
    {
        return _probability;
    }

    double cost(int i) const
    {
        return _costs[i];
    }

    double length() const
    {
        return _length;
    }

    double csCost() const // [eMATES]
    {
        return _csCost;
    }

    bool includesStraightDriving() const
    {
        return _includesStraightDriving;
    }

    bool includesLeftTurn() const
    {
        return _includesLeftTurn;
    }

    bool includesRightTurn() const
    {
        return _includesRightTurn;
    }

    unsigned int width() const
    {
        return _width;
    }

    ///@}
};

#endif //__ROUTING_LINK_HPP__
