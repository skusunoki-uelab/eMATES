/* *************************************************************************
 * Copyright 2014-2024 ADVENTURE Project
 * All Rights Reserved
 * ************************************************************************** */
/**
 * @file Intersection.hpp
 */
#ifndef __INTERSECTION_HPP__
#define __INTERSECTION_HPP__
#include "Border.hpp"
#include "Config.hpp"
#include "CustomMessage.hpp"
#include "Lane.hpp"
#include "LaneBundle.hpp"
#include "LinkFlowRecord.hpp"
#include "RelativeDirection.hpp"
#include "RelativeDirectionTable.hpp"
#include "RouteCacheContainer.hpp"
#include "Signal.hpp"
#include "SubLaneBundle.hpp"
#include "VehicleRestriction.hpp"
#include "VehicleType.hpp"
#ifdef INCLUDE_PEDESTRIANS
#include "ped/IntersectionPedExt.hpp"
#endif //INCLUDE_PEDESTRIANS
#ifdef INCLUDE_TRAMS
#include "tram/IntersectionTramExt.hpp"
#endif //INCLUDE_TRAMS
#include <AmuLineSegment.hpp>
#include <AmuPoint.hpp>
#include <algorithm>
#include <vector>
#include <string>

class Connector;
class IntersectionBuilder;
class RoadMap;
class RoutingLink;
class Section;
class Vehicle;

//##############################################################################
/**
 * @~japanese 単路部の端点となる交差点
 * @~english  Intersection in the endpoint of a road segment
 * @~ @ingroup RoadNetwork
 */
class Intersection : public LaneBundle
{
public:
#ifdef INCLUDE_TRAMS
    friend class IntersectionTramExt;
#endif //INCLUDE_TRAMS
#ifdef INCLUDE_PEDESTRIANS
    friend class IntersectionPedExt;
#endif //INCLUDE_PEDESTRIANS

    //==========================================================================
public:
    /**
     * @~japanese レーン @p lanes を識別番号順にソートする
     * @~english  Sort @p lanes by ID number
     */
    static void sortLanes(std::vector<const Lane*>& lanes)
    {
        sort(
            lanes.begin(), lanes.end(),
            [](const Lane* rl, const Lane* rr)
            {
                return (rl->id() < rr->id());
            });
    }

    //==========================================================================
public:
    Intersection(
        const std::string& id, const std::string& type, RoadMap* parent);
    virtual ~Intersection();

    /**
     * @~japanese メンバのリソースを動的に確保する
     * @note コンストラクタから呼ばれる
     * 
     * @~english  Dynamically allocate resources for members
     * @note Called from the constructor
     */
    void initializeMembers();

    //==========================================================================
    /**
     * @~japanese @name 幾何形状に関する関数群 
     * @~english  @name Functions related to geometry
     */
    ///@{
public:
    /**
     * @~japanese
     * 方向 @p inDir から方向 @p outDir までの代表長さを戻す
     *
     * @~english
     * Return the representative length from direction @p inDir to direction
     * @p outDir
     */
    double length(int inDir, int outDir) const;

    /**
     * @~japanese
     * この交差点と交差点 @p inter を接続する単路部に接する辺を戻す
     *
     * @~english
     * Return the edge that touches the section connecting this intersection
     * with the intersection @p inter
     */
    const amu::geometry::AmuLineSegment edgeToNextInter(
        const Intersection* inter) const
    {
        return edge(_dir2edge[direction(inter)]);
    }

    /**
     * @~japanese 境界番号 @p i を変換した辺番号を戻す
     * @~english  Return the edge number converted from the border number @p i
     */
    int dir2edge(int i) const
    {
        ASSERT_MSG(i >= 0);
        ASSERT_MSG(i < _dir2edge.size());
        return _dir2edge[i];
    }

    /**
     * @~japanese 辺番号 @p i を変換した境界番号を戻す
     * @~english  Return the border number converted from the edge number @p i  
     */
    int edge2dir(int i) const
    {
        ASSERT_MSG(i >= 0);
        ASSERT_MSG(i < _vertexes.size());
        for (int j = 0; j < static_cast<signed int>(_dir2edge.size()); j++)
        {
            if (_dir2edge[j] == i)
                return j;
        }
        return -1;
    }

