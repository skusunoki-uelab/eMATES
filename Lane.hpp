/* **************************************************************************
 * Copyright 2014-2024 ADVENTURE Project
 * All Rights Reserved
 * ************************************************************************** */
/**
 * @file Lane.hpp
 */
#ifndef __LANE_HPP__
#define __LANE_HPP__
#include "Config.hpp"
#include "Connector.hpp"
#ifdef INCLUDE_PEDESTRIANS
#include "ped/LanePedExt.hpp"
#endif //INCLUDE_PEDESTRIANS
#include <AmuInterval.hpp>
#include <AmuLineSegment.hpp>
#include <algorithm>
#include <iostream>
#include <string>
#include <unordered_map>
#include <vector>
#ifdef _OPENMP
#include <omp.h>
#endif //_OPENMP

class LaneBundle;
class ObjectInLane;
class TrafficCounterComponent;
class Vehicle;

//##############################################################################
/**
 * @~japanese
 * レーン同士の横方向の相対位置に関する列挙型を定義する構造体
 *
 * @note
 * おもに車線変更時に利用する
 *
 * @~english
 * Struct defining enumerations about the relative horizontal positions of lanes
 *
 * @note
 * Mainly used at lane-change
 */
struct LanePosition
{
public:
    /**
     * @~japanese レーン同士の横方向の相対位置をあらわす列挙型
     * @note キャストせず配列のインデックスとして使うため enum class にしない
     *
     * @~english  Enumerations about the relative horizontal positions of lanes
     * @note Non-enum-class to enable to use as array indexes without casting
     */
    enum Type
    {
        Left   = 0,
        Center = 1,
        Right  = 2,
    };
};

//##############################################################################
/**
 * @~japanese 仮想走行レーン
 *
 * 原則として，車両の軌跡を表す．車線変更時のみレーンと横方向のずれが生じるが，
 * その場合であっても車線はかならず1つのレーンに所属している．したがって，車両は
 * 原則として所属レーンの1次元空間を移動する．シミュレーション空間におけるある
 * 点から別の点へ向かう線分 (厳密にはAmuLineSegmentおよびその派生クラス) に
 * よって形状が定義される．単路部内では車線の概念とほぼ同等である．交差点に
 * おいては，右折・左折・直進などの進行方向に対応するレーンが設置される．
 *
 * @~english  Virtual driving lane
 *
 * In principle, represents the trajectory of the vehicle. A lateral deviation
 * from the lane occurs only when changing lanes, but even in that case, one
 * vehicle always belongs to one lane. Therefore, the vehicle moves in the one-
 * dimensional space of the lane to which it belongs. Its shape is defined by a
 * line segment (strictly AmuLineSegment class and its derived classes) from one
 * point to another in the simulation space. Within sections, it is almost
 * equivalent to the concept of real lane. At intersections, lanes corresponding
 * to the running direction, such as right turn, left turn, and go straight,
 * are installed.
 *
 * @~ @ingroup RoadMap
 */
class Lane
{
public:
    Lane(
        const std::string& id, const Connector* ptBegin, const Connector* ptEnd,
        amu::geometry::AmuLineSegment* ptLineSegment, LaneBundle* parent);
    virtual ~Lane();

    //==========================================================================
    /**
     * @~japanese @name レーン自身に関する関数群
     * @~english  @name Functions for lane itself
     */
    ///@{
public:
    /**
     * @~japanese レーンの長さを戻す
     * @~english  Return lane length
     */
    double length() const
    {
        return _lineSegment->length();
    }

    /**
     * @~japanese 方向ベクトルを戻す
     * @~english  Return direction vector
     */
    const amu::math::AmuVector directionVector() const
    {
        return _lineSegment->directionVector();
    }

    /**
     * @~japanese 位置 @p distance における方向ベクトルを戻す
     * @note ポリラインや曲線で表される場合に用いる
     *
     * @~english  Return direction vector at the position @p distance
     * @note Used when represented by polylines or curves 
     */
    const amu::math::AmuVector directionVector(double distance) const
    {
        return _lineSegment->directionVector(distance);
    }

