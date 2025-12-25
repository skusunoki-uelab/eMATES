/* **************************************************************************
 * Copyright 2014-2024 ADVENTURE Project
 * All Rights Reserved
 * ************************************************************************** */
/**
 * @file LaneBundle.hpp
 */
#ifndef __LANE_BUNDLE_HPP__
#define __LANE_BUNDLE_HPP__
#include "Lane.hpp"
#include "CustomMessage.hpp"
#include "SubLaneBundle.hpp"
#include <AmuLineSegment.hpp>
#include <AmuPoint.hpp>
#include <list>
#include <map>
#include <string>
#include <unordered_map>
#include <vector>
#ifdef _OPENMP
#include <omp.h>
#endif //_OPENMP

class Connector;
class LaneBundleBuilder;
class RoadMap;
class ObjectInLane;
class Vehicle;

//##############################################################################
/**
 * @~japanese レーン束オブジェクト
 *
 * 単路部 (Section) と交差点 (Intersection) の基底クラス
 *
 * @~english  Lane bundle object
 *
 * Base class for Section and Intersection classes
 *
 * @~ @ingroup RoadNetwork
 */
class LaneBundle
{
public:
    LaneBundle(const std::string& id, RoadMap* parent);
    virtual ~LaneBundle();

    //==========================================================================
    /**
     * @~japanese @name 幾何形状に関する関数群 
     * @~english  @name Functions related to geometry
     */
    ///@{
public:
    /**
     * @~japanese 中心点を戻す 
     * @~english  Return center point
     */
    virtual const amu::geometry::AmuPoint center() const = 0;

    /**
     * @~japanese 頂点の数を戻す
     * @~english  Return the number of vertexes 
     */
    int numVertexes() const
    {
        return static_cast<signed int>(_vertexes.size());
    }

    /**
     * @~japanese @p i 番目の頂点を戻す
     * @~english  Return the @p i -th vertex
     */
    const amu::geometry::AmuPoint& vertex(int i) const
    {
        ASSERT_MSG(i >= 0);
        ASSERT_MSG(i < _vertexes.size());
        return _vertexes[i];
    }

    /**
     * @~japanese 頂点 @p vertex を追加する
     * @~english  Add @p vertex
     */
    void addVertex(amu::geometry::AmuPoint vertex)
    {
        _vertexes.emplace_back(vertex);
    }

    /**
     * @~japanese @p i 番目の辺を戻す
     *
     * n番目の辺とはn番目と(n+1)番目の頂点を結ぶ辺
     *
     * @~english  Return the @p i -th edge
     *
     * The n-th edge is the edge connecting the n-th and (n+1)-th vertexes.
     */
    const amu::geometry::AmuLineSegment edge(int i) const
    {
        return amu::geometry::AmuLineSegment(
            vertex(i), vertex((i + 1) % numVertexes()));
    }

    ///@}

    //==========================================================================
    /**
     * @~japanese @name 道路構造に関する関数群 
     * @~english  @name functions related to road structure
     */
    ///@{
public:
    /**
     * @~japanese
     * @p lane の下流レーンが所属するレーン束オブジェクトを戻す
     *
     * @~english
     * Return the lane bundle object that downstream lane of @p lane belongs to
     */
    virtual LaneBundle* nextBundle(const Lane* lane) const = 0;

    /**
     * @~japanese
     * @p lane の上流レーンが所属するレーン束オブジェクトを戻す
     *
     * @~english
     * Return the lane bundle object that upstream lane of @p lane belongs to
     */
    virtual LaneBundle* previousBundle(const Lane* lane) const = 0;

    /**
     * @~japanese
     * @p lane の位置 @p distance からこのレーン束オブジェクトの前方の端までの
     * 距離を戻す
     *
     * @~english
     * Return the distance from the position @p distance in @p lane to the front
     * edge of this lane bundle object
     */
    double distanceToNext(const Lane* lane, double distance) const;

