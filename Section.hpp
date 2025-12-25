/* **************************************************************************
 * Copyright 2014-2024 ADVENTURE Project
 * All Rights Reserved
 * ************************************************************************** */
/**
 * @file Section.hpp
 */
#ifndef __SECTION_HPP__
#define __SECTION_HPP__
#include "LaneBundle.hpp"
#include "LinkFlowRecord.hpp"
#include "SubLaneBundle.hpp"
#include "VehicleRestriction.hpp"
#include "VehicleType.hpp"
#include "VehicleTypeManager.hpp"
#include <AmuPoint.hpp>
#include <AmuVector.hpp>
#include <cassert>
#include <iostream>
#include <list>
#include <map>
#include <string>
#include <vector>

class Intersection;
class LinkFlowMonitor;
class RoadMap;
class RoutingNode;
class SectionBuilder;
class SpeedLimitItem;

#ifdef INCLUDE_TRAMS
class SectionTramExt;
#endif //INCLUDE_TRAMS

//##############################################################################
/**
 * @~japanese 2つの交差点を接続する単路部
 *
 * @note
 * 接続する交差点の識別番号の小さいものから大きいものに向かう方向を単路部の
 * 「上り方向」と呼ぶ．上り方向の始点である交差点を _incInters[0]，終点である
 * 交差点を _incInters[1] が保持する．また，_incInters[0] と接する境界の方向を
 * 境界方向0とする．
 *
 * @~english  Road segment connecting 2 intersections
 *
 * @note
 * The direction from the connected intersection with smaller ID number to one
 * with larger ID number is called the "ascending direction" of the section.
 * _incInters[0] holds the start intersection and _incInters[1] holds the end
 * intersection in the ascending direction. Also, the direction of the border
 * that touches _incInters[0] is defined as border direction 0.
 *
 * @~ @ingroup RoadNetwork
 */
class Section : public LaneBundle
{
public:
#ifdef INCLUDE_TRAMS
    friend class SectionTramExt;
#endif //INCLUDE_TRAMS
    //==========================================================================
    Section(
        const std::string& id, Intersection* first, Intersection* second,
        RoadMap* parent);
    ~Section();

    //==========================================================================
    /**
     * @~japanese @name 幾何形状に関する関数群
     * @~english  @name Functions related to geometry
     */
    ///@{
public:
    /**
     * @~japanese 代表長さを計算する
     * @~english  Calculate representative length
     */
    void calcLength()
    {
        assert(_vertexes.size() == 4);
        _length = (amu::geometry::AmuLineSegment(_vertexes[1], _vertexes[2])
                       .length()
                   + amu::geometry::AmuLineSegment(_vertexes[3], _vertexes[0])
                         .length())
            / 2.0;
    }

    /**
     * @~japanese 代表道路幅を戻す
     *
     * ひとまず単にレーン数を戻す
     *
     * @~english  Return the representative road width
     *
     * Just return the number of lanes for now 
     */
    double width() const
    {
        return (_numIn[0] + _numIn[1] + _numOut[0] + _numOut[1]) / 2.0;
    }

    /**
     * @~japanese 上り方向の道路幅を戻す
     * @~english  Return the road width in the ascending direction
     */
    double upWidth() const
    {
        return _numIn[0];
    }

    /**
     * @~japanese 下り方向の道路幅を戻す
     * @~english  Return the road width in the descending direction
     */
    double downWidth() const
    {
        return _numIn[1];
    }

    /**
     * @~japanese
     * 交差点@p interから見た左右の歩道の幅 [m]
     *
     * @param lestSide true:左側，false:右側
     * 
     * 歩道が設置されていない場合は0を戻す
     * 
     * @~english
     * Sidewalk width [m] on the left or right of the section seen from the
     * intersection @p inter
     * 
     * @param lestSide true: left side, false: right side
     * 
     * Returns 0.0 if no sidewalk is installed.
     */
    double sidewalkWidth(Intersection* inter, bool leftSide) const;

    ///@}

    //==========================================================================
    /**
     * @~japanese @name 道路構造に関する関数群
     * @~english  @name Functions related to road structure
     */
    ///@{
    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:
    /**
     * @~japanese
     * @p lane の下流レーンが所属するレーン束オブジェクトを戻す
     *
     * @~english
     * Return the lane bundle object that the downstream lane of @p lane
     * belongs to
     */
    virtual LaneBundle* nextBundle(const Lane* lane) const override;

