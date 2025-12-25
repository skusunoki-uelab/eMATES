/* ******************************************************************
 * Copyright 2014-2024 ADVENTURE Project
 * All Rights Reserved
 * ****************************************************************** */
/**
 * @file Vehicle.hpp
 */
#ifndef __VEHICLE_HPP__
#define __VEHICLE_HPP__
#include "Blinker.hpp"
#include "Config.hpp"
#include "LocalLaneRouter.hpp"
#include "RandomNumberGenerator.hpp"
#include "RelativeDirection.hpp"
#include "ObjectInLane.hpp"
#include "VehicleGlobalRoute.hpp"
#include "VehicleActor.hpp"
#include "VehicleBehavior.hpp"
#include "VehicleBodyProperty.hpp"
#include "VehicleDecision.hpp"
#include "VehicleDeterminer.hpp"
#include "VehicleTypeManager.hpp"
#include "VehicleLaneChangeActor.hpp"
#include "VehicleLaneChangeDeterminer.hpp"
#include "VehicleLaneChangePerceiver.hpp"
#include "VehicleLocalRoute.hpp"
#include "VehicleLocation.hpp"
#include "VehicleScene.hpp"
#include "VehiclePerceiver.hpp"
#ifdef INCLUDE_PEDESTRIANS
#include "ped/VehiclePedExt.hpp"
#endif //INCLUDE_PEDESTRIANS
#include <string>
#include <deque>
#include <AmuVector.hpp>

class RoadMap;
class LaneBundle;
class Intersection;
class Section;
class Lane;
class VirtualLeader;

/**
 * @defgroup Vehicle
 * @ingroup Agent
 * @~japanese 自動車エージェント
 * @~english  Car agent
 */

//######################################################################
/**
 * @~japanese 自動車エージェントの基底クラス
 * @~english  Base class of car agent
 * @~ @ingroup Vehicle
 */
class Vehicle : public ObjectInLane
{
public:
    Vehicle();
    virtual ~Vehicle();

    // コピー禁止
    // No copy allowed
    Vehicle(const Vehicle&)            = delete;
    Vehicle& operator=(const Vehicle&) = delete;

public:
    //==================================================================
    /**
     * @~japanese 認知前に必要な処理を実行する
     * @~english  Perform necessary processing before recognition
     */
    virtual void preperceive();

    /**
     * @~japanese 周囲の状況を認知する
     * @~english  Recognize circumstance
     */
    virtual void perceive();

    /**
     * @~japanese 意思決定する
     * @~english  Make decision
     */
    virtual void determine();

    /**
     * @~japanese 行動を実行する
     * @~english  Perform action
     */
    virtual void act();

    /**
     * @~japanese 行動後に必要な処理を実行する
     * @~english  Perform necessary processing after action
     */
    virtual void postact();

    //==================================================================
    /**
     * @~japanese
     * 単路部 @p section の車線 @p lane 上の位置 @p distance に登場する
     *
     * @note
     * 最初の登場時のみ呼び出される．@p roadMap の登録も兼ねる．
     *
     * @~english
     * Appear at @p distance on lane @p lane of section @p section
     *
     * @note
     * Called only on first appearance. Also register @p roadMap .
     */
    virtual bool addToSection(
        RoadMap* roadMap, Section* section, const Lane* lane, double distance);

    /**
     * @~japanese 最初のローカル経路を設定する
     * @~english  Set first local route
     */
    void firstLocalReroute(
        const Section* section, const Lane* lane, double length);

    /**
     * @~japanese
     * 発生点から離れていて，結果に反映される車両であるかどうか
     *
     * @note
     * 登場時には速度が外的に与えられるため，排気排出量に影響が生じる
     * 懸念がある．したがって，発生点に近すぎる自動車は出力から除外する
     * 必要がある．結果として出力しないだけで，速度計算は通常通り行う．
     *
     * @~english
     * Whether the car far from the origin and reflected in the result
     *
     * @note
     * Since the velocity is given externally at the time of appearance,
     * there is a concern that it will be affect the amount of exhaust
     * emissions. Therefore, cars that are too close to the origin point
     * should be excluded from the output. Just do not output results,
     * and perform the velocity calculation as usual.
     */
    bool isAwayFromOriginNode() const;