    /**
     * @~japanese
     * @p lane の位置 @p distance からこのレーン束オブジェクトの後方の端までの
     * 距離を戻す
     *
     * @~english
     * Return the distance from the position @p distance in @p lane to the back
     * edge of this lane bundle object
     */
    double distanceFromPrevious(const Lane* lane, double distance) const;

    /**
     * @~japanese 識別番号 @p id の内部コネクタを戻す
     * @~english  Return the internal connector with ID number @p id
     */
    const Connector* internalConnector(const std::string& id) const;

    /**
     * @~japanese 識別番号 @p id の内部コネクタ @p connector を追加する
     * @~english  Add internal connector @p connector with ID number @p id 
     */
    void addInternalConnector(const std::string& id, Connector* connector)
    {
        _internalConnectors.insert(make_pair(id, connector));
    }

    /**
     * @~japanese 内部コネクタの数を戻す
     * @~english  Return the number of internal connectors
     */
    int numInternalConnectors() const
    {
        return _internalConnectors.size();
    }

    /**
     * @~japanese 識別番号 @p id のサブセクション @p subsec を追加する
     * @~english  Add subsection @p subsec with ID number @p id
     */
    void addSubLaneBundle(const std::string& id, SubLaneBundle* subsec)
    {
        _subsecs.insert(make_pair(id, subsec));
    }

    /**
     * @~japanese
     * サブセクション @p subsec がこのレーン束オブジェクトに含まれるかどうかを
     * 戻す
     *
     * @~english
     * Return whether subsection @p subsec is contained in this lane bundle
     * object
     */
    bool containsSubLaneBundle(const SubLaneBundle* subsec) const;

    /**
     * @~japanese 車道サブセクションを戻す
     *
     * @todo 汎用性が低く改廃を検討する
     *
     * @~english  Return roadway subsection
     */
    SubLaneBundle* roadwaySubLaneBundle()
    {
        return _subsecs["00"];
    }

    /**
     * @~japanese
     * サブセクション @p subsec の @p edgeNum 番目の辺に接するサブセクションを
     * 戻す
     *
     * @note 隣接するレーン束オブジェクトも検索する
     * 
     * @~english
     * Return subsection that touches the @p edgeNum -th edge of subsection
     * @p subsec
     *
     * @note Search also adjacent lane bundle objects 
     */
    virtual SubLaneBundle* pairedSubLaneBundle(
        SubLaneBundle* subsec, int edgeNum) const
        = 0;

    /**
     * @~japanese レーン @p lane を追加する
     * @~english  Add @p lane
     */
    void addLane(Lane* lane)
    {
        _lanes.insert(make_pair(lane->id(), lane));
    }

    /**
     * @~japanese
     * レーン @p lane がこのレーン束オブジェクトに含まれるかどうかを戻す
     *
     * @~english
     * Return whether @p lane is contained in this lane bundle object
     */
    bool containsLane(const Lane* lane) const
    {
        ASSERT_MSG(lane);
        return (lane->parent() == this);
    }

    /**
     * @~japanese
     * レーン @p lane の下流レーンがこのレーン束オブジェクトに含まれるかどうかを
     * 戻す
     *
     * @~english
     * Return whether the downstream lane of @p lane is contained in this lane
     * bundle object  
     */
    bool containsNextLane(const Lane* lane) const
    {
        return (containsLane(lane) && containsLane(lane->nextLane(0)));
    }

    /**
     * @~japanese
     * レーン @p lane の上流レーンがこのレーン束オブジェクトに含まれるかどうかを
     * 戻す
     *
     * @~english
     * Return whether the upstream lane of @p lane is contained in this lane
     * bundle object  
     */
    bool containsPreviousLane(const Lane* lane) const
    {
        return (containsLane(lane) && containsLane(lane->previousLane(0)));
    }

    /**
     * @~japanese
     * このレーン束オブジェクトに含まれるレーンのうち， @p connector を始点と
     * するレーンの集合を戻す
     *
     * @~english
     * Return the set of lanes starting at @p connector among the lanes
     * contained in this lane bundle object
     */
    std::vector<const Lane*> lanesFromConnector(
        const Connector* connector) const;