    /**
     * @~japanese
     * @p lane の上流レーンが所属するレーン束オブジェクトを戻す
     *
     * @~english
     * Return the lane bundle object that the upstream lane of @p lane belongs
     * to
     */
    virtual LaneBundle* previousBundle(const Lane* lane) const override;

    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:
    /**
     * @~japanese
     * 指定した方向の下流に接続する交差点を戻す
     *
     * _incInters[0]->_incInters[1] の方向を単路部の上り方向に合わせる．
     * @p isUp が true(1) の場合は _incInters[1] ， @p isUp が false(0) の場合は
     * _incInters[0] を戻す． 
     *
     * @~english
     * Return the intersection connecting downstream in the specified
     * direction
     *
     * Match the direction _incInters[0]->_incInters[1] to the up direction of
     * the section. Return _incInters[1] if @p isUp is true(1) and return
     * _incInters[0] if @p isUp is false(0).
     */
    Intersection* intersection(bool isUp) const
    {
        return _incInters[isUp];
    }

    /**
     * @~japanese
     * 接続する交差点のうち，単路部 @p another と共有されるものを戻す
     *
     * @~english
     * Return the connecting intersection that is shared with the section
     * @p another
     */
    Intersection* sharedIntersection(const Section* another) const;

    /**
     * @~japanese
     * 接続する交差点のうち @p inter 以外の残りのひとつを戻す
     *
     * @~english
     * Return the remaining one of the connecting intersections other than
     * @p inter
     */
    Intersection* anotherIntersection(const Intersection* inter) const
    {
        for (unsigned int i = 0; i < 2; i++)
        {
            if (_incInters[i] == inter)
            {
                return _incInters[(i + 1) % 2];
            }
        }
        return nullptr;
    }

    /**
     * @~japanese
     * @p lane の下流レーンが所属する交差点を戻す
     *
     * @~english
     * Return the intersection that the downstream lane of @p lane belongs to
     */
    Intersection* nextIntersection(const Lane* lane) const
    {
        if (containsNextLane(lane))
        {
            return nullptr;
        }
        return intersection(isUp(lane));
    }

    /**
     * @~japanese
     * @p lane の上流レーンが所属するレーン束オブジェクトを戻す
     *
     * @~english
     * Return the intersection that the upstream lane of @p lane belongs to
     */
    Intersection* previousIntersection(const Lane* lane) const
    {
        if (containsPreviousLane(lane))
        {
            return nullptr;
        }
        return intersection(!isUp(lane));
    }

    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:
    /**
     * @~japanese
     * レーン @p lane から下流交差点の境界方向 @p direction へのパスが存在するか
     * どうかを戻す
     *
     * @~english
     * Return whether there is a path that from @p lane to the border direction
     * @p direction in the downstream intersection
     * 
     */
    bool hasValidPath(const Lane* lane, int direction) const;

    /**
     * @~japanese レーン @p lane が上り向きかどうかを戻す
     * @~english  Return whether @p lane is in the ascending direction
     */
    bool isUp(const Lane* lane) const;

    /**
     * @~japanese
     * 交差点 @p from から交差点 @p to への方向が上りかどうかを戻す
     *
     * @~english
     * Return whether the direction from intersection @p from to intersection
     * @p to is up.  
     */
    bool isUp(const Intersection* from, const Intersection* to) const;

    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:
    /**
     * @~japanese
     * 境界方向 @p dir の流入レーン数を戻す
     *
     * @~english
     * Return the number of inflow lanes in border direction @p dir
     */
    int numIn(unsigned int dir) const
    {
        assert(dir == 0 || dir == 1);
        return _numIn[dir];
    }

    /**
     * @~japanese
     * 境界方向 @dir の流入車線数を @p num に設定する
     *
     * @~english
     * Set the number of inflow lanes in border direction @p dir to @p num  
     */
    void setNumIn(unsigned int dir, int num)
    {
        assert(dir == 0 || dir == 1);
        _numIn[dir] = num;
    }

    /**
     * @~japanese 境界方向 @p dir の流出レーン数を戻す
     * @~english  Return the number of outflow lanes in border direction @p dir
     */
    int numOut(unsigned int dir) const
    {
        assert(dir == 0 || dir == 1);
        return _numOut[dir];
    }

