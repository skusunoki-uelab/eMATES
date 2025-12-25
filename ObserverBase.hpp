/* ******************************************************************
 * Copyright 2014-2024 ADVENTURE Project
 * All Rights Reserved
 * ****************************************************************** */
/**
 * @file ObserverBase.hpp
 */
#ifndef __OBSERVER_BASE_HPP__
#define __OBSERVER_BASE_HPP__
#include <string>

/**
 * @defgroup Monitoring
 * @~japanese シミュレーションの計測
 * @~english  Simulation monitoring
 */

//######################################################################
/**
 * @~japanese 観測器の純粋抽象クラス
 *
 * 交通量を観測するための TrafficCounter などが継承する．
 *
 * @~english  Abstract class for observation device
 *
 * Inherited by TrafficCounter for observing traffic volume etc.
 *
 * @~ @ingroup Monitoring
 */
class ObserverBase
{
public:
    ObserverBase() {};
    virtual ~ObserverBase() {};

    /**
     * @~japanese 機器の識別番号を戻す
     * @~english  Return the device ID number 
     */
    virtual const std::string& id() const = 0;
};

#endif //__OBSERVER_BASE_HPP__