    /**
     * @~japanese
     * 次の交差点に流入する際の境界番号を戻す
     *
     * @~english
     * Return the border number when entering the next intersection
     */
    int directionFrom() const;

    /**
     * @~japanese
     * 次の交差点から流出する際の境界番号を戻す
     *
     * @~english
     * Return the border number when exiting the next intersection
     */
    int directionTo() const;

    /**
     * @~japanese 経路を探索して自身の_routeに設定する
     * @return    経路を発見したかどうか
     *
     * @~english  Search a route and set it to own _route
     * @return    Whether the route found
     */
    bool reroute(const Intersection* rear, const Intersection* front);

private:
    /**
     * @~japanese 経路探索に失敗した場合に新たなゴールを設定する
     * @~english  Set a new goal if routing fails
     */
    void _setNewGoal();

public:
    /**
     * @~japanese 車両の情報を @p out に出力する
     * @~english  Output car information to @p out
     */
    void print(std::ostream& out) const;

    //==================================================================
    /**
     * @~japanese
     * @name 自動車の挙動と情報を集約する部分クラス
     *
     * @~english
     * @name Part classes aggregating car behavior and information
     */
    ///@{
protected:
    /**
     * @~japanese 車体の属性
     * @~english  Property variables for the vehicle body
     */
    std::unique_ptr<VehicleBodyProperty> _body {new VehicleBodyProperty};

    /**
     * @~japanese 自動車エージェントの局所的な挙動
     * @~english  Local behavior variables of the car agent
     */
    VehicleBehavior _behavior;

    /**
     * @~japanese 自動車エージェントの意思決定結果
     * @~english  Decision result of the car agent
     */
    VehicleDecision _decision;

    /**
     * @~japanese 自動車エージェントの時空間上の位置
     * @~english  Spatio-temporal position variables of the car agent
     */
    VehicleLocation _location;

    /**
     * @~japanese 認知した局所環境
     * @~english  Perceived local environment
     */
    VehicleScene _scene;

    /**
     * @~japanese 認知機能モジュール
     * @~english  Recognition module
     */
    VehiclePerceiver _perceiver;

    /**
     * @~japanese 車線変更に関する認知機能モジュール
     * @~english  Recognition module for lane-changing
     */
    VehicleLaneChangePerceiver _lcPerceiver;

    /**
     * @~japanese 意思決定機能モジュール
     * @~english  Decision-making module
     */
    VehicleDeterminer _determiner;

    /**
     * @~japanese 車線変更に関する意思決定機能モジュール
     * @~english  Decision-making module for lane-changing
     */
    VehicleLaneChangeDeterminer _lcDeterminer;

    /**
     * @~japanese 行動機能モジュール
     * @~english  Action module
     */
    // by abe 2025/04/22 [eMATES] ポリモーフィズムを使うためポインタ型に変更
    // もともとVehicle 1台につき VehicleActor が1つインスタンス化されていたため、
    // unique_ptr で1台ごとnewしてもメモリ消費量は変わらない。
    //VehicleActor _actor;
    std::unique_ptr<VehicleActor> _actor {new VehicleActor};

    /**
     * @~japanese 車線変更に関する行動機能モジュール
     * @~english  Action module for lane-changing
     */
    VehicleLaneChangeActor _lcActor;

    /**
     * @~japanese 大局的経路オブジェクト
     *
     * @note
     * Routerは持たず，必要なときに取得する
     *
     * @~english  Global route object
     *
     * @note
     * Not have a Router, get it when needed.
     */
    VehicleGlobalRoute _globalRoute;

    /**
     * @~japanese 車線単位の局所的経路オブジェクト
     * @~english  Lane-wise Local route object
     */
    VehicleLocalRoute _localRoute;

    /**
     * @~japanese 局所的経路探索機能モジュール
     * @~english  Local routing module
     */
    LocalLaneRouter _localRouter;

    /**
     * @~japanese ウィンカー
     * @~english  Blinker
     */
    Blinker _blinker;

    ///@}

