/* ******************************************************************
 * Copyright 2014-2024 ADVENTURE Project
 * All Rights Reserved
 * ****************************************************************** */
/**
 * @file DrawerForMonitor.hpp
 */
#ifndef __DRAWER_FOR_MONITOR_HPP__
#define __DRAWER_FOR_MONITOR_HPP__
#include "../TrafficCounter.hpp"

//######################################################################
/**
 * @~japanese 観測器を描画する
 * @~english  Draw a monitoring device
 * @~ @ingroup Drawing
 */
class DrawerForMonitor
{
public:
    DrawerForMonitor() {};
    ~DrawerForMonitor() {};

    /**
     * @~japanese 車両感知器 @p counter を描画する
     * @~english  Draw traffic counter @p counter
     */
    void drawTrafficCounter(const TrafficCounter& counter) const;
};

#endif //__DRAWER_FOR_MONITOR_HPP__