    /**
     * @~japanese 始点側の境界番号を戻す
     * @~english  Return the border number of the start point side
     */
    virtual int beginDirection() const = 0;

    /**
     * @~japanese レーン終点側の境界番号を戻す
     * @~english  Return the border number of the end point side
     */
    virtual int endDirection() const = 0;

    /**
     * @~japanese
     * 始点から距離 @p distance の位置にある点を戻す
     *
     * @~english
     * Return a point at the distance @p distance from the start point  
     */
    amu::geometry::AmuPoint position(double distance) const
    {
        return _lineSegment->position(distance);
    }

    /**
     * @~japanese
     * 線分 @p line との交点を戻す
     *
     * @~english
     * Return the intersection point with the line segment @p line
     */
    bool createIntersectionPoint(
        const amu::geometry::AmuLineSegment* line,
        amu::geometry::AmuPoint*             result_point) const
    {
        return _lineSegment->createIntersectionPoint(line, result_point);
    }

    /**
     * @~japanese
     * レーンを @p d0 : @p d1 に内分する点を戻す
     * 
     * @~english
     * Return a point that divides the lane into @p d0 : @p d1
     */
    amu::geometry::AmuPoint createInteriorPoint(double d0, double d1) const
    {
        return _lineSegment->createInteriorPoint(d0, d1);
    }

    /**
     * @~japanese
     * レーン上で点 @p point にもっとも近い点を戻す 
     *
     * @~english
     * Return the nearest point on the lane to point @p point
     */
    const amu::geometry::AmuPoint calcNearestPoint(
        amu::geometry::AmuPoint point) const;

    /**
     * @~japanese 勾配を戻す
     * @~english  Return gradient
     */
    double gradient() const
    {
        return _lineSegment->gradient();
    }

    ///@}

    //==========================================================================
    /**
     * @~japanese レーンの所属と接続に関する関数群
     * @~english  Functions related to lane membership and connection
     */
    ///@{
public:
    /**
     * @~japanese レーンの交錯関係を設定する
     * @~english  Set lane intersections
     */
    virtual bool setCollision()
    {
        return true;
    }

    /**
     * @~japanese 下流レーン @p lane を追加する
     * @~english  Add downstream lane @p lane
     */
    void addNextLane(const Lane* lane)
    {
        _nextLanes.emplace_back(lane);
    }

    /**
     * @~japanese @p i 番目の下流レーンを戻す
     * @~english  Return @p i -th downstream lane
     */
    const Lane* nextLane(int i) const
    {
        return _nextLanes[i];
    }

    /**
     * @~japanese 上流レーン @p lane を追加する
     * @~english  Add upstream lane @p lane
     */
    void addPreviousLane(const Lane* lane)
    {
        _previousLanes.emplace_back(lane);
    }

    /**
     * @~japanese @p i 番目の上流レーンを戻す
     * @~english  Return @p i -th upstream lane
     */
    const Lane* previousLane(int i) const
    {
        return _previousLanes[i];
    }

    ///@}

    //==========================================================================
    /**
     * @~japanese @name レーン内のエージェントの操作に関する関数群
     * @~english  @name Functions related to operations for agents in the lane
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
    void makeAgentsPreperceive();

    /**
     * @~japanese エージェントに周囲の状況を認知させる
     * @~english  Let agents recognize circumstances
     */
    void makeAgentsPerceive();

    /**
     * @~japanese エージェントに意思決定させる
     * @~english  Let agents make decisions
     */
    void makeAgentsDetermine();

    /**
     * @~japanese エージェントに行動を実行させる
     * @~english  Let agents perform actions
     */
    void makeAgentsAct();

    /**
     * @~japanese 行動後に必要なエージェントの処理を実行する
     * @~english  Perform necessary agent processing after action
     */
    void makeAgentsPostact();