    //==================================================================
    /**
     * @~japanese 識別番号
     * @~english  ID number
     */
    std::string _id;

    /**
     * @~japanese 乱数生成器
     * @~english  Random number generator
     */
    RandomNumberGenerator _rng;

    /**
     * @~japanese 経路探索パラメータ
     * @~english  Routing parameters
     */
    double _routingParams[VEHICLE_ROUTING_PARAMETER_SIZE];

    /**
     * @~japanese 選好するネットワークのランク
     * @~english  Preferred network rank
     */
    int _preferredNetworkRank;

    //==================================================================
    /**
     * @~japanese @name 部分クラスへのアクセッサ
     * @~english  @name Accessor for part classes
     */
    ///@{
public:
    const VehicleBodyProperty* body() const
    {
        return _body.get();
    }

    const VehicleBehavior* behavior() const
    {
        return &_behavior;
    }

    const VehicleDecision* decision() const
    {
        return &_decision;
    }

    const VehicleLocation* location() const
    {
        return &_location;
    }

    const VehicleScene* scene() const
    {
        return &_scene;
    }

    const VehiclePerceiver* perceiver() const
    {
        return &_perceiver;
    }

    const VehicleLaneChangePerceiver* laneChangePerceiver() const
    {
        return &_lcPerceiver;
    }

    const VehicleDeterminer* determiner() const
    {
        return &_determiner;
    }

    const VehicleLaneChangeDeterminer* laneChangeDeterminer() const
    {
        return &_lcDeterminer;
    }

    const VehicleActor* actor() const
    {
        return _actor.get();
    }

    const VehicleLaneChangeActor* laneChangeActor() const
    {
        return &_lcActor;
    }

    const VehicleGlobalRoute* globalRoute() const
    {
        return &_globalRoute;
    }

    const VehicleLocalRoute* localRoute() const
    {
        return &_localRoute;
    }

    const LocalLaneRouter* localRouter() const
    {
        return &_localRouter;
    }

    const Blinker* blinker() const
    {
        return &_blinker;
    }

    ///@}

    //==================================================================
    /**
     * @~japanese @name アクセッサ
     * @~english  @name Accessor
     */
    ///@{
public:
    void setId(const std::string& id)
    {
        _id = id;
    }

    void setRoadMap(RoadMap* roadMap)
    {
        _location.setRoadMap(roadMap);
    }

    RandomNumberGenerator* randomNumberGenerator()
    {
        return &_rng;
    }

    void setRoutingParams(const std::vector<double>& params)
    {
        for (unsigned int i = 0; i < VEHICLE_ROUTING_PARAMETER_SIZE; i++)
        {
            _routingParams[i] = params[i];
        }
    }

    void setPreferredNetworkRank(int networkRank)
    {
        _preferredNetworkRank = networkRank;
    }

    ///@}

    //==================================================================
    /**
     * @~japanese @name 親クラスの関数のオーバーライド
     * @~english  @name Override functions of parent class
     */
    ///@{
public:
    virtual const std::string& id() const override
    {
        return _id;
    }

    virtual double bodyWidth() const override
    {
        return _body->bodyWidth();
    }

    virtual double bodyLength() const override
    {
        return _body->bodyLength();
    }

    virtual double bodyHeight() const override
    {
        return _body->bodyHeight();
    }

    virtual double distance() const override
    {
        return _location.distance();
    }

    virtual double distanceFromInflowBorder() const override
    {
        return _location.distanceFromInflowBorder();
    }

    virtual const amu::math::AmuVector directionVector() const override
    {
        return _location.directionVector();
    }

    virtual double velocity() const override
    {
        return _behavior.velocity();
    }

    virtual double accel() const override
    {
        return _behavior.accel();
    }

    ///@}

#ifdef INCLUDE_PEDESTRIANS
    //==================================================================
    /**
     * @~japanese @name 歩行者用拡張
     * @~english  @name Extension for pedestrian
     */
    ///@{
protected:
    VehiclePedExt* _pedExt;

public:
    VehiclePedExt* pedExt()
    {
        return _pedExt;
    }

    ///@}
#endif //INCLUDE_PEDESTRIANS
};

#endif //__VEHICLE_HPP__