    /**
     * @~japanese 横断歩道幅 @p width を追加する
     * @attention _next と同じ順に追加しなければならない
     *
     * @~english  Add crosswalk width @p width
     * @attention Must be added in the same order as _next .
     */
    void addCrosswalkWidth(double width)
    {
        _crosswalkWidth.push_back(width);
    }

    /**
     * @~japanese 境界番号 @p dir の横断歩道の幅を戻す
     * @attention 実際に横断歩道が設置されていることを保証しない
     *
     * @~english  Return crosswalk width at border number @p dir
     * @attention Not guarantee that a crosswalk is actually in place. 
     */
    double crosswalkWidth(int dir) const
    {
        ASSERT_MSG(dir >= 0);
        ASSERT_MSG(dir < _crosswalkWidth.size());
        return _crosswalkWidth[dir];
    }

    ///@}

    //==========================================================================
    /**
     * @~japanese @name 道路構造に関する関数群 
     * @~english  @name Functions related to road structure
     */
    ///@{
    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    // レーン束オブジェクトを戻す関数群
    // Functions that return lane bundle object
public:
    /**
     * @~japanese
     * @p lane の下流レーンが所属するレーン束オブジェクトを戻す
     *
     * @~english
     * Return the lane bundle object that the downstream lane of @p lane belongs
     * to
     */
    virtual LaneBundle* nextBundle(const Lane* lane) const override;

    /**
     * @~japanese
     * @p lane の上流レーンが所属するレーン束オブジェクトを戻す
     *
     * @~english
     * Return the lane bundle object that the upstream lane of @p lane
     * belongs to
     */
    virtual LaneBundle* previousBundle(const Lane* lane) const override;

    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    // 隣接交差点に関する関数群
    // Functions related to adjacent intersections
public:
    /**
     * @~japanese 隣接交差点の個数を戻す
     * @~english  Return the number of adjacent intersections
     */
    int numNexts() const
    {
        return static_cast<signed int>(_nexts.size());
    }

    /**
     * @~japanese 境界方向 @p dir に隣接する交差点を戻す
     * @~english  Return the intersection adjacent to border direction @p dir
     */
    Intersection* next(int dir) const
    {
        return _nexts[dir];
    }

    /**
     * @~japanese 隣接交差点 @p inter を設定する
     * @attention 登録順に注意．
     *
     * @~english  Set an adjacent intersection @p inter
     * @attention Note the order of registration
     */
    void setNext(Intersection* inter);

    /**
     * @~japanese
     * 交差点 @p from からこの交差点に流入後に到達可能な直進方向の交差点を戻す 
     *
     * @~english
     * Return the intersection in the straight direction that can be reached
     * after flowing into this intersection from intersection @p from
     */
    Intersection* nextStraight(const Intersection* from) const;

    /**
     * @~japanese
     * 交差点 @p from からこの交差点に流入後に到達可能な左折方向の交差点を戻す 
     *
     * @~english
     * Return the intersection in the left-turn direction that can be reached
     * after flowing into this intersection from intersection @p from
     */
    Intersection* nextLeft(const Intersection* from) const;

    /**
     * @~japanese
     * 交差点 @p from からこの交差点に流入後に到達可能な右折方向の交差点を戻す 
     *
     * @~english
     * Return the intersection in the right-turn direction that can be reached
     * after flowing into this intersection from intersection @p from
     */
    Intersection* nextRight(const Intersection* from) const;

    /**
     * @~japanese
     * 交差点 @p from からこの交差点に流入後に到達可能な別の交差点を戻す 
     *
     * @attention
     * 特定の方向の交差点を戻す保証はないので注意すること
     *
     * @~english
     * Return another intersection that can be reached after flowing into this
     * intersection from intersection @p from
     *
     * @attention
     * Note that there is no guarantee that the intersection in a particular
     * direction will be returned.
     */
    Intersection* nextAnother(const Intersection* from) const;

    /**
     * @~japanese
     * 交差点 @p from からこの交差点を経由して交差点 @p to に到達するパスが
     * あるかどうかを戻す
     *
     * @~english
     * Return whether there is a path that reaches intersection @p to from
     * intersection @p from via this intersection
     */
    bool hasValidPath(const Intersection* from, const Intersection* to) const;

