/* ******************************************************************
 * Copyright 2014-2024 ADVENTURE Project
 * All Rights Reserved
 * ****************************************************************** */
/**
 * @file TimeManager.hpp
 */
#ifndef __TIME_MANAGER_HPP__
#define __TIME_MANAGER_HPP__
#include "Config.hpp"
#include "ManagerBase.hpp"
#include <typeinfo>

/**
 * @defgroup  Time
 * @~japanese シミュレーション中で時刻を扱う
 * @~english  Handling time for simulation
 */

//######################################################################
/**
 * @~japanese 時刻と時間刻み幅を管理する
 *
 * 時間の単位は [ms]
 *
 * @~english  Manage time and time step size
 *
 * The time unit is [ms].
 *
 * @~ @ingroup Manager Time
 */
class TimeManager : public ManagerBase
{
    friend class ManagerPool;

private:
    TimeManager()
    {
        _time      = 0;
        _unit      = 100;
        _step      = 0;
        _className = typeid(this).name();
    }
    ~TimeManager() {}

    /**
     * @~japanese 親クラスの関数のオーバーライド
     * @~english  Override functions of parent class
     */
    void _finalizeInside() override {}

    //==================================================================
private:
    /**
     * @~japanese 現在時刻 [ms]
     * @~english  Current time [ms]
     */
    ulint _time;

    /**
     * @~japanese
     * 時間刻み幅 [ms]
     *
     * @note
     * 標準では \f$ \Delta t = 100 \mathrm{ms} \f$ .
     *
     * @~english  
     * Time step size
     *
     * @note
     * \f$ \Delta t = 100 \mathrm{ms} \f$ by default.
     */
    ulint _unit;

    /**
     * @~japanese 現在のステップ数
     * @~english  Current step
     */
    ulint _step;

public:
    /**
     * @~japanese 時間を1ステップ前進させる
     * @~english  Advance time by one step
     */
    void increment()
    {
        _step++;
        _time += _unit;
    }

    //==================================================================
    /**
     * @~japanese @name アクセッサ
     * @~english  @name Accessor
     */
    ///@{
    ulint time()
    {
        return _time;
    }
    void setTime(ulint presentTime)
    {
        _time = presentTime;
    }
    ulint unit()
    {
        return _unit;
    }
    void setUnit(ulint unit)
    {
        _unit = unit;
    }
    ulint step()
    {
        return _step;
    }
    void setStep(ulint step)
    {
        _step = step;
    }
    ///@}
};

#endif //__TIME_MANAGER_HPP__