    /**
     * @~japanese レーンの先頭のエージェントを戻す
     * @~english  Return the head agent of the lane
     */
    const ObjectInLane* headAgent() const;

    /**
     * @~japanese レーンの先頭の車両を戻す
     *
     * @note
     * 先頭が車両でない場合は先頭から走査して最初に見つかった車両を戻す
     *
     * @~english  Return the head vehicle of the lane
     *
     * @note
     * If the head is not a vehicle, scan from the head and return the first
     * vehicle found.
     */
    const Vehicle* headVehicle() const;

    /**
     * @~japanese レーンの末尾のエージェントを戻す
     * @~english  Return the tail agent of the lane
     */
    const ObjectInLane* tailAgent() const;

    /**
     * @~japanese レーンの末尾の車両を戻す
     *
     * @note
     * 末尾が車両でない場合は末尾から走査して最初に見つかった車両を戻す
     *
     * @~english  Return the tail vehicle of the lane
     *
     * @note
     * If the tail is not a vehicle, scan from the tail and return the first
     * vehicle found.
     */
    const Vehicle* tailVehicle() const;

    /**
     * @~japanese @p agent のひとつ前方のエージェントを戻す
     * @~english  Return the agent just in front of p agent
     */
    const ObjectInLane* frontAgent(const ObjectInLane* agent) const;

    /**
     * @~japanese @p agent のひとつ前方の車両を戻す
     *
     * @note
     * ひとつ前方が車両でない場合は @p agent から前方に向けて調査し，最初に
     * 見つかった車両を戻す
     *
     * @~english  Return the vehicle just in front of @p agent
     *
     * @note
     * If just in front is not a vehicle, scan forward from @p agent and return
     * the first vehicle found.
     */
    const Vehicle* frontVehicle(const ObjectInLane* agent) const;

    /**
     * @~japanese @p agent のひとつ前方のエージェントを，レーンを越えて検索する
     * @param threshold 検索を打ち切る距離
     * @param[out] result_agent 見つかった先行エージェント
     * @param[out] result_distance 見つかったエージェントまでの距離
     *
     * @todo 複数の値を返す関数の検討
     *
     * @~english  Find the agent just in front of @p agent across lanes
     * @param threshold Threshold distance to cut off search
     * @param[out] result_agent Preceding agent found
     * @param[out] result_distance Distance to preceding agent found
     */
    void getFrontAgentFar(
        const ObjectInLane* agent, double threshold,
        const ObjectInLane** result_agent, double* result_distance) const;

    /**
     * @~japanese 位置 @p distance のひとつ前方のエージェントを戻す
     * @~english  Return the agent just in front of the position @p distance  
     */
    const ObjectInLane* frontAgent(double distance) const;

    /**
     * @~japanese 位置 @p distance のひとつ前方の車両を戻す
     *
     * @note
     * ひとつ前方が車両でない場合は位置 @p distance から前方に向けて調査し
     * 最初に見つかった車両を戻す
     *
     * @~english  Return the vehicle just in front of the position @p distance 
     *
     * @note
     * If just in front is not a vehicle, scan forward from the position
     * @p distance and return the first vehicle found.
     */
    const Vehicle* frontVehicle(double distance) const;

    /**
     * @~japanese
     * 位置 @p distance のひとつ前方のエージェントを，レーンを越えて検索する
     *
     * @param threshold 検索を打ち切る距離
     * @param[out] result_agent 見つかったエージェント
     * @param[out] result_distance 見つかったエージェントまでの距離
     *
     * @todo 複数の値を返す関数の検討
     *
     * @~english
     * Find the agent just in front of the position @p distance across lanes
     *
     * @param threshold Threshold distance to cut off search
     * @param[out] result_agent Preceding agent found
     * @param[out] result_distance Distance to preceding agent found
     */
    void getFrontAgentFar(
        double distance, double threshold, const ObjectInLane** result_agent,
        double* result_distance) const;

    /**
     * @~japanese @p agent のひとつ後方のエージェントを戻す
     * @~english  Return the agent just in the rear of @p agent
     */
    const ObjectInLane* followingAgent(const ObjectInLane* agent) const;

