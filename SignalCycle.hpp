/* **************************************************************************
 * Copyright 2014-2024 ADVENTURE Project
 * All Rights Reserved
 * ************************************************************************** */
/**
 * @fle SignalCycle.hpp
 */
#ifndef __SIGNAL_CYCLE_HPP__
#define __SIGNAL_CYCLE_HPP__
#include "Config.hpp"
#include "ScheduleItemBase.hpp"
#include "SignalSplit.hpp"
#include <cassert>
#include <iostream>
#include <vector>

class Signal;

//##############################################################################
/**
 * @~japanese 信号サイクル
 * 
 * 適用開始時刻，適用終了時刻と，サイクルを構成する SignalSplit の集合を持つ．
 * 次の SignalCycle へのポインタを持ち，切り替えのタイミングは ScheduleManager
 * が管理する．
 * 
 * @~english  Signal cycle
 * 
 * It has the start and end times of its application, and a set of SignalSplits
 * that compose a cycle. It also has a pointer to the next SignalCycle, and 
 * ScheduleManager manages the switching timing.
 * 
 * @~ @ingroup Signal
 */
class SignalCycle : public ScheduleItemBase
{
public:
    SignalCycle(const Signal* signal, unsigned int id, ulint begin, ulint end);
    ~SignalCycle();

    /**
     * @~japanese 親クラスの関数のオーバーライド
     * @~english  Override functions of parent class
     */
    void activate() override;

    /**
     * @~japanese 信号スプリットを追加する
     * @~english  Add traffic light split
     */
    void addSplit(SignalSplit* split)
    {
        _splits.emplace_back(split);
    }

    /**
     * @~japanese @p i 番目の信号スプリットを戻す
     * @~english  Return the @p i -th split
     */
    SignalSplit* split(unsigned int i)
    {
        assert(i < _splits.size());
        return _splits[i];
    }

    /**
     * @~japanese 登録された信号スプリットのループを設定する
     * @~english  Set the loop of registered traffic light splits
     */
    void makeSplitLoop();

    /**
     * @~japanese 登録されている信号パラメータを @p out に出力する
     * @~english  Output registered signal parameters to @p out
     */
    void print(std::ostream& out) const override;

    //==========================================================================
private:
    /**
     * @~japanese このスサイクルが適用される信号機
     * @~english  Traffic light to which this cycle is applied
     */
    const Signal* _signal;

    /**
     * @~japanese サイクルの識別番号
     * 
     * 情報出力用に用いる．
     * 
     * @~english  Cycle ID number
     * 
     * Used for information output.
     */
    unsigned int _cycleId;

    /**
     * @~japanese 適用開始時刻
     * @~english  Start time of application [ms]
     */
    const ulint _beginTime;

    /**
     * @~japanese 適用終了時刻
     * @~english  End time of application [ms]
     */
    const ulint _endTime;

    /**
     * @~japanese サイクル長 [ms]
     * @~english  Cycle length [ms]
     */
    ulint _cycleLength;

    /**
     * @~japanese 信号スプリットの集合
     * @~english  Set of traffic light splits
     */
    std::vector<SignalSplit*> _splits;

    /**
     * @~japanese 次のサイクルへのポインタ
     * @~english  Pointer to the next cycle
     */
    SignalCycle* _nextCycle;

    //==========================================================================
    /**
     * @~japanese @name アクセッサ
     * @~english  @name Accessor
     */
    ///@{
public:
    unsigned int id() const
    {
        return _cycleId;
    }

    ulint beginTime() const
    {
        return _beginTime;
    }

    ulint endTime() const
    {
        return _endTime;
    }

    ulint length() const
    {
        return _cycleLength;
    }

    void setNextCycle(SignalCycle* cycle)
    {
        _nextCycle = cycle;
    }

    ///@}
};

#endif //__SIGNAL_CYCLE_HPP__