    /**
     * @~japanese @p inter と同一ネットワーク上にあるかどうかを戻す
     * @note 現在は使用されていない
     *
     * @~english  Return whether @p inter is on the same network
     * @note Not currently used
     */
    bool isNetworked(const Intersection* inter);

private:
    /**
     * @~japanese @p inter と同一ネットワーク上にあるかどうかを戻す
     *
     * isNetworked(const Intersection*) から呼ばれる．再起処理の無限ループを
     * 防ぐため，調査済みの交差点を引数として渡す
     *
     * @note 現在は使用されていない
     *
     * @~english  Return whether @p inter is on the same network
     *
     * Called from isNetworked(const Intersection*) . To avoid endless loops in
     * the recursive process, pass the already investigated intersections as an
     * argument.
     *
     * @note Not currently used
     */
    bool _isNetworked(
        std::vector<const Intersection*>& alreadySearched,
        const Intersection*               inter);

    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    // 接続単路部に関する関数群
    // Functions related to connecting sections
public:
    /**
     * @~japanese 境界方向 @p dir に接続する単路部 @p section を設定する
     * @~english  Set @p section connected to the border direction @p dir
     */
    void setNextSection(int dir, Section* section)
    {
        ASSERT_MSG(dir >= 0);
        ASSERT_MSG(dir < _incSections.size());
        ASSERT_MSG(!_incSections[dir]);
        _incSections[dir] = section;
    }

    /**
     * @~japanese 交差点 @p inter に向かう単路部を戻す
     * @~english  Return the section towards the intersection @p inter
     */
    Section* nextSection(const Intersection* inter) const
    {
        int dir = direction(inter);
        if (dir == -1)
        {
            return nullptr;
        }
        return nextSection(dir);
    }

    /**
     * @~japanese 境界方向 @p dir に接続する単路部を戻す
     * @~english  Return the section connected to the border direction @p dir
     */
    Section* nextSection(int dir) const
    {
        return _incSections[dir];
    }

    /**
     * @~japanese 交差点内のレーン @p lane から接続する単路部を戻す
     * @~english  Return the section connecting from @p lane in the intersection
     */
    Section* nextSection(const Lane* lane) const
    {
        int dir = direction(lane->endConnector());
        if (dir == -1)
        {
            return nullptr;
        }
        return _incSections[dir];
    }

    /**
     * @~japanese 交差点内のレーン @p lane へ接続する単路部を戻す
     * @~english  Return the section connecting to @p lane in the intersection
     */
    Section* previousSection(const Lane* lane) const
    {
        int dir = direction(lane->beginConnector());
        if (dir == -1)
        {
            return nullptr;
        }
        return _incSections[dir];
    }

    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    // 境界に関する関数群
    // Functions related to borders
public:
    /**
     * @~japanese @p i 番目の境界を戻す
     * @~english  Return @p i -th border
     */
    const Border* border(int i) const
    {
        ASSERT_MSG(i >= 0);
        ASSERT_MSG(i < _borders.size());
        return _borders[i];
    }

    /**
     * @~japanese 辺番号 @p edgeNum の辺に境界 @p border を設定する
     * @~english  Set @p border to the edge with edge number @p edgeNum
     */
    void addBorder(Border* border, int edgeNum);

    /**
     * @~japanese 交差点 @p inter 方向の境界番号を戻す
     * @~english  Return the border number towards @p inter
     */
    int direction(const Intersection* inter) const
    {
        for (unsigned int i = 0; i < _nexts.size(); i++)
        {
            if (_nexts[i] == inter)
            {
                return i;
            }
        }
        return -1;
    }

    /**
     * @~japanese 単路部 @p section 方向の境界番号を戻す
     * @~english  Return the border number towards @p section
     */
    int direction(const Section* section) const
    {
        for (unsigned int i = 0; i < _incSections.size(); i++)
        {
            if (_incSections[i] == section)
            {
                return i;
            }
        }
        return -1;
    }

    /**
     * @~japanese サブセクション @p subsec が接する境界番号を戻す
     *
     * サブセクションの識別番号から10を減じた数が辺番号をあらわす
     *
     * @~english  Return the border number that the subsection @p subsec touches
     *
     * Subsection ID number minus 10 represents edge number.
     */
    int direction(const SubLaneBundle* subsec) const
    {
        if (containsSubLaneBundle(subsec))
        {
            int idInt = stoi(subsec->id());
            if (idInt >= 10)
            {
                return edge2dir(idInt - 10);
            }
        }
        return -1;
    }

    /**
     * @~japanese コネクタ @p connector を持つ境界の境界番号を戻す
     * @~english  Return the border number of the border with @p connector
     */
    int direction(const Connector* connector) const;