    /**
     * @~japanese
     * このレーン束オブジェクトに含まれるレーンのうち， @p connector を終点と
     * するレーンの集合を戻す
     *
     * @~english
     * Return the set of lanes ending at @p connector among the lanes
     * contained in this lane bundle object
     */
    std::vector<const Lane*> lanesToConnector(const Connector* connector) const;

    /**
     * @~japanese レーンの接続をチェックする
     * @param isIntersection 交差点かどうか
     * @return    レーンの接続が無矛盾かどうか
     *
     * @note
     * ODNodeの単路部が接続しない境界以外では，すべてのレーンには少なくとも
     * 1つずつの上流レーンと下流レーンが必要．
     *
     * @~english  Check lane connectivity
     * @param isIntersection Whether it is intersection
     * @return    Whether the lane connectivity are consistent
     *
     * @note
     * All lanes must have at least one upstream lane and one downstream lane,
     * except at borders of ODNodes where sections do not connect. 
     */
    virtual bool checkLaneConnectivity(bool isIntersection) const;

    ///@}

    //==========================================================================
    /**
     * @~japanese
     * @name このレーン束オブジェクトの含まれるレーン内のエージェントの操作に
     * 関する関数群
     *
     * @~english
     * @name Functions related to operations for agents in lanes containing
     * this lane bundle object
     */
    ///@{
public:
    /**
     * @~japanese エージェントの順序列を更新する
     * @~english  Update agent order
     */
    void renewAgentOrder();

    /**
     * @~japanese 認知前に必要なエージェントの処理を実行する
     * @~english  Perform necessary agent processing before recognition
     */
    void makeAgentsPreperceive()
    {
        for (auto itr : _usedLanes)
        {
            itr->makeAgentsPreperceive();
        }
    }

    /**
     * @~japanese エージェントに周囲の状況を認知させる
     * @~english  Let agents recognize circumstances
     */
    void makeAgentsPerceive()
    {
        for (auto itr : _usedLanes)
        {
            itr->makeAgentsPerceive();
        }
    }

    /**
     * @~japanese エージェントに意思決定させる
     * @~english  Let agents make decisions
     */
    void makeAgentsDetermine()
    {
        for (auto itr : _usedLanes)
        {
            itr->makeAgentsDetermine();
        }
    }

    /**
     * @~japanese エージェントに行動を実行させる
     * @~english  Let agents perform actions
     */
    void makeAgentsAct()
    {
        for (auto itr : _usedLanes)
        {
            itr->makeAgentsAct();
        }
    }

    /**
     * @~japanese 行動後に必要なエージェントの処理を実行する
     * @~english  Perform necessary agent processing after action
     */
    void makeAgentsPostact()
    {
        for (auto itr : _usedLanes)
        {
            itr->makeAgentsPostact();
        }
    }

    /**
     * @~japanese
     * エージェント @p agent がこのレーン束オブジェクトの先頭であるかどうかを
     * 戻す
     *
     * @note
     * レーンごとに判断するので，複数のエージェントが先頭になりうる
     *
     * @~english
     * Return whether @p agent is the head of this lane bundle object
     *
     * @note
     * Since being judged for each lane, multiple agents can be the head.  
     */
    bool isHeadAgent(const ObjectInLane* agent, const Lane* lane) const;

    /**
     * @~japanese @p vehicle を _watchedVehicles に追加する
     * @~english  Add @p vehicle to _watchedVehicles
     */
    void addWatchedVehicle(const Vehicle* vehicle);

    /**
     * @~japanese @p vehicle を _watchedVehicles から削除する
     * @~english  Remove @p vehicle from _watchedVehicles
     */
    void eraseWatchedVehicle(const Vehicle* vehicle);

    ///@}

    //==========================================================================
protected:
    /**
     * @~japanese 識別番号
     * @~english  ID number
     */
    std::string _id;

    /**
     * @~japanese このレーン束オブジェクトが所属する地図オブジェクト
     * @~english  Map object to which this lane bundle object belongs
     */
    RoadMap* _parent;

