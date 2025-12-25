/* ******************************************************************
 * Copyright 2014-2024 ADVENTURE Project
 * All Rights Reserved
 * ****************************************************************** */
/**
 * @file PedestrianBehavior.cpp
 */
#ifdef INCLUDE_PEDESTRIANS
#include "PedestrianBehavior.hpp"
#include "Pedestrian.hpp"
#include "../AppMates.hpp"
#include "../RandomNumberGenerator.hpp"
#include "../TimeManager.hpp"
#include <cassert>

using namespace std;

//======================================================================
void PedestrianBehavior::setPedestrian(Pedestrian* ped)
{
    _pedestrian = ped;

    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    /*
     * 歩行者の乱数生成器を使用するため，コンストラクタでなくこの関数で
     * 通常時の最高歩行速さを設定する
     *
     * Set the maximum normal walking speed in this function instead of
     * the constructor, to use the pedestrian's random number generator.
     */

    // 標準歩行速さ
    // standard walking speed
    _maxSpeed = 1.3 / 1000.0; // 1.3 [m/s] = 4.68 [km/h]

    // [0.8, 1.2] の乱数
    // Random number in [0.8, 1.2]
    double r = 0.8 + 0.4 * ped->randomNumberGenerator()->uniform();

    _maxSpeed *= r;
}

//======================================================================
void PedestrianBehavior::print(ostream& out) const
{
    stringstream ss;

    ss << "DesiredDirection: " << _desiredDirection
       << ", Velocity: " << _velocity << endl;
    ss << "MaxSpeed: " << _maxSpeed << ", AccelFactor: " << _accelFactor
       << endl;

#pragma omp critical(out_critical)
    out << ss.str();
}

#endif //INCLUDE_PEDESTRIANS