    /**
     * @~japanese
     * レーン @p lane の流入方向となりうる境界番号を戻す
     *
     * @~english
     * Return the border numbers that can be the inflow direction of @p lane
     */
    std::vector<int> inflowDirections(const Lane* lane) const;

    /**
     * @~japanese
     * レーン @p lane の流出方向となりうる境界番号を戻す
     *
     * @~english
     * Return the border numbers that can be the outflow direction of @p lane
     */
    std::vector<int> outflowDirections(const Lane* lane) const;

    /**
     * @~japanese 境界方向 @p dir の反対の境界番号を戻す
     * @~english  Return the opposite border number of the border number @p dir
     */
    int oppositeDirection(int dir) const;

    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    // 局所経路に関する関数群
    // Functions related to local route
public:
    /**
     * @~japanese
     * 交差点 @p prevInter から交差点 @p nextInter へのパスが直進方向であるか
     * どうかを戻す
     *
     * @~english
     * Return whether the path from intersection @p prevInter to intersection
     * @p nextInter is in the straight direction.   
     */
    bool hasStraightPath(
        const Intersection* prevInter, const Intersection* nextInter) const
    {
        ASSERT_MSG(prevInter);
        ASSERT_MSG(nextInter);
        if ((*_rdTable)(direction(prevInter), direction(nextInter))
            == RD::STRAIGHT)
        {
            return true;
        }
        return false;
    }

    /**
     * @~japanese レーン @p lane が直進レーンであるかどうかを戻す
     * @~english  Return whether @p lane is a straight lane
     */
    bool hasStraightLane(const Lane* lane) const
    {
        ASSERT_MSG(containsLane(lane));
        if ((*_rdTable)(lane->beginDirection(), lane->endDirection())
            == RD::STRAIGHT)
        {
            return true;
        }
        return false;
    }

    /**
     * @~japanese
     * 交差点 @p prevInter から交差点 @p nextInter へのパスが左折方向であるか
     * どうかを戻す
     *
     * @~english
     * Return whether the path from intersection @p prevInter to intersection
     * @p nextInter is in the left-turn direction.   
     */
    bool hasLeftTurnPath(
        const Intersection* prevInter, const Intersection* nextInter) const
    {
        ASSERT_MSG(prevInter);
        ASSERT_MSG(nextInter);
        if ((*_rdTable)(direction(prevInter), direction(nextInter)) == RD::LEFT)
        {
            return true;
        }
        return false;
    }

    /**
     * @~japanese レーン @p lane が左折レーンであるかどうかを戻す
     * @~english  Return whether @p lane is a left-turn lane
     */
    bool hasLeftTurnLane(const Lane* lane) const
    {
        ASSERT_MSG(containsLane(lane));
        if ((*_rdTable)(lane->beginDirection(), lane->endDirection())
            == RD::LEFT)
        {
            return true;
        }
        return false;
    }

    /**
     * @~japanese
     * 交差点 @p prevInter から交差点 @p nextInter へのパスが右折方向であるか
     * どうかを戻す
     *
     * @~english
     * Return whether the path from intersection @p prevInter to intersection
     * @p nextInter is in the right-turn direction.   
     */
    bool hasRightTurnPath(
        const Intersection* prevInter, const Intersection* nextInter) const
    {
        ASSERT_MSG(prevInter);
        ASSERT_MSG(nextInter);
        if ((*_rdTable)(direction(prevInter), direction(nextInter))
            == RD::RIGHT)
        {
            return true;
        }
        return false;
    }

    /**
     * @~japanese レーン @p lane が右折レーンであるかどうかを戻す
     * @~english  Return whether @p lane is a right-turn lane
     */
    bool hasRightTunLane(const Lane* lane) const
    {
        ASSERT_MSG(containsLane(lane));
        if ((*_rdTable)(lane->beginDirection(), lane->endDirection())
            == RD::RIGHT)
        {
            return true;
        }
        return false;
    }

    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    // 相対方向に関する関数群
    // Functions related to relative direction
public:
    /**
     * @~japanese
     * 交差点 @p prevInter から交差点 @p nextInter へのパスの相対方向を戻す
     *
     * @~english
     * Return the relative direction of the path from intersection @p prevInter
     * to intersection @p nextInter
     */
    const RelativeDirection& relativeDirection(
        const Intersection* prevInter, const Intersection* nextInter) const
    {
        ASSERT_MSG(prevInter);
        ASSERT_MSG(nextInter);
        return (*_rdTable)(direction(prevInter), direction(nextInter));
    }

