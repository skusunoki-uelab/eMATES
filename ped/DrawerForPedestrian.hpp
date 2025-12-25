/* ******************************************************************
 * Copyright 2014-2024 ADVENTURE Project
 * All Rights Reserved
 * ****************************************************************** */
/**
 * @file DrawerForPedestrian.hpp
 */
#ifdef INCLUDE_PEDESTRIANS
#ifndef __DRAWER_FOR_PEDESTRIAN_HPP__
#define __DRAWER_FOR_PEDESTRIAN_HPP__

class Pedestrian;

//######################################################################
/**
 * @~japanese 歩行者を描画する
 * @~english  Draw a pedestrian
 * @~ @ingroup Drawing PedSim
 */
class DrawerForPedestrian
{
public:
    DrawerForPedestrian() {};
    ~DrawerForPedestrian() {};

    /**
     * @~japanese 歩行者 @p ped を描画する
     * @~english  Draw pedestrian @p ped
     */
    void draw(const Pedestrian& ped) const;

    // void drawSimple(const Pedestrian& pds, double size) const;
};

#endif //__DRAWER_FOR_PEDESTRIAN_HPP__
#endif //INCLUDE_PEDESTRIANS
