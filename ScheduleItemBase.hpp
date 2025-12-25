/* **************************************************************************
 * Copyright 2014-2024 ADVENTURE Project
 * All Rights Reserved
 * ************************************************************************** */
/**
 * @file ScheduleItemBase.hpp
 */
#ifndef __SCHEDULE_ITEM_BASE_HPP__
#define __SCHEDULE_ITEM_BASE_HPP__
#include <iostream>
#include <string>

//##############################################################################
/**
 * @~japanese 時間ドリブンのイベントを記述するための規定クラス
 * 
 * ScheduleManager が持つイベントリストの要素となる．
 * 
 * @~english  Base class for describing time-driven events
 * 
 * It is an element of the event list held by ScheduleManager.
 * 
 * @~ @see ScheduleManager
 */
class ScheduleItemBase
{
public:
    ScheduleItemBase() : _className("SheduleItemBase") {}
    virtual ~ScheduleItemBase() {}

protected:
    /**
     * @~japanese クラス名
     * 
     * 表示に用いる．
     * 
     * @~english  Class name
     * 
     * Used for display.
     */
    std::string _className;

    /**
     * @~japanese 処理内容
     * 
     * 表示に用いる．
     * 
     * @~english  Processing details
     * 
     * Used for display.
     */
    std::string _contents;

public:
    /**
     * @~japanese 登録された処理を有効化する
     * @~english  Activate registered procedure
     */
    virtual void activate() = 0;

    /**
     * @~japanese 処理内容を @p out に出力する
     * @~english  Output processing details to @p out
     */
    virtual void print(std::ostream& out) const;

    /**
     * @~japanese クラス名を返す
     * @~english  Return class name
     */
    std::string className() const
    {
        return _className;
    }
};

#endif //__SCHEDULE_ITEM_BASE_HPP__