    /**
     * @~japanese 多角形の頂点
     * @~english  Polygon vertexes
     */
    std::vector<amu::geometry::AmuPoint> _vertexes;

    /**
     * @~japanese 内部コネクタの集合
     * 
     * @note ObjectManager でnew & deleteする
     *
     * @note
     * コネクタのローカルIDは，内部コネクタをあらわす"9" + グループ番号 [1桁]
     * + ポイントID [2桁] の4桁とする．レーンの識別番号は始端コネクタと終端
     * コネクタの識別番号の組であらわす．

     * @~english  Set of internal connectors
     *
     * @note New & delete with ObjectManager .
     *
     * @note
     * Local ID number of the connector is 4 digits: "9" representing the
     * internal connector + group number [1 digit] + point ID number [2 digits].
     * Lane ID number is represented by a pair of the ID numbers of the start
     * and the end connectors.
     */
    std::unordered_map<std::string, Connector*> _internalConnectors;

    /**
     * @~japanese このレーン束オブジェクトに含まれるサブセクションの集合
     * @~english  Set of subsections contained in this lane bundle object
     */
    std::unordered_map<std::string, SubLaneBundle*> _subsecs;

    /**
     * @~japanese このレーン束にオブジェクトに含まれるレーンの集合
     * @~english  Set of lanes contained in this lane bundle object  
     */
    std::unordered_map<std::string, Lane*> _lanes;

    /**
     * @~japanese 使用中のレーンの集合
     * @~english  Set of lanes in use
     */
    std::vector<Lane*> _usedLanes;

    /**
     * @~japanese 使用中フラグ
     *
     * @note
     * renewAgentOrder でセットされる．このフラグが true のレーン束
     * オブジェクトのみ RoadMap::_usedLaneBundles に追加され，その
     * ステップの以降の処理の対象となる．
     * 
     * @~english  Usage flag
     *
     * @note
     * Set in renewAgentOrder . Only lane bundle objects with this flag
     * set to true are added to RoadMap::_usedLaneBundles , and are
     * subject to subsequent processing of that step.
     */
    bool _isUsed;

    /**
     * @~japanese 
     * 車線変更など，他車両から注目される行動をとっている車両の集合
     * 
     * @note
     * 車線変更は複数のレーンにまたがる動作であるため，レーンではなくレーン束
     * オブジェクトに登録する．このコンテナは Lane::_agents と異なり，各タイム
     * ステップで自動的に更新されないため，明示的な add と erase が必要．要素の
     * ソートは必要ない．
     *
     * @~english
     * Set of vehicles taking actions that attract attention from other
     * vehicles, such as lane-changes
     *
     * @note
     * Lane-change is an action that spans multiple lanes, so it is registered
     * with a lane bundle object instead of a lane. Since this container is not
     * automatically updated at each time step, unlike Lane::_agents, requires
     * explicit add and erase. Sorting element is not needed.
     */
    std::list<const Vehicle*> _watchedVehicles;

#ifdef _OPENMP
    /**
     * @~japanese ロック変数
     * @~english  Lock variable
     */
    omp_lock_t _lock;
#endif //_OPENMP

    //========================================================================
    /**
     * @~japanese @name アクセッサ
     * @~english  @name Accessor
     */
    ///@{
public:
    const std::string& id() const
    {
        return _id;
    }

    const std::vector<amu::geometry::AmuPoint>& vertexes()
    {
        return _vertexes;
    }

    const std::unordered_map<std::string, Connector*>& internalConnectors()
        const
    {
        return _internalConnectors;
    }

    const std::unordered_map<std::string, SubLaneBundle*>& subLaneBundles()
        const
    {
        return _subsecs;
    }

    const std::unordered_map<std::string, Lane*>& lanes() const
    {
        return _lanes;
    }

    const std::list<const Vehicle*>& watchedVehicles() const
    {
        return _watchedVehicles;
    }

    bool isUsed() const
    {
        return _isUsed;
    }

    ///@}
};

#endif //__LANE_BUNDLE_HPP__
