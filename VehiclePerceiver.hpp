/* ******************************************************************
 * Copyright 2014-2024 ADVENTURE Project
 * All Rights Reserved
 * ****************************************************************** */
/**
 * @file VehiclePerceiver.hpp
 */
#ifndef __VEHICLE_PERCEIVER_H__
#define __VEHICLE_PERCEIVER_H__
#include <vector>

class Intersection;
class Lane;
class Section;
class Vehicle;
#ifdef INCLUDE_PEDESTRIANS
class VehiclePedExt;
#endif //INCLUDE_PEDESTRIANS

struct VehicleBehavior;
struct VehicleBodyProperty;
struct VehicleDecision;
struct VehicleGlobalRoute;
struct VehicleLocalRoute;
struct VehicleLocation;
struct VehicleScene;

//######################################################################
/**
 * @~japanese 自動車エージェントの認知機能モジュール
 *
 * VehicleScene を更新する．
 *
 * @~english  Recognition module of car agent
 *
 * Update VehicleScene.
 *
 * @~ @ingroup Vehicle
 */
class VehiclePerceiver
{
public:
    VehiclePerceiver();
    virtual ~VehiclePerceiver() {};

    /**
     * @~japanese 所有者とその部分クラスオブジェクトを登録する
     * @~english  Set owner and its part class objects
     */
    void setVehicle(
        Vehicle* vehicle, VehicleBehavior* behavior, VehicleBodyProperty* body,
        VehicleDecision* decision, VehicleGlobalRoute* globalRoute,
        VehicleLocalRoute* localRoute, VehicleLocation* location,
        VehicleScene* scene);

    //==================================================================
    /**
     * @~japanese @name 認知前処理
     * @~english  @name Pre-recognition processing
     */
    ///@{
public:
    /**
     * @~japanese 認知前の処理を行う
     * @~english  Perform pre-recognition processing
     */
    virtual void preperceive();

    ///@}

    //==================================================================
    /**
     * @~japanese @name 認知
     * @~english  @name Recognition
     */
    ///@{
public:
    /**
     * @~japanese 周囲の状況を認識する
     * @~english  Recognize surroundings
     */
    virtual void perceive();

protected:
    /**
     * @~japanese 希望速度を決定する
     * @~english  Determine desired velocity
     */
    void _determineDesiredVelocity();

    /**
     * @~japanese 先行エージェントを調べる
     * @param threshold 検索を打ち切る距離
     *
     * @note
     * Lane::getFrontAgentFar() に類似するが，LaneではnextStraightLaneを
     * 次々と検索するのに対し，ここではローカル経路に沿って検索する
     *
     * @~english  Search for preceding agent
     * @param threshold Distance to cut off searching
     *
     * @note
     * Similar to Lane::getFrontAgentFar() , but here it searches
     * along the local route, whereas the Lane's function searches
     * _nextStraightLane one after another.
     */
    void _searchFrontAgent(double threshold);

    /** 
     * @~japanese 前方の制限速度を調べる
     * @param threshold 検索を打ち切る距離
     *
     * @~english  Check the speed limit ahead
     * @param threshold Distance to cut off checking
     */
    void _searchFrontSpeedLimit(double threshold);

    /**
     * @~japanese 信号によって停止すべきか判断する
     * @param nextInter 次に進入ようとする交差点
     * @param prevInter 既に通過したあるいは通過中の交差点
     * @param threshold 検索を打ち切る距離
     * @return 次の交差点手前で停車するか
     *
     * @~english  Determine whether to stop at a traffic light
     * @param nextInter Intersection to enter next
     * @param prevInter Intersection already passed or passing
     * @param threshold Distance to stop searching
     * @return Whether to stop before the next intersection
     */
    bool _shouldStopBySignal(
        const Intersection* nextInter, const Intersection* prevInter,
        double threshold);

    /**
     * @~japanese
     * 交差点 @p nextInter 内の交錯レーン @p clInter にいる車両により
     * 停車すべきかどうか判断する
     *
     * @return 次の交差点手前で停車するか
     *
     * @~english
     * Determine Whether to stop due to cars in the collision lanes
     * @p clInter in the intersection @p nextInter
     *
     * @return Whether to stop before the next intersection
     */
    bool _shouldStopByCollisionInIntersection(
        const Intersection* nextInter, const std::vector<const Lane*>& clInter);

    /** 
     * @~japanese
     * 交差点 @p nextInter の接続する単路部内の交錯レーン
     * @p clSection にいる車両により停車すべきかどうか判断する
     *
     * @return 次の交差点手前で停車するか
     *
     * @~english
     * Determine Whether to stop due to cars in the collision lanes
     * @p clSection in the sections connecting to the intersection
     * @p nextInter
     *
     * @return Whether to stop before the next intersection
     */
    bool _shouldStopByCollisionInSection(
        const Intersection*             nextInter,
        const std::vector<const Lane*>& clSection);