    /**
     * @~japanese
     * 境界方向 @dir の流出車線数を @p num に設定する
     *
     * @~english
     * Set the number of outflow lanes in border direction @p dir to @p num  
     */
    void setNumOut(unsigned int dir, int num)
    {
        assert(dir == 0 || dir == 1);
        _numOut[dir] = num;
    }

    /**
     * @~japanese レーン @p lane の下流レーンの集合を戻す
     * @~english  Return the set of downstream lanes of @p lane
     */
    std::vector<const Lane*> nextLanes(const Lane* lane) const;

    /**
     * @~japanese レーン @p lane の上流レーンの集合を戻す
     * @~english  Return the set of upstream lanes of @p lane
     */
    std::vector<const Lane*> previousLanes(const Lane* lane) const;

    /**
     * @~japanese 指定された方向のレーンの集合を戻す
     * @~english  Return the set of lanes in the specified direction
     */
    std::vector<const Lane*> lanesWithDirection(bool isUp) const;

    /**
     * @~japanese 交差点 @p inter から流入するレーンの集合を戻す
     * @~english  Return the set of lanes flowing from @p inter
     */
    std::vector<const Lane*> lanesFrom(const Intersection* inter) const;

    /**
     * @~japanese 交差点 @p inter へ流出するレーンの集合を戻す
     * @~english  Return the set of lanes flowing out to @p inter
     */
    std::vector<const Lane*> lanesTo(const Intersection* inter) const;

    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    /**
     * @~japanese
     * サブセクション @p subsec の @p edgeNum 番目の辺に接するサブセクションを
     * 戻す
     *
     * @note 隣接する交差点も検索する
     * 
     * @~english
     * Return subsection that touches the @p edgeNum -th edge of subsection
     * @p subsec
     *
     * @note Search also adjacent intersections 
     */
    SubLaneBundle* pairedSubLaneBundle(
        SubLaneBundle* subsec, int edgeNum) const override;

    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    /**
     * @~japanese 制限速度情報 @p item を追加する
     * @~english  Add speed limit information @p item
     */
    void addSpeedLimitItem(const SpeedLimitItem* item)
    {
        _speedLimitList.emplace_back(item);
    }

    ///@}

    //==========================================================================
    /**
     * @~japanese @name エージェントや経路探索に関する関数群
     * @~english  @name Functions related to agents and routing
     */
    ///@{

public:
    /**
     * @~japanese
     * 位置 @p begin から長さ @p length の区間を上り方向に進行する
     * エージェント数を戻す
     *
     * @~english
     * Return the number of agents traveling in the ascending direction in the
     * interval of length @p length from position @p begin
     */
    int numUpAgents(double begin, double length) const
    {
        return _numAgents(begin, length, true);
    }

    /**
     * @~japanese
     * 位置 @p begin から長さ @p length の区間を下り方向に進行する
     * エージェント数を戻す
     *
     * @~english
     * Return the number of agents traveling in the descending direction in the
     * interval of length @p length from position @p begin
     */
    int numDownAgents(double begin, double length) const
    {
        return _numAgents(begin, length, false);
    }

protected:
    /**
     * @~japanese
     * 位置 @p begin から長さ @p length の区間に存在するエージェント数を戻す
     *
     * 上り方向であるかどうかを @p isUp で指定する．
     *
     * @~english
     * Returns the number of agents in the interval of length @p length from
     * position @p begin
     *
     * Specify with @p isUp whether it is ascending direction or not.
     */
    int _numAgents(double start, double distance, bool isUp) const;

public:
    /**
     * @~japanese
     * 交差点 @p inter から流入したすべてのレーンで車線変更実行可能かどうかを
     * 戻す
     *
     * @~english
     * Return whether lane-changes are executable in all lanes flowing from
     * @p inter
     */
    bool canAcceptLaneShift(const Intersection* inter) const;

    /**
     * @~japanese この単路部に対応する経路探索用ノード @p node を登録する
     * 
     * 上り方向であるかどうかを @p isUp で指定する．
     * 
     * @~english  Register a @p node for routing corresponding to this section
     * 
     * Specify with @p isUp whether it is ascending direction or not.
     */
    void addRoutingNode(bool isUp, RoutingNode* node)
    {
        _routingNodes[isUp] = node;
    }