    /**
     * @~japanese
     * 境界方向 @p inflowDir から 境界方向 @p outflowDir へのパスの相対方向を
     * 戻す
     * 
     * @~english
     * Return the relative direction of the path from border direction
     * @p inflowDir to border direction @p outflowDir 
     */
    const RelativeDirection& relativeDirection(
        int inflowDir, int outflowDir) const
    {
        ASSERT_MSG(inflowDir >= 0);
        ASSERT_MSG(inflowDir < _borders.size());
        ASSERT_MSG(outflowDir >= 0);
        ASSERT_MSG(outflowDir < _borders.size());
        return (*_rdTable)(inflowDir, outflowDir);
    }

    /**
     * @~japanese レーン @p lane の相対方向を戻す
     * @~english  Return the relative direction of @p lane
     */
    const RelativeDirection& relativeDirection(const Lane* lane) const
    {
        ASSERT_MSG(containsLane(lane));
        return (*_rdTable)(lane->beginDirection(), lane->endDirection());
    }

    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    // レーンに関する関数群
    // Functions related to lanes
public:
    /**
     * @~japanese 境界方向 @p dir の流入レーン数を戻す
     * @~english  Return the number of inflow lanes in border direction @p dir
     */
    int numIn(int dir) const
    {
        ASSERT_MSG(dir >= 0);
        ASSERT_MSG(dir < _nexts.size());
        return _numIn[dir];
    }

    /**
     * @~japanese 境界方向 @p dir の流出レーン数を戻す
     * @~english  Return the number of outflow lanes in border direction @p dir  
     */
    int numOut(int dir) const
    {
        ASSERT_MSG(dir >= 0);
        ASSERT_MSG(dir < _nexts.size());
        return _numOut[dir];
    }

    /**
     * @~japanese 単路部 @p section から流入するレーンの集合を返す
     * @~english  Return the set of lanes flowing from @p section
     */
    std::vector<const Lane*> lanesFrom(const Section* section) const
    {
        return lanesFrom(direction(section));
    }

    /**
     * @~japanese 境界方向 @p dir から流入するレーンの集合を戻す
     * @~english  Return the set of lanes flowing from border direction @p dir
     */
    std::vector<const Lane*> lanesFrom(int dir) const;

    /**
     * @~japanese 単路部 @p section へ流出するレーンの集合を返す
     * @~english  Return the set of lanes flowing out to @p section
     */
    std::vector<const Lane*> lanesTo(const Section* section) const
    {
        return lanesTo(direction(section));
    }

    /**
     * @~japanese 境界方向 @p dir へ流出するレーンの集合を返す
     * @~english  Return the set of lanes flowing out to border direction @p dir
     */
    std::vector<const Lane*> lanesTo(int direction) const;

    /**
     * @~japanese
     * レーン @p lane から境界方向 @p dir へのパスが存在するかどうかを戻す
     *
     * @~english
     * Return whether there is a path that reaches border direction @p dir from
     * @p lane
     */
    bool hasValidPath(const Lane* lane, int direction) const;

    /**
     * @~japanese レーン @p lane が交差点内中心レーンかどうかを戻す
     * @~english  Return whether @p lane is the main lane in this intersection
     */
    bool hasMainLane(const Lane* lane) const;

    /**
     * @~japanese 中心サブセクションを通るレーンの集合を戻す
     * @~english  Return set of lanes through center subsection
     */
    std::vector<const Lane*> mainLanes() const;

    /**
     * @~japanese レーン @p lane と交錯するレーンの集合を返す
     * @~english  Return the set of lanes that cross with @p lane
     */
    std::vector<const Lane*> collisionLanes(const Lane* lane) const;

    /**
     * @~japanese
     * レーン @p lane に対する交差点内交錯レーンと単路部内交錯レーンの集合を
     * 取得する．
     * 
     * @param[out] result_inter 交差点内交錯レーン
     * @param[out] result_section 単路部内交錯レーン
     *
     * @~english
     * Get the sets of crossing lanes in intersection and crossing lanes in
     * section for @p lane
     *
     * @param[out] result_inter The set of crossing lanes in intersection
     * @param[out] result_section The set of crossing lanes in section 
     */
    void getCollisionLanes(
        const std::vector<const Lane*>& lanes,
        std::vector<const Lane*>&       result_inter,
        std::vector<const Lane*>&       result_section) const;