    /**
     * @~japanese @p agent のひとつ後方の車両を戻す
     *
     * @note
     * ひとつ後方が車両でない場合は @p agent から後方に向けて調査し，最初に
     * 見つかった車両を戻す
     *
     * @~english  Return the vehicle just in front of @p agent
     *
     * @note
     * If just in the rear is not a vehicle, scan backward from @p agent and
     * return the first vehicle found.
     */
    const Vehicle* followingVehicle(const ObjectInLane* agent) const;

    /**
     * @~japanese @p agent のひとつ後方のエージェントを，レーンを越えて検索する
     * @param threshold 検索を打ち切る距離
     * @param[out] result_agent 見つかったエージェント
     * @param[out] result_distance 見つかったエージェントまでの距離
     *
     * @todo 複数の値を返す関数の検討
     *
     * @~english  Find the agent just in the rear of @p agent across lanes
     * @param threshold Threshold distance to cut off search
     * @param[out] result_agent Following agent found
     * @param[out] result_distance Distance to following agent found
     */
    void getFollowingAgentFar(
        const ObjectInLane* agent, double threshold,
        const ObjectInLane** result_agent, double* result_distance) const;

    /**
     * @~japanese 位置 @p distance のひとつ後方のエージェントを戻す
     * @~english  Return the agent just in the rear of the position @p distance  
     */
    const ObjectInLane* followingAgent(double distance) const;

    /**
     * @~japanese
     * 位置 @p distance のひとつ後方の車両を戻す
     *
     * @note
     * ひとつ後方が車両でない場合は位置 @p position から後方に向けて調査し，
     * 最初に見つかった車両を戻す
     *
     * @~english
     * Return the vehicle just in the rear of the position @p distance 
     *
     * @note
     * If just in the rear is not a vehicle, scan forward from the position
     * @p distance and return the first vehicle found.
     */
    const Vehicle* followingVehicle(double distance) const;

    /**
     * @~japanese
     * 位置 @p distance のひとつ後方のエージェントを，レーンを越えて検索する
     *
     * @param threshold 検索を打ち切る距離
     * @param[out] result_agent 見つかったエージェント
     * @param[out] result_distance 見つかったエージェントまでの距離
     *
     * @todo 複数の値を返す関数の検討
     *
     * @~english
     * Find the agent just in the rear of the position @p distance across lanes
     *
     * @param threshold Threshold distance to cut off search
     * @param[out] result_agent Following agent found
     * @param[out] result_distance Distance to following agent found
     */
    void getFollowingAgentFar(
        double distance, double threshold, const ObjectInLane** result_agent,
        double* result_distance) const;

    /**
     * @~japanese @p agent をレーンに所属させる
     * @~english  Make @p agent belong to the lane
     */
    bool putAgent(ObjectInLane* agent);

    /**
     * @~japanese 新たに追加するエージェントとして @p agent を登録する
     * @~english  Register @p agent as an agent to be newly added
     */
    bool registerAgentToAdd(ObjectInLane* agent);

    /**
     * @~japanese このレーンで車線変更実行可能かどうか
     * @~english  Whether lane-changes are executable in this lane
     */
    bool canAcceptLaneShift() const;

    /**
     * @~japanese エージェント密度 [agent/km] を戻す
     *
     * レーン内に車両のみが存在する場合は車両密度に等しい
     *
     * @~english  Return agent density [agent/km]
     *
     * Equal to vehicle density when there are only vehicles in the lane
     */
    double density() const
    {
        return _agents.size() * 1000.0 / _lineSegment->length();
    }

    /**
     * @~japanese 車両の空間平均速度 [m/ms] を戻す
     * @~english  Return the spacial mean speed [m/ms] of vehicles
     */
    double averageVel() const;