    /**
     * @~japanese 車種 @p type をAllowリストに追加する
     *
     * 上り方向であるかどうかを @p isUp で指定する．
     *
     * @~english  Add vehicle type @p type to allow-list
     *
     * Specify with @p isUp whether it is ascending direction or not.
     */
    void addAllowedVehicleType(bool isUp, const VehicleType& type)
    {
        _restrictions[isUp].addAllowedVehicleType(type);
    }

    /**
     * @~japanese 車種 @p type をDenyリストに追加する
     *
     * 上り方向であるかどうかを @p isUp で指定する．
     *
     * @~english  Add vehicle type @p type to deny-list
     *
     * Specify with @p isUp whether it is ascending direction or not.
     */
    void addDeniedVehicleType(bool isUp, const VehicleType& type)
    {
        _restrictions[isUp].addDeniedVehicleType(type);
    }

    /**
     * @~japanese
     * 車種 @p type の車両が @p isUp 方向へ通行可能かどうかを戻す
     * 
     * @~english
     * Return whether vehicle with type @p type can pass in direction @p isUp 
     */
    bool permitsPassing(bool isUp, const VehicleType& type) const;

    /**
     * @~japanese 車種 @p type の探索確率を @p prob に設定する
     *
     * 上り方向であるかどうかを @p isUp で指定する．
     *
     * @~english  Set search probability of vehicle type @p type to @ prob
     *
     * Specify with @p isUp whether it is ascending direction or not.
     */
    void addRoutingProbability(bool isUp, const VehicleType& type, double prob);

    /**
     * @~japanese 車種 @p type の車両の経路選択確率を戻す
     *
     * 上り方向であるかどうかを @p isUp で指定する．
     *
     * @~english  Return routing probability for vehicles type @p type
     *
     * Specify with @p isUp whether it is ascending direction or not.
     */
    double routingProbability(bool isUp, VehicleType type) const;

    /**
     * @~japanese
     * 経路探索時に車種 @p type の車両が交差点 @p inter からこの単路部を通じて
     * 次の交差点に探索を展開する確率を戻す
     *
     * @~english
     * Return the probability that a vehicle of vehicle type @p type will expand
     * the search from @p inter to the next intersection via this section when
     * routing
     */
    double routingProbability(Intersection* inter, VehicleType type) const;

    /**
     * @~japanese 平均速度を戻す
     *
     * 上り方向であるかどうかを @p isUp で指定する．
     *
     * @~english  Return mean speed
     *
     * Specify with @p isUp whether it is ascending direction or not.
     */
    double averageVelocity(bool isUp) const;

    ///@}

    /**
     * @~japanese 属性を @p out に出力する
     * @~english  Output attributes to @p out
     */
    void print(std::ostream& out) const;

    //==========================================================================
protected:
    /**
     * @~japanese 中心点（参照点）
     * @~english  Center point (reference point)
     */
    amu::geometry::AmuPoint _center;

    /**
     * @~japanese 代表長さ
     *
     * 両境界の中点どうしの距離として定義される．
     *
     * @todo 単路部をn角形(n>4)にする場合には要変更
     * 
     * @~english  Representative length
     *
     * Defined as the distance between the midpoints of both borders.
     */
    double _length;

    /**
     * @~japanese 接続する交差点
     * @~english  Connected intersections
     */
    Intersection* _incInters[2];

    /**
     * @~japanese リンク交通流の記録
     * 
     * リンク旅行時間の算出に用いる．
     *   
     * @~english  Link traffic flow record
     * 
     * Used to calculate link travel time.
     */
    LinkFlowRecord* _linkFlowRecords[2];

    /**
     * @~japanese 境界からの流入レーン数
     *
     * 隣接交差点の該当する境界の流出レーン数と一致する．
     *
     * @~english  Number of inflow lanes from border
     *
     * Matches the number of outflow lanes of the corresponding border of the
     * connected intersection.
     */
    int _numIn[2];

    /**
     * @~japanese 境界への流出レーン数
     *
     * 隣接交差点の該当する境界の流入レーン数と一致する．
     *
     * @~english  Number of outflow lanes to border
     *
     * Matches the number of inflow lanes of the corresponding border of the
     * connected intersection.
     */
    int _numOut[2];

    /**
     * @~japanese レーン幅 [m]
     * @~english  Lane width [m]
     */
    double _laneWidth;