    /**
     * @~japanese
     * レーン @p lane の交錯レーンのうち，交点が位置 @p distance より下流の
     * ものを戻す
     * 
     * @~english
     * Return crossing lanes of @p lane whose intersection point is downstream
     * from position @p distance
     */
    std::vector<const Lane*> collisionLanesFront(
        const Lane* lane, double distance) const;

    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    // コネクタに関する関数群
    // Functions related to connectors
public:
    /**
     * @~japanese
     * 境界方向 @p dir から見た左コネクタ数を戻す
     *
     * 左側通行なら numIn(dir) ，右側通行なら numOut(dir) を戻す
     *
     * @~english
     * Return the number of left connectors as seen from the border
     * direction @p dir
     *
     * Return numIn(dir) for left-hand traffic, numOut(dir) for right-hand
     * traffic
     */
    int numLeftConnectors(int dir) const
    {
#ifdef RIGHT_HAND_TRAFFIC
        return numOut(dir);
#else  //RIGHT_HAND_TRAFFIC not defined
        return numIn(dir);
#endif //RIGHT_HAND_TRAFFIC
    }

    /**
     * @~japanese
     * 境界方向 @p dir から見た右コネクタ数を戻す
     *
     * 左側通行なら numOut(dir) ，右側通行なら numIn(dir) を戻す
     *
     * @~english
     * Return the number of right connectors as seen from the border
     * direction @p dir
     *
     * Return numOut(dir) for left-hand traffic, numIn(dir) for right-hand
     * traffic
     */
    int numRightConnectors(int dir) const
    {
#ifdef RIGHT_HAND_TRAFFIC
        return numIn(dir);
#else  //RIGHT_HAND_TRAFFIC
        return numOut(dir);
#endif //RIGHT_HAND_TRAFFIC
    }

    /**
     * @~japanese
     * 識別番号 @p idInt にひもづく中心サブセクションの辺上のコネクタを戻す
     * 
     * @~english
     * Return the connector on the edge of the central subsection associated
     * with the ID number @p idInt
     */
    const Connector* edgeConnector(int idInt);

    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    // サブセクションに関する関数群
    // Functions related to subsections
public:
    /**
     * @~japanese
     * サブセクション @p subsec の @p edgeNum 番目の辺に接するサブセクションを
     * 戻す
     *
     * @note 隣接する単路部も検索する
     * 
     * @~english
     * Return subsection that touches the @p edgeNum -th edge of subsection
     * @p subsec
     *
     * @note Search also adjacent sections 
     */
    SubLaneBundle* pairedSubLaneBundle(
        SubLaneBundle* subsec, int edgeNum) const override;

    ///@}

    //==========================================================================
    /**
     * @~japanese @name 信号に関する関数
     * @~english  @name Function related to traffic light
     */
    ///@{
public:
    /**
     * @~japanese
     * 境界方向 @p inflowDir から流入する車両の進入許可を戻す
     *
     * @~english
     * Return restrictio for a vehicle flowing in from border direction
     * @p inflowDir
     */
    Signal::Permission permission(int inflowDir) const;

    /**
     * @~japanese
     * 境界方向 @p inflowDir から流入して相対方向 @p turning に向かう車両
     * @p vehicle の進入許可を戻す
     *
     * @~english
     * Return permission for @p vehicle flowing in from border direction
     * @p inflowDir and out to the relative direction @p turning
     */
    Signal::Permission permission(
        int inflowDir, const RelativeDirection& turning,
        Vehicle* vehicle) const;

    ///@}

    //==========================================================================
    /**
     * @~japanese @name エージェントや経路探索に関する関数群
     * @~english  @name Functions related to agents and routing
     */
    ///@{
public:
    /**
     * @~japanese 経路探索結果コンテナを削除する
     * @~english  Delete route search result container
     */
    void deleteRouteCacheContainer()
    {
        if (_routeCacheContainer)
        {
            delete _routeCacheContainer;
        }
    }

    /**
     * @~japanese
     * この交差点に対応する経路探索用リンク @p linkを登録する
     * 
     * @~english
     * Register a @p link for routing corresponding to this intersection
     */
    void addRoutingLink(RoutingLink* link);

