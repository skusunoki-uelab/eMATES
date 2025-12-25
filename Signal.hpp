/* **************************************************************************
 * Copyright 2014-2024 ADVENTURE Project
 * All Rights Reserved
 * ************************************************************************** */
/*
 * @file Signal.hpp
 */
#ifndef __SIGNAL_HPP__
#define __SIGNAL_HPP__
#include "Config.hpp"
#include "SignalAspect.hpp"
#include "SignalColor.hpp"
#include "SignalCycle.hpp"
#include "SignalSplit.hpp"
#include "TimeManager.hpp"
#include <algorithm>
#include <cassert>
#include <iostream>
#include <memory>
#include <string>
#include <vector>

class Intersection;

/**
 * @defgroup Signal
 * @ingroup RoadEnvironment
 * @~japanese 信号機
 * @~english  Traffic light
 */

//##############################################################################
/**
 * @~japanese 信号機
 *
 * 現示パターンと現示時間を管理する．
 *
 * 灯火色は以下の3種があり， SignalColor で定義される．
 * - メイン灯火 SignalColor::MainState
 * - 矢印灯火 SignalColor::SubState
 * - 歩行者用信号 SignalColor::WalkerState
 *
 * 1スプリット・交差点全方向の現示は SignalAspect が保持し，その現時時間とともに
 * SignalSplit が管理する．信号1サイクルは SignalSplit の vector であらわされ，
 * SignalCycle が管理する．SignalCycle にはその適用開始時刻と適用終了時刻も
 * 定められており，時間帯別に各現示の時間を調整することが可能である．なお，
 * 現在のバージョンでは時間帯別に現示パターンやその順序を変更することはできず，
 * 現示時間のみが変更可能である．Signal はメンバ変数として SignalCycle の
 * vector を持つ．
 * 
 * @~english  Traffic light
 *
 * Manage traffic light aspects and their durations.
 *
 * There are three subtypes of aspects, defined by SignalColor .
 * - Main light color: SignalColor::MainState
 * - Allow light: SignalColor::SubState
 * - Pedestrian light:  SignalColor::WalkerState
 *
 * SignalAspect has the aspects of 1 split in all directions at an intersection,
 * and SignalSplit manages the aspects along with its display time. A traffic 
 * light cycle is represented by the vector of SignalSplit and managed by
 * SignalCycle, which also has the start and end times of its application, 
 * allowing the display time of each aspect to be adjusted according to the 
 * time of day. In the current version, it is not allowed to change the aspect 
 * pattern or its display order, but only the display time of each aspect. 
 * Signal has a vector of SignalCycle as a member variable.
 * 
 * @~ @ingroup Signal
 */
class Signal
{
public:
    /**
     * @~japanese 信号の進入許可の列挙
     * @~english  Enumeration of signal entry permits
     */
    enum class Permission : int
    {
        PROHIBITION,
        PERMISSION,
        CREEPING,
        PAUSING
    };

    //==========================================================================
public:
    Signal() : _id("0")
    {
        _inter = nullptr;
        _cycles.clear();
        _currentCycle   = nullptr;
        _previousCycle  = nullptr;
        _currentAspect  = nullptr;
        _previousAspect = nullptr;
    }
    explicit Signal(const Intersection* inter);
    ~Signal();

    /**
     * @~japanese 信号サイクルデータを追加する
     * @~english  Add signal cycle data
     */
    void addCycle(SignalCycle* cycle);

    /**
     * @~japanese 登録された信号サイクルデータの前後関係を設定する
     * @~english  Set the sequence of registered traffic light cycles
     */
    void makeCycleSequence();

    /**
     * @~japanese 登録されている信号パラメータを @p out に出力する
     * @~english  Output registered signal parameters to @p out
     */
    void print(std::ostream& out) const;

private:
    /**
     * @~japanese 識別番号
     * @~english  ID
     */
    const std::string _id;

    /**
     * @~japanese 設置されている交差点
     * @~english  Intersection where the signal is installed
     */
    const Intersection* _inter;

    /**
     * @~japanese 信号サイクルデータの集合
     * @~english  Sets of signal cycle data
     */
    std::vector<SignalCycle*> _cycles;

    /**
     * @~japanese 現時点で有効な現示サイクルのポインタ
     * @~english  Pointer to currently valid cycle
     */
    const SignalCycle* _currentCycle;

    /**
     * @~japanese 一つ前に有効であった現示サイクルへのポインタ
     * @~english  Pointer to previously valid cycle
     */
    const SignalCycle* _previousCycle;

    /**
     * @~japanese 現時点で有効な現示パターンへのポインタ
     * @~english  Pointer to currently valid aspect
     */
    const SignalAspect* _currentAspect;

    /**
     * @~japanese 一つ前に有効であった現示パターンへのポインタ
     * @~english  Pointer to previously valid aspect
     */
    const SignalAspect* _previousAspect;

    //==========================================================================
    /**
     * @~japanese @name 信号現示を戻す関数群
     * @~english  @name Functions returning signal aspect
     */
    ///@{
public:
    /**
     * @~japanese 現在のメイン灯火の状態を戻す
     * @~english  Return current state of main light
     */
    SignalColor::MainState mainColor(const int direction) const
    {
        return _currentAspect->mainColor(direction);
    }

    /**
     * @~japanese 現在の矢印灯火の状態を返す
     * @~english  Return current state of allow light
     */
    SignalColor::SubState subColor(const int direction) const
    {
        return _currentAspect->subColor(direction);
    }

    /**
     * @~japanese 現在の歩行者用信号の状態を返す
     * @~english  Return current state of light for pedestrian
     */
    SignalColor::WalkerState walkerColor(const int direction) const
    {
        return _currentAspect->walkerColor(direction);
    }

    /**
     * @~japanese 一つ前のメイン灯火の状態をを返す
     * @~english  Return previous state of main light
     */
    SignalColor::MainState prevMainColor(const int direction) const
    {
        return _previousAspect->mainColor(direction);
    }

    /**
     * @~japanese 一つ前の矢印灯火の状態を返す
     * @~english  Return previous state of arrow light
     */
    SignalColor::SubState prevSubColor(const int direction) const
    {
        return _previousAspect->subColor(direction);
    }

    /**
     * @~japanese 一つ前の歩行者用信号の状態を返す
     * @~english  Return previous state of light for pedestrian
     */
    SignalColor::WalkerState prevWalkerColor(const int direction) const
    {
        return _previousAspect->walkerColor(direction);
    }

    ///@}

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

    const Intersection* intersection() const
    {
        return _inter;
    }

    std::vector<SignalCycle*>& cycles()
    {
        return _cycles;
    }

    SignalCycle* cycle(unsigned int n)
    {
        assert(n < _cycles.size());
        return _cycles[0];
    }

    void setCurrentCycle(const SignalCycle* cycle)
    {
        _previousCycle = (_currentCycle ? _currentCycle : cycle);
        _currentCycle  = cycle;
    }

    const SignalAspect* currentAspect() const
    {
        return _currentAspect;
    }

    const SignalAspect* previousAspect() const
    {
        return _previousAspect;
    }

    void setCurrentAspect(const SignalAspect* aspect)
    {
        _previousAspect = (_currentAspect ? _currentAspect : aspect);
        _currentAspect  = aspect;
    }

    ///@}
};

#endif //__SIGNAL_HPP__