    /**
     * @~japanese 路肩幅 [m]
     * @~english  Shoulder width [m]
     */
    double _roadsideWidth;

    /**
     * @~japanese 歩道幅 [m]
     *
     * インデックス0は右，1は左をあらわす．
     *
     * @~english  Sidewalk width [m]
     *
     * Index 0 indicates right, and 1 indicates left side.
     */
    double _sidewalkWidth[2];

    /**
     * @~japanese 対応するランク0の経路探索用ノード
     * @~english  Corresponding routing node with rank 0
     */
    RoutingNode* _routingNodes[2];

    /**
     * @~japanese 通行権
     * 
     * インデックス0は下り方向，1は上り方向をあらわす．
     *
     * @~english  Right-of-way
     * 
     * Index 0 indicates descending direction, and 1 indicates ascending
     * direction.
     */
    VehicleRestriction _restrictions[2];

    /**
     * @~japanese 制限速度 [km/h]
     *
     * インデックス0は下り方向，1は上り方向をあらわす．
     *
     * @~english  Speed limit [km/h]
     *
     * Index 0 indicates descending direction, and 1 indicates ascending
     * direction.
     */
    double _speedLimit[2];

    /**
     * @~japanese 制限速度情報を保持するコンテナ
     * 
     * このうちの1つから_speedLimit が決定される．
     * 
     * @~english  Container storing speed limit information
     * 
     * _speedLimit is determined from one of these.
     */
    std::vector<const SpeedLimitItem*> _speedLimitList;

    /**
     * @~japanese 経路探索時の探索確率テーブル
     *
     * インデックス0は下り方向，1は上り方向をあらわす．
     *
     * @todo RoutingProbabilityに代替
     *
     * @~english  Search probability table for routing
     *
     * Index 0 indicates descending direction, and 1 indicates ascending
     * direction.
     */
    std::map<VehicleType, double> _routingProbability[2];

    //==========================================================================
    /**
     * @~japanese @name 観測器
     * @~english  @name Monitoring devices
     */
    ///@{
    /**
     * @~japanese リンク交通流観測器
     * @~english  Link traffic flow monitor
     */
    LinkFlowMonitor* _linkFlowMonitors[2];

    ///@}

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

    LinkFlowRecord* linkFlowRecord(unsigned int i) const
    {
        assert(i == 0 || i == 1);
        return _linkFlowRecords[i];
    }

    void setLinkFlowRecord(unsigned int i, LinkFlowRecord* record)
    {
        assert(i == 0 || i == 1);
        _linkFlowRecords[i] = record;
    }

    double length() const
    {
        return _length;
    }

    double laneWidth() const
    {
        return _laneWidth;
    }

    void setLaneWidth(double width)
    {
        _laneWidth = width;
    }

    double roadsideWidth() const
    {
        return _roadsideWidth;
    }

    void setRoadsideWidth(double width)
    {
        _roadsideWidth = width;
    }

    double sidewalkWidth(bool isLeft)
    {
        return _sidewalkWidth[isLeft];
    }

    void setSidewalkWidth(bool isLeft, double width)
    {
        _sidewalkWidth[isLeft] = width;
    }

    const VehicleRestriction& restriction(bool isUp) const
    {
        return _restrictions[isUp];
    }

    double speedLimit(bool isUp) const
    {
        return _speedLimit[isUp];
    }

    void setSpeedLimit(bool isUp, double speedLimit);

    const std::map<VehicleType, double>& routingProbabilities(bool isUp) const
    {
        return _routingProbability[isUp];
    }

    LinkFlowMonitor* linkFlowMonitor(bool isUp) const
    {
        return _linkFlowMonitors[isUp];
    }

    void setLinkFlowMonitor(bool isUp, LinkFlowMonitor* monitor)
    {
        _linkFlowMonitors[isUp] = monitor;
    }

    ///@}

#ifdef INCLUDE_TRAMS
    //==========================================================================
    /**
     * @~japanese @name 路面電車用拡張
     * @~english  @name Extension for tram
     */
    ///@{
protected:
    SectionTramExt* _tramExt;

public:
    SectionTramExt* tramExt()
    {
        return _tramExt;
    }

    ///@}
#endif //INCLUDE_TRAMS
};

#endif //__SECTION_H__