    /**
     * @~japanese リンク交通流を観測し巨視的諸量を更新する
     * 
     * 観測結果は経路探索などに用いる
     * 
     * @~english  Observe link traffic flow and update macroscopic quantities
     * 
     * Observation results are used for route searching, etc.
     */
    void observeLinkFlow();

    /**
     * @~japanese リンク交通流の観測結果を初期化する
     * @~english  Initialize link traffic flow observation results
     */
    void initializeLinkFlowObservation();

    /**
     * @~japanese 車種 @p type をAllowリストに追加する
     * @param from 流入方向に接続する交差点
     * @param to   流出方向に接続する交差点
     *
     * @~english  Add vehicle type @p type to allow-list
     * @param from Intersection connecting to inflow direction
     * @param to   Intersection connecting to outflow direction
     */
    void addAllowedVehicleType(
        const Intersection* from, const Intersection* to,
        const VehicleType& type);

    /**
     * @~japanese 車種 @p typeStr をDenyリストに追加する
     * @param from 流入方向に接続する交差点
     * @param to   流出方向に接続する交差点
     *
     * @~english  Add vehicle type @p typeStr to deny-list
     * @param from Intersection connecting to inflow direction
     * @param to   Intersection connecting to outflow direction
     */
    void addDeniedVehicleType(
        const Intersection* from, const Intersection* to,
        const VehicleType& type);

    /**
     * @~japanese
     * 車種 @p type の車両が @p from から @p to へ通行可能かどうかを戻す
     *
     * @~english
     * Return whether vehicle with type @p type can pass from @p from to @p to
     */
    bool permitsPassing(
        const Intersection* from, const Intersection* to,
        const VehicleType& type) const;

    ///@}

    //==========================================================================
    /**
     * @~japanese 座標，種別と隣接交差点を @p out に出力する
     * @~english  Output coordinates, type and adjacent intersections to @p out
     */
    void printMapInfo(std::ostream& out) const;

    /**
     * @~japanese 属性を @p out に出力する
     *
     * @param isODNode ODノードであることをあらわすフラグ
     *
     * @~english  Output attributes to @p out
     *
     * @param isODNode Flag indicating that it is an ODNode
     */
    void print(std::ostream& out, bool isODNode) const;

    //==========================================================================
protected:
    /**
     * @~japanese 中心点
     * @~english  Center point
     */
    amu::geometry::AmuPoint _center;

    /**
     * @~japanese 境界
     * @~english  Borders
     */
    std::vector<Border*> _borders;

    /**
     * @~japanese 境界番号から辺番号へのマッピングテーブル
     *
     * インデックスが境界番号，値が辺番号をあらわす
     *
     * @~english  Mapping table from border number to edge number
     *
     * The indexes represent border numbers and values represent edge numbers
     */
    std::vector<int> _dir2edge;

    /**
     * @~japanese 境界方向間の相対方向テーブル
     * @~english  Relative direction table between border directions
     */
    RelativeDirectionTable* _rdTable;

    /**
     * @~japanese 隣接交差点
     *
     * @note
     * インデックスは境界方向をあらわす
     *
     * @attention
     * 重複を許す構造になっており，隣接交差点の重複は単路部が複数あることを
     * あらわす．ただし本当に重複させる場合には単路部の識別番号の付け方に
     * 修正が必要．
     *
     * @~english  Adjacent intersections
     *
     * @note
     * The indexes represent border directions.
     *
     * @attention
     * The structure allows duplication, and duplication of adjacent
     * intersections indicates that there are multiple sections. However, when
     * actually overlapping, it is required to correct how to assign the
     * section ID number.
     */
    std::vector<Intersection*> _nexts;

    /**
     * @~japanese 接続する単路部
     * @note インデックスは境界方向をあらわす
     * 
     * @~english  Incident sections
     * @note The indexes represent border directions. 
     */
    std::vector<Section*> _incSections;

    /**
     * @~japanese 各境界からの流入レーン数
     * @~english  Number of inflow lanes from each border
     */
    std::vector<int> _numIn;

    /**
     * @~japanese 各境界への流出レーン数
     * @~english  Number of outflow lanes to each border
     */
    std::vector<int> _numOut;

    /**
     * @~japanese この交差点に設置されている信号
     * @~english  Traffic light installed at this intersection
     */
    Signal* _signal;