    /**
     * @~japanese
     * 右左折時に交差点 @p nextInter 内にいる先行車により停車すべきか
     * どうか判断する
     *
     * @return 次の交差点手前で停車するか
     *
     * @~english
     * Determine Whether to stop due to the preceding car in the
     * intersection @p nextInter when turning
     * 
     * @return Whether to stop before the next intersection
     */
    bool _shouldStopByLaneInInter(const Intersection* nextInter);

    /**
     * @~japanese
     * 交差点通過後の単路部に十分な空きがあるか調べる
     *
     * 単路部内の最後尾の車が交差点に付近に停車しているとき，
     * 渋滞が交差点内まで延伸するのを防ぐため手前で停止する
     *
     * @return 次の交差点手前で停車するか
     *
     * @~english
     * Check if there is enough space on the section after passing
     * the intersection
     *
     * When the rearmost car in a section stops near the intersection,
     * stop before the intersection to prevent the congestion from
     * extending into it.
     *
     * @return Whether to stop before the next intersection
     */
    bool _shouldStopByShortSpace();

    /**
     * @~japanese
     * 交差点 @p nextInter において最小ヘッドウェイを維持するために
     * 停止するかどうか判断する
     *
     * 最小ヘッドウェイ（時間）が定義されている場合，右左折時には
     * 先行車が交差点に進入してから最小ヘッドウェイ以上の時間間隔を
     * 空けなければならない
     *
     * @return 次の交差点手前で停車するか
     *
     * @~english
     * Determine whether to stop to keep minimum headway
     *
     * If a minimum headway (time) is given, time interval of at least
     * the minimum headway must be kept after the preceding car enters
     * the intersection when turning.
     *
     * @return Whether to stop before the next intersection
     */
    bool _shouldStopByMinHeadway(const Intersection* nextInter);

    /**
     * @~japanese 右左折前に事前に減速する
     * @~english  Decelerate before turning
     */
    void _determineTurningVelocity();

    /**
     * @~japanese
     * 交差点内を走行中の優先エージェントを調べる
     *
     * @~english
     * Search for preferred agents traveling through the intersection
     */
    void _searchPreferentialAgentInIntersection();

    /**
     * @~japanese
     * 他車 @p other を「見る」ことができるかどうか調べる
     *
     * @return 他車が見えるかどうか
     *
     * @note
     * ここでいう「見る」とは，シミュレーションのシナリオにより
     * 目視によるものか情報通信によるものか意味合いが異なる
     *
     * @~english
     * Check if another car @p other is "watchable"
     *
     * @return Whether another car is watchable
     * 
     * @note
     * "Watchable" has different meanings, by sight or by information
     * communication, depending on the simulation scenario.
     */
    bool _canWatch(const Vehicle* other) const;

    /**
     * @~japanese
     * 交差点 @p inter への進入を他車 @p other に譲るか判断する
     *
     * @param thisDir 自身の進入方向
     * @param thatDir 相手の進入方向
     * @return 次の交差点手前で停車するか
     *
     * @~english
     * Determine whether to give way to another car @p other
     * to enter the intersection @p inter
     *
     * @param thisDir Approach direction of the subject car
     * @param thatDir Approach direction of another car
     * @return Whether to stop before the next intersection
     */
    bool _yields(
        const Intersection* inter, int thisDir, int thatDir,
        const Vehicle* other);

    ///@}

    //==================================================================
protected:
    /**
     * @~japanese 所有者
     * @~english  Owner
     */
    Vehicle* _vehicle;

    /**
     * @~japanese 交錯を厳密にチェックするかどうか
     * @~english  Whether to check for collision strictly
     */
    bool _checksCollisionStrictly;

    //==================================================================
    /**
     * @~japanese
     * @name 所有者の部分クラスオブジェクトへのポインタ
     *
     * @~english
     * @name Pointers to the part class objects of the owner
     */
    ///@{
protected:
    VehicleBehavior*           _behavior;
    const VehicleBodyProperty* _body;
    VehicleDecision*           _decision;
    const VehicleGlobalRoute*  _globalRoute;
    const VehicleLocalRoute*   _localRoute;
    const VehicleLocation*     _location;
    VehicleScene*              _scene;

#ifdef INCLUDE_PEDESTRIANS
    VehiclePedExt* _pedExt;
#endif //INCLUDE_PEDESTRIANS
    ///@}

#ifdef INCLUDE_PEDESTRIANS
    //==================================================================
    /**
     * @~japanese アクセッサ
     * @~english  Accessor
     */
    ///@{
public:
    void setPedExt(VehiclePedExt* pedExt)
    {
        _pedExt = pedExt;
    }
#endif //INCLUDE_PEDESTRIANS

    ///@}
};

#endif //__VEHICLE_SCENE_PERCEIVER_H__
