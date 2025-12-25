/* **************************************************************************
 * Copyright 2014-2024 ADVENTURE Project
 * All Rights Reserved
 * ************************************************************************** */
/**
 * @file SignalAspect.hpp
 */
#ifndef __SIGNAL_ASPECT_HPP__
#define __SIGNAL_ASPECT_HPP__
#include "SignalColor.hpp"
#include <AmuConverter.hpp>
#include <algorithm>
#include <cassert>
#include <cstdlib>
#include <iostream>
#include <sstream>
#include <vector>

//##############################################################################
/**
 * @~japanese 信号現示
 *
 * @note
 * ある瞬間に該当する交差点に設置されているすべての信号の状態をまとめて
 * 「現示 (aspect) 」とする．1スプリットに1現示が対応する．1サイクルには複数の
 * 現示が含まれる．
 * 
 * @~english  Traffic light aspect
 *
 * @note
 * At a given moment, states of all traffic lights installed at a  intersection
 * are collectively referred to as "aspect". One aspect corresponds to one 
 * split. One cycle includes multiple aspects.
 *
 * @~ @ingroup Signal
 */
struct SignalAspect
{
public:
    /**
     * @~japanese
     * ある瞬間の1つの信号の状態をあらわす構造体
     *
     * @~english
     * Struct representing a state of a traffic light at a given moment
     */
    struct State
    {
    public:
        SignalColor::MainState   main;
        SignalColor::SubState    sub;
        SignalColor::WalkerState walker;
    };

    //==========================================================================
public:
    SignalAspect() {};
    explicit SignalAspect(std::vector<State>& states)
    {
        setStates(states);
    }
    ~SignalAspect() {};

    /**
    * @~japanese コピー代入演算子
    * @~english  Copy assignment operator
    */
    SignalAspect& operator=(const SignalAspect& other)
    {
        _states.clear();
        _states.resize(other._states.size());
        for (unsigned int i = 0; i < other._states.size(); i++)
        {
            _states[i] = other._states[i];
        }
        return *this;
    }

    /**
     * @~japanese 現示情報が準備できたかどうか
     * @~english  Whether the aspect data is ready 
     */
    bool isReady() const
    {
        return _isValidSize(_states.size());
    }

    /**
     * @~japanese 有効な方向の数を返す
     * @~english  Return number of valid directions
     */
    int numDirections() const
    {
        return _states.size();
    }

    /**
     * @~japanese 現示の集合を出力する
     * @~english  Display set of aspects
     */
    void print() const;

private:
    /**
     * @~japanese @p size は有効なサイズかどうか
     * @~english  Whether @p size a valid size
     */
    bool _isValidSize(const int size) const
    {
        // 交差点の次数の下限と上限
        // Lower and upper limits of the degree of intersection
        if (size <= 1 || size > 10)
        {
            return false;
        }
        return true;
    }

    /**
     * @~japanese @p direction が信号のサイズとして有効かどうか
     * @~english  Whether @p direction is valid signal size
     */
    bool _isValidDirection(const unsigned int direction) const
    {
        if (direction > _states.size())
        {
            std::cerr << "ERROR: signal direction(" << direction
                      << ") is invalid" << std::endl;
            exit(EXIT_FAILURE);
        }
        return true;
    }

    //==========================================================================
private:
    /**
     * @~japanese
     * 該当交差点におけるある瞬間のすべての信号の状態の集合
     *
     * @~english
     * States of all traffic lights at intersection at a given moment
     */
    std::vector<State> _states;

    //==========================================================================
    /**
     * @~japanese @name アクセッサ
     * @~english  @name Accessor
     */
    ///@{
public:
    bool setStates(std::vector<State>& states);

    SignalColor::MainState mainColor(const int direction) const
    {
        assert(_isValidDirection(direction));
        return _states[direction].main;
    }

    SignalColor::SubState subColor(const int direction) const
    {
        assert(_isValidDirection(direction));
        return _states[direction].sub;
    }

    SignalColor::WalkerState walkerColor(const int direction) const
    {
        assert(_isValidDirection(direction));
        return _states[direction].walker;
    }

    /**
     * @~japanese
     * メイン，サブ，歩行者用現示の状態をまとめて返す
     *
     * @~english
     * Return states of main, sub and pedestrian lights collectively
     */
    const State& colors(const int direction) const
    {
        assert(_isValidDirection(direction));
        return _states[direction];
    }

    ///@}

    //==========================================================================
    /**
    * @~japanese 出力ストリーム挿入演算子
     * @~english  Output stream insertion operator
     */
    friend std::ostream& operator<<(std::ostream& out, const SignalAspect& a)
    {
        std::ostringstream oss;
        for (unsigned int i = 0; i < a._states.size(); i++)
        {
            oss << amu::converter::toUnderlying(a._states[i].main) << ","
                << amu::converter::toUnderlying(a._states[i].sub) << ","
                << amu::converter::toUnderlying(a._states[i].walker) << " ";
        }
        out << oss.str();
        return out;
    }
};

#endif //__SIGNAL_ASPECT_HPP