    /**
     * @~japanese 経路探索結果コンテナ
     *
     * @note
     * 並列性および結果の再現性を考慮し各交差点に持たせる．"from,  next->goal"
     * の "from" に相当する交差点が関連する結果を保持する．ODノードはかならず
     * "from" として指定される．
     *
     * @~english  Route search result container
     *
     * @note
     * Considering parallelism and reproducibility of the results, placed at
     * each intersection. The intersection corresponding to "from" in "from, 
     * next->goal" holds the associated result. ODNodes is always specified as
     * "from".
     */
    RouteCacheContainer* _routeCacheContainer;

    /**
     * @~japanese 対応するランク0の経路探索用リンク
     * 
     * 1交差点に対して複数のリンクが対応するためvectorに登録する．
     * 
     * @~english  Corresponding routing link with rank 0
     * 
     * Since multiple links correspond to one intersection, register them in
     * vector.
     */
    std::vector<RoutingLink*> _routingLinks;

    /**
     * @~japanese リンク交通流の記録
     * 
     * リンク旅行時間の算出に用いる
     *  
     * @~english  Link traffic flow record
     * 
     * Used to calculate link travel time
     */
    std::vector<LinkFlowRecord*> _linkFlowRecords;

    /**
     * @~japanese
     * 前の計測期間における流入方向ごとの交差点通過台数
     *
     * @note
     * 指定された間隔で _numPassedVehicles で上書きされる
     *
     * @~english
     * The number of vehicles passing the intersection for each inflow
     * direction in the previous measurement period
     *
     * @note
     * Overridden by _numPassedVehicles at specified intervals.
     */
    std::vector<ulint> _previousNumPassedVehicles;

    /**
     * @~japanese 歩道幅 [m]
     * @~english  Sidewalk width [m]
     */
    double _sidewalkWidth;

    /**
     * @~japanese 横断歩道幅 [m]
     *
     * @note インデックスは境界方向をあらわす
     *
     * @~english  Crosswalk width [m] 
     * 
     * @note The indexes represent border directions. 
     */
    std::vector<double> _crosswalkWidth;

    /**
     * @~japanese 通行権
     * 
     * @note
     * 2次元vectorで管理する．第1インデックスが流入境界番号，第2インデックスが
     * 流出境界番号をあらわす．
     *
     * @~english  Right-of-way
     * 
     * @note
     * Managed in a 2D vector. The first index represents the inflow border
     * number, and the second index represents the outflow border number.
     */
    std::vector<std::vector<VehicleRestriction>> _restrictions;

    //==========================================================================
    /**
     * @~japanese @name アクセッサ
     * @~english  @name Accessor
     */
    ///@{
public:
    const amu::geometry::AmuPoint center() const override
    {
        return _center;
    }

    void setCenter(const amu::geometry::AmuPoint& center)
    {
        _center = center;
    }

    const std::vector<Border*>& borders() const
    {
        return _borders;
    }

    void setRDTable(RelativeDirectionTable* rdTable)
    {
        _rdTable = rdTable;
    }

    Signal* signal() const
    {
        return _signal;
    }

    void setSignal(Signal* signal)
    {
        _signal = signal;
    }

    RouteCacheContainer* routeCacheContainer()
    {
        return _routeCacheContainer;
    }

    LinkFlowRecord* linkFlowRecord(unsigned int i) const
    {
        ASSERT_MSG(i < _linkFlowRecords.size());
        return _linkFlowRecords[i];
    }

    double sidewalkWidth() const
    {
        return _sidewalkWidth;
    }

    void setSidewalkWidth(double width)
    {
        _sidewalkWidth = width;
    }

    const VehicleRestriction& restriction(int from, int to) const
    {
        return _restrictions[from][to];
    }

    ///@}

#ifdef INCLUDE_PEDESTRIANS
    //==========================================================================
    /**
     * @~japanese @name 歩行者用拡張
     * @~english  @name Extension for pedestrian
     */
    ///@{
protected:
    IntersectionPedExt* _pedExt;

public:
    IntersectionPedExt* pedExt() const
    {
        return _pedExt;
    }

    ///@}
#endif //INCLUDE_PEDESTRIANS

#ifdef INCLUDE_TRAMS
    //==========================================================================
    /**
     * @~japanese @name 路面電車用拡張
     * @~english  @name Extension for tram
     */
    ///@{
protected:
    IntersectionTramExt* _tramExt;

public:
    IntersectionTramExt* tramExt()
    {
        return _tramExt;
    }

    ///@}
#endif //INCLUDE_TRAMS
};

#endif //__INTERSECTION_HPP__