    /**
     * @~japanese レーンに存在するすべてのエージェントを消去する
     * @note ODノードにおいて車両を削除する際に用いる
     *
     * @~english  Clear all agents present in the lane
     * @note Used when deleting a vehicle in the ODNode.
     */
    void clearAgents()
    {
        _agents.clear();
    }

    ///@}

    //==========================================================================
    /**
     * @~japanese @name 観測器に関する関数群
     * @~english  @name Functions related to monitoring devices
     */
    ///@{
    /**
     * @~japanese
     * 位置 @p distance の感知器コンポーネント @p counter を登録する
     *
     * @~english
     * Register traffic counter component @p counter at position
     * @p distance
     */
    void addTrafficCounter(double distance, TrafficCounterComponent* counter);

    /**
     * @~japanese
     * @p oldDistance と @p distance に挟まれる感知器コンポーネントを取得し，
     * @p result_counters に格納する
     *
     * @~english
     * Get traffic counter components between @p oldDistance and @p distance
     * and store them in @p result_component . 
     */
    void counters(
        std::vector<TrafficCounterComponent*>* result_counters,
        double oldDistance, double distance) const;

    ///@}

    //==========================================================================
    /**
     * @~japanese
     * @name 派生する LaneInIntersection クラスで実装する関数群
     *
     * @~english
     * @name Functions implemented by derived LaneInIntersection class
     */
    ///@{
    virtual void addCollisionLanesInIntersection(const Lane*) {}
    virtual void addCollisionLanesInSection(const Lane*) {}
    ///@}

    /**
     * @~japanese @name 派生する LaneInSection クラスで実装する関数群
     * @~english  @name Functions implemented by derived LaneInSection class
     */
    ///@{
    virtual void addSideLane(
        amu::math::AmuInterval, const Lane*, LanePosition::Type)
    {
    }
    virtual bool isSideLaneFound(const Lane*, LanePosition::Type) const
    {
        return false;
    }
    virtual void getSideLaneDistance(
        double, LanePosition::Type, const Lane**, double*) const
    {
    }
    virtual double lengthOnSideLane(double, const Lane*) const
    {
        return 0.0;
    };
    virtual const Lane* leftLane(double) const
    {
        return nullptr;
    }
    virtual const Lane* rightLane(double) const
    {
        return nullptr;
    }
    virtual const Lane* sideLane(LanePosition::Type, double) const
    {
        return nullptr;
    }
    ///@}

    //==========================================================================
    /**
     * @~japanese レーンの属性を @p out に出力する
     * @~english  Output lane attributes to @p out 
     */
    virtual void print(std::ostream& out) const;

    //==========================================================================
protected:
    /**
     * @~japanese 識別番号
     *
     * このレーンが含まれるオブジェクト内で一意
     *
     * @~english  ID number
     *
     * Unique within the object that contains this lane.
     */
    std::string _id;

    /**
     * @~japanese このレーンが含まれるレーン束オブジェクト
     * @~english  Lane bundle object that contains this lane
     */
    LaneBundle* _parent;

    /**
     * @~japanese このレーンの直下流レーンの集合
     * @~english  Set of lanes immediately downstream of this lane
     */
    std::vector<const Lane*> _nextLanes;

    /**
     * @~japanese 舵角が最小の直下流レーン
     * @~english  Immediately downstream lane with the smallest steering angle
     */
    const Lane* _nextStraightLane;

    /**
     * @~japanese このレーンの直上流レーンの集合
     * @~english  Set of lanes immediately upstream of this lane
     */
    std::vector<const Lane*> _previousLanes;

    /**
     * @~japanese 舵角が最小の直上流レーン
     * @~english  Immediately downstream lane with the smallest steering angle 
     */
    const Lane* _previousStraightLane;

    /**
     * @~japanese 制限速度 [km/h]
     * @~english  Speed limit [km/h]
     */
    double _speedLimit;

    /**
     * @~japanese 最新の車両流入時刻 [ms]
     * @~english  Latest vehicle inflow time [ms]
     */
    ulint _lastArrivalTime;

