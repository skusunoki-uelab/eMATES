/* **************************************************************************
 * Copyright 2014-2024 ADVENTURE Project
 * All Rights Reserved
 * ************************************************************************** */
/**
 * @fle SignalSplit.hpp
 */
#ifndef __SIGNAL_SPLIT_HPP__
#define __SIGNAL_SPLIT_HPP__
#include "Config.hpp"
#include "ScheduleItemBase.hpp"
#include "SignalAspect.hpp"
#include <iostream>
#include <vector>

class Signal;
class SignalCycle;

//##############################################################################
/**
 * @~japanese 信号スプリット
 * 
 * 信号現示 SignalAspect と現示時間を保持する．次の SignalSplit へのポインタを
 * 持ち，切り替えのタイミングは ScheduleManager で管理する．
 * 
 * @~english  Traffic light split
 * 
 * It holds SignalAspect and its display time. It has a pointer to the next
 * SignalSplit, and ScheduleManager manages the switching timing.
 *
 * @~ @ingroup Signal
 */
class SignalSplit : public ScheduleItemBase
{
public:
    SignalSplit(
        const Signal* signal, const SignalCycle* cycle, unsigned int id,
        ulint duration);
    virtual ~SignalSplit() {}

    /**
     * @~japanese 親クラスの関数のオーバーライド
     * @~english  Override functions of parent class
     */
    void activate() override;

    /**
     * @~japanese 登録されたパラメータを @p out に出力する
     * @~english  Output registered parameters to @p out
     */
    void print(std::ostream& out) const override;

    //==========================================================================
private:
    /**
     * @~japanese このスプリットが適用される信号機
     * @~english  Traffic light to which this split is applied
     */
    const Signal* _signal;

    /**
     * @~japanese このスプリットを含む信号サイクル
     * @~english  Traffic light cycle containing this split
     */
    const SignalCycle* _cycle;

    /**
     * @~japanese スプリットの識別番号
     * 
     * 情報出力用に用いる．
     * 
     * @~english  Split ID number
     * 
     * Used for information output.
     */
    const unsigned int _splitId;

    /**
     * @~japanese 現示時間 [ms]
     * @~english  Display time [ms]
     */
    const ulint _duration;

    /**
     * @~japanese 信号現示
     * @~english  Traffic light aspect
     */
    SignalAspect _aspect;

    /**
     * @~japanese 次のスプリットへのポインタ
     * @~english  Pointer to the next split
     */
    SignalSplit* _nextSplit;

    //==========================================================================
    /**
     * @~japanese @name アクセッサ
     * @~english  @name Accessor
     */
    ///@{
public:
    ulint duration() const
    {
        return _duration;
    }

    const SignalAspect& aspect() const
    {
        return _aspect;
    }

    void setAspectStates(std::vector<SignalAspect::State>& states)
    {
        _aspect.setStates(states);
    }

    const SignalSplit* nextSplit() const
    {
        return _nextSplit;
    }

    void setNextSplit(SignalSplit* next)
    {
        _nextSplit = next;
    }

    ///@}
};

#endif //__SIGNAL_SPLIT_HPP__
