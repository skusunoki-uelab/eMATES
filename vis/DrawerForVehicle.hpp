/* ******************************************************************
 * Copyright 2014-2024 ADVENTURE Project
 * All Rights Reserved
 * ****************************************************************** */
/**
 * @file DrawerForVehicle.hpp
 */
#ifndef __DRAWER_FOR_VEHICLE_HPP__
#define __DRAWER_FOR_VEHICLE_HPP__
#include "../Vehicle.hpp"

//######################################################################
/**
 * @~japanese 車両を描画する
 * @~english  Draw a vehicle
 * @~ @ingroup Drawing
 */
class DrawerForVehicle
{
public:
    DrawerForVehicle(){};
    ~DrawerForVehicle(){};

    /**
     * @~japanese 車両 @p vehicle を描画する
     * @~english  Draw @p vehicle
     */
    void draw(const Vehicle& vehicle) const;

    /**
     * @~japanese 車両 @p vehicle をサイズ @p size で簡易描画する
     * @~english  Simply draw @p vehicle with size @p size 
     */
    void drawSimple(const Vehicle& vehicle, double size) const;

private:
    /**
     * @~japanese 車体色を設定する
     * @~english  Set body color
     */
    void _setColor(const Vehicle& vehicle) const;

    /**
     * @~japanese 速度比に応じた車体色を取得する
     * @~english  Get body color according to speed ratio
     */
    void _getVelocityColor(double* result_r,
                           double* result_g,
                           double* result_b,
                           double ratio) const;
    
    /**
     * @~japanese 1両編成の車両を描画する
     * @~english  Draw a 1-car formation vehicle
     */
    void _draw1Car(const Vehicle& vehicle) const;

    /**
     * @~japanese 2両編成の車両を描画する
     * @~english  Draw a 2-car formation vehicle
     */
    void _draw2Car(const Vehicle& vehicle) const;

    /**
     * @~japanese 3両編成の車両を描画する
     * @~english  Draw a 3-car formation vehicle
     */
    void _draw3Car(const Vehicle& vehicle) const;

    /**
     * @~japanese 車両描画用の内部関数
     * @~english  Internal function for drawing vehicle
     */
    void _drawCarInside(const Vehicle& vehicle,
                        double lengthRatio,
                        amu::geometry::AmuPoint& pv,
                        amu::math::AmuVector& dvec,
                        bool showsBlinker) const;
};

#endif //__DRAWER_FOR_VEHICLE_HPP__
