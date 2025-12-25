/* ******************************************************************
 * Copyright 2014-2024 ADVENTURE Project
 * All Rights Reserved
 * ****************************************************************** */
/**
 * @file SignalTimingTable.hpp
 */
#ifndef __SIGNAL_TIMING_TABLE_HPP__
#define __SIGNAL_TIMING_TABLE_HPP__
#include "SignalTiming.hpp"
#include <algorithm>
#include <functional>
#include <iterator>
#include <vector>

//######################################################################
/**
 * @~japanese SignalTimingの集合
 *
 * @note
 * 特定の時間区分に適用されるSignalTimingを集約する
 *
 * @todo
 * 要素の扱いをGenerateTableと同じにするかどうか検討する．
 *
 * @~english  Set of Signal Timing
 *
 * @note
 * Collect SignalTiming applied to specific time intervals
 *
 * @~
 * @ingroup Signal
 * @see SignalTiming
 */

class SignalTimingTable
{
public:
    SignalTimingTable(){};
    SignalTimingTable(std::vector<SignalTiming>& timings)
    {
        assert(timings.size() > 0);
        _timings.resize(timings.size());
        copy(timings.begin(), timings.end(), _timings.begin());
        _index = _timings.begin();
    }
    ~SignalTimingTable(){};

    /**
     * @~japanese 代入演算子
     * @~english  Assignment operator
     */
    SignalTimingTable& operator=(const SignalTimingTable& table)
    {
        if (this == &table)
        {
            return *this;
        }
        assert(table._timings.size() > 0);
        _timings.resize(table._timings.size());
        copy(table._timings.begin(), table._timings.end(),
             _timings.begin());
        _index = _timings.begin();
        return *this;
    }

    /**
     * @~japanese 時刻 @p time で有効な現示を返す
     * @~english  Return valid SignalTiming at time @p time
     */
    const SignalTiming& validTiming(ulint time) const;

    /**
     * @~japanese 保持しているSignalTimingのサイズを返す
     * @~english  Return size of stored SignalTimings
     */
    unsigned int columnSize() const
    {
        return _timings.size();
    }

protected:
    /**
     * @~japanese SignalTimingの集合
     *
     * @todo
     * AmuIntervalをキーとするmap or AmuPriorityQueueにすべき？
     *
     * @~english  Set of SignalTimings
     */
    std::vector<SignalTiming> _timings;

    /**
     * @~japanese 現在のカーソル位置を保持する
     *
     * @todo
     * Signalの要請によって指す位置が改変されてしまう(const性が維持
     * できない)iteratorなのと時間がないのでmutableとしてしまうが，
     * いずれ改善すべき
     *
     * @~english  Keep current cursor position
     */
    mutable std::vector<SignalTiming>::const_iterator _index;
};

//######################################################################
/**
 * @~japanese 比較用関数オブジェクト
 *
 * データのリストの中から end() を見て，最初に reference よりも大きく
 * なった要素の反復子を戻す機能を実装するために作られた
 *
 * @~english  Function object for comparison
 *
 * Created to implement a function that check end() in the data list
 * and returns an iterator for the first element larger than reference.
 */
struct SignalTimingTableCmp
{
    bool operator() (const SignalTiming& data, const ulint reference) const
    {
        return (bool) (!(data.end() > reference));
    }
};

#endif //__SIGNAL_CYCLE_SEQUENCE_HPP__