    /**
     * @~japanese 始端コネクタ
     * @~english  Start connector
     */
    const Connector* _beginConnector;

    /**
     * @~japanese 終端コネクタ
     * @~english  End connector
     */
    const Connector* _endConnector;

    /**
     * @~japanese 形状を表す線分
     * @~english  Line segment representing shape
     */
    const amu::geometry::AmuLineSegment* _lineSegment;

    /**
     * @~japanese このレーンに配置されているオブジェクトの集合
     *
     * 行動プロセスの後，このレーンに継続して配置されるオブジェクトは
     * _tmpAgent にコピーされ，別のレーンに移るオブジェクトは移動先のレーンの
     * _tmpAgentsToAdd にコピーされる．次のステップの renewAgentOrder でこれら
     * 2つのコンテナが統合される．
     *
     * @~english  Set of objects placed in this lane
     *
     * After the act process, objects that continue to be placed in this lane
     * are copied to _tmpAgent, and objects that move to another lane are copied
     * to _tmpAgentsToAdd of the destination lane. The next step renewAgentOrder
     * merges these two containers.
     */
    std::vector<ObjectInLane*> _agents;

    /**
     * @~japanese
     * 次のステップでこのレーンに継続的に配置されてるオブジェクトのコンテナ
     *
     * @~english
     * A container for objects that are continuously placed in this lane in the
     * next step
     */
    std::vector<ObjectInLane*> _tmpAgents;

    /**
     * @~japanese
     * 次のステップでこのレーンの _agents にコピーすべきオブジェクトのコンテナ
     *
     * @~english
     * A container for objects to be added to _agents in this lane
     */
    std::vector<ObjectInLane*> _tmpAgentsToAdd;

    //==========================================================================
    /**
     * @~japanese @name 観測器
     * @~english  @name Monitoring devices
     */
    ///@{
    /**
     * @~japanese 車両感知器
     * 
     * @attention
     * サブコンテナでありデストラクタでのdeleteは不要
     *
     * @~english  Traffic counter
     *
     * @attention
     * Since it is a subcontainer, there is no need to delete it in the
     * destructor.
     */
    std::unordered_map<double, TrafficCounterComponent*> _counters;
    ///@}

#ifdef _OPENMP
    /**
     * @~japanese ロック変数
     * @~english  Lock variable
     */
    omp_lock_t _lock;
#endif //_OPENMP

    //==========================================================================
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

    void setId(const std::string& id)
    {
        _id = id;
    }

    LaneBundle* parent() const
    {
        return _parent;
    }

    const std::vector<const Lane*>& nextLanes() const
    {
        return _nextLanes;
    }

    const Lane* nextStraightLane() const
    {
        return _nextStraightLane;
    }

    void setNextStraightLane(const Lane* lane)
    {
        _nextStraightLane = lane;
    }

    const std::vector<const Lane*>& previousLanes() const
    {
        return _previousLanes;
    }

    const Lane* previousStraightLane() const
    {
        return _previousStraightLane;
    }

    void setPreviousStraightLane(const Lane* lane)
    {
        _previousStraightLane = lane;
    }

    const Connector* beginConnector() const
    {
        return _beginConnector;
    }

    const Connector* endConnector() const
    {
        return _endConnector;
    }

    const amu::geometry::AmuLineSegment* lineSegment() const
    {
        return _lineSegment;
    }

    double speedLimit() const
    {
        return _speedLimit;
    }

    void setSpeedLimit(double limit)
    {
        _speedLimit = limit;
    }

    ulint lastArrivalTime() const
    {
        return _lastArrivalTime;
    }

    void setLastArrivalTime(ulint arrivalTime)
    {
        _lastArrivalTime = arrivalTime;
    }

    std::vector<ObjectInLane*>& agents()
    {
        return _agents;
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
    LanePedExt* _pedExt;

public:
    LanePedExt* pedExt() const
    {
        return _pedExt;
    }

    ///@}
#endif //INCLUDE_PEDESTRIANS
};

#endif //__LANE_HPP__
