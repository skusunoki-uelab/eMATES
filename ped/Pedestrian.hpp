/* ******************************************************************
 * Copyright 2014-2024 ADVENTURE Project
 * All Rights Reserved
 * ****************************************************************** */
/**
 * @file Pedestrian.hpp
 */
#ifdef INCLUDE_PEDESTRIANS
#ifndef __PEDESTRIAN_HPP__
#define __PEDESTRIAN_HPP__
#include "PedestrianActor.hpp"
#include "PedestrianBehavior.hpp"
#include "PedestrianBodyProperty.hpp"
#include "PedestrianDecision.hpp"
#include "PedestrianDeterminer.hpp"
#include "PedestrianLocation.hpp"
#include "PedestrianPerceiver.hpp"
#include "PedestrianScene.hpp"
#include "../RandomNumberGenerator.hpp"
#include <AmuPoint.hpp>
#include <AmuVector.hpp>
#include <string>

//#define PDS_DEBUG

/*
 * 歩行者の自動車認知とそれに基づく判断を有効化する場合は以下のマクロを
 * 有効にする．ただし判断のための関数 (changeVelocityFor()) を実装する
 * こと．
 *
 * Enable macros below to enable pedestrians' vehicle recognition and
 * decisions based on it. However, implement a function for judgment
 * (changeVelocityFor()).
 */
//#define ENABLE_PEDESTRIAN_SEARCH_VEHICLE

class Intersection;
class Zebra;

/**
 * @defgroup Pedestrian
 * @~ingroup PedSim Agent
 * @~japanese 歩行者エージェント
 * @~english  Pedestrian agent
 */

//######################################################################
/**
 * @~japanese 歩行者エージェント
 * @~english  Pedestrian agent
 * @~ @ingroup Pedestrian
 */
class Pedestrian
{
public:
    Pedestrian();
    ~Pedestrian();

    //==================================================================
public:
    /**
     * @~japanese 周囲の状況を認知する
     * @~english  Recognize circumstance
     */
    void perceive();

    /**
     * @~japanese 意思決定する
     * @~english  Make decision
     */
    void determine();

    /**
     * @~japanese 行動を実行する
     * @~english  Perform action
     */
    void act();

    //==================================================================
public:
    /**
     * @~japanese 横断歩道上に登場する
     *
     * @param inter 横断歩道の設置された交差点
     * @param zebra 横断歩道
     * @param dir 横断方向
     * @param positioin 初期位置
     * @param velocity  初期速度
     *
     * @note
     * 最初の登場時のみ呼び出される．
     *
     * @~english  Appear on a crosswalk
     *
     * @param inter Intersection to which the crosswalk is installed
     * @param zebra Crosswalk
     * @param dir Crossing direction
     * @param positioin Initial position
     * @param velocity Initial velocity
     *
     * @note
     * Called only on first appearance. 
     */
    bool addToZebra(
        Intersection* inter, Zebra* zebra, int dir,
        amu::geometry::AmuPoint position,
        amu::math::AmuVector    desiredDirection);

    /**
     * @~japanese 位置ベクトルを戻す
     * @~english  Return position vector
     */
    const amu::geometry::AmuPoint& position() const
    {
        return _location.position();
    }

    /**
     * @~japanese 速度ベクトルを戻す
     * @~english  Return velocity vector
     */
    const amu::math::AmuVector& velocity() const
    {
        return _behavior.velocity();
    }

    /**
     * @~japanese 歩行者の情報を @p out に出力する
     * @~english  Display pedestrian information to @p out
     */
    void print(std::ostream& out) const;

    //==================================================================
    /**
     * @~japanese
     * @name
     * 歩行者の挙動と情報を集約する部分クラス
     *
     * @~english
     * @name
     * Part classes aggregating pedestrian behavior and information
     */
    ///@{
private:
    /**
     * @~japanese 歩行者の身体属性
     * @~english  Property variables for the pedestrian body
     */
    PedestrianBodyProperty _body;

    /**
     * @~japanese 歩行者エージェントの局所的な挙動
     * @~english  Local behavior variables of the pedestrian agent
     */
    PedestrianBehavior _behavior;

    /**
     * @~japanese 歩行者エージェントの意思決定結果
     * @~english  Decision result of the pedestrian agent
     */
    PedestrianDecision _decision;

    /**
     * @~japanese
     * 歩行者エージェントの時空間上の位置
     *
     * @~english
     *  Spatio-temporal position variables of the pedestrian agent
     */
    PedestrianLocation _location;

    /**
     * @~japanese 認知した局所環境
     * @~english  Perceived local environment
     */
    PedestrianScene _scene;

    /**
     * @~japanese 認知機能モジュール
     * @~english  Recognition module
     */
    PedestrianPerceiver _perceiver;

    /**
     * @~japanese 意思決定機能モジュール
     * @~english  Decision-making module
     */
    PedestrianDeterminer _determiner;

    /**
     * @~japanese 行動機能モジュール
     * @~english  Action module
     */
    PedestrianActor _actor;

    ///@}

    //==================================================================
private:
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

    //==================================================================
    /**
     * @~japanese @name 部分クラスへのアクセッサ
     * @~english  @name Accessor for part classes
     */
    ///@{
public:
    const PedestrianBodyProperty* body() const
    {
        return &_body;
    }

    const PedestrianBehavior* behavior() const
    {
        return &_behavior;
    }

    const PedestrianDecision* decision() const
    {
        return &_decision;
    }

    const PedestrianLocation* location() const
    {
        return &_location;
    }

    const PedestrianScene* scene() const
    {
        return &_scene;
    }

    const PedestrianPerceiver* perceiver() const
    {
        return &_perceiver;
    }

    const PedestrianDeterminer* determiner() const
    {
        return &_determiner;
    }

    const PedestrianActor* actor() const
    {
        return &_actor;
    }

    ///@}

    //==================================================================
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

    RandomNumberGenerator* randomNumberGenerator()
    {
        return &_rng;
    }

    ///@}
};

#endif //__PEDESTRIAN_HPP__
#endif //INCLUDE_PEDESTRIANS
