/* ******************************************************************
 * Copyright 2014-2024 ADVENTURE Project
 * All Rights Reserved
 * ****************************************************************** */
/**
 * @file VehicleTypeProperty.hpp
 */
#ifndef __VEHICLE_TYPE_PROPERTY_HPP__
#define __VEHICLE_TYPE_PROPERTY_HPP__
#include "VehicleType.hpp"
#include <iostream>

//######################################################################
/**
 * @~japanese 車種ごとの属性を集約する構造体
 * @~english  Struct aggregating properties for each vehicle type
 * @~ @ingroup Vehicle
 */
struct VehicleTypeProperty
{
protected:
    /**
     * @~japanese 車種
     * @~english  Vehicle type
     */
    const VehicleType _type;

    /**
     * @~japanese 車長 [m]
     * @~english  Body length [m]
     */
    const double _bodyLength;

    /**
     * @~japanese 車幅 [m]
     * @~english  Body width [m]
     */
    const double _bodyWidth;

    /**
     * @~japanese 車高 [m]
     * @~english  Body height [m]
     */
    const double _bodyHeight;

    /**
     * @~japanese 車重 [kg] (未使用)
     * @~english  Vehicle weight [kg] (not used)
     */
    const double _bodyWeight;

    /**
     * @~japanese 車体の連接数
     *
     * @note
     * 路面電車や連結バスに用いる．通常は1に設定．
     *
     * @~english  Number of cars articulated
     *
     * @note
     * Used for trams and articulated buses. Usually Set to 1.
     */
    const int _numCars;

    /**
     * @~japanese 最大加速度 [m/(sec^2)]
     * @~english  Maximum acceleration [m/(sec^2)]
     */
    const double _maxAcceleration;

    /**
     * @~japanese 最大減速度 (<0) [m/(sec^2)]
     * @~english  Maximum deceleration (<0) [m/(sec^2)]
     */
    const double _maxDeceleration;

    /**
     * @~japanese 描画色の赤成分 [0, 1]
     * @~english  Red value of the color for drawing [0, 1]
     */
    const double _bodyColorR;

    /**
     * @~japanese 描画色の緑成分 [0, 1]
     * @~english  Green value of the color for drawing [0, 1]
     */
    const double _bodyColorG;

    /**
     * @~japanese 描画色の青成分 [0, 1]
     * @~english  Blue value of the color for drawing [0, 1]
     */
    const double _bodyColorB;

public:
    VehicleTypeProperty()
        : _bodyLength(4.5),
          _bodyWidth(1.8),
          _bodyHeight(1.4),
          _bodyWeight(1000.0),
          _numCars(1),
          _maxAcceleration(3.0),
          _maxDeceleration(-5.0),
          _bodyColorR(1.0),
          _bodyColorG(1.0),
          _bodyColorB(1.0) {};
    VehicleTypeProperty(
        VehicleType type, double bodyLength, double bodyWidth,
        double bodyHeight, double bodyWeight, int numCars,
        double maxAcceleration, double maxDeceleration,
        double bodyColorR, double bodyColorG, double bodyColorB)
        : _type(type),
          _bodyLength(bodyLength),
          _bodyWidth(bodyWidth),
          _bodyHeight(bodyHeight),
          _bodyWeight(bodyWeight),
          _numCars(numCars),
          _maxAcceleration(maxAcceleration),
          _maxDeceleration(maxDeceleration),
          _bodyColorR(bodyColorR),
          _bodyColorG(bodyColorG),
          _bodyColorB(bodyColorB) {};
    ~VehicleTypeProperty() {};

    /**
     * @~japanese 車体サイズをまとめてポインタとして取得する
     * @~english  Get body size as pointers
     */
    void getSize(
        double* result_l, double* result_w, double* result_h) const
    {
        *result_l = _bodyLength;
        *result_w = _bodyWidth;
        *result_h = _bodyHeight;
    }

    /**
     * @~japanese
     * 加減速性能をまとめてポインタとして取得する
     *
     * @~english
     * Get acceleration/deceleration performance as pointers
     */
    void getPerformance(double* result_a, double* result_d) const
    {
        *result_a = _maxAcceleration;
        *result_d = _maxDeceleration;
    }

    /**
     * @~japanese 描画色の色成分をまとめてポインタとして取得する
     * @~english  Get color values for drawing as pointers
     */
    void getBodyColor(
        double* result_r, double* result_g, double* result_b) const
    {
        *result_r = _bodyColorR;
        *result_g = _bodyColorG;
        *result_b = _bodyColorB;
    }

    //==================================================================
    /**
     * @~japanese アクセッサ
     * @~english  Accessor
     */
    ///@
public:
    const VehicleType* type() const
    {
        return &_type;
    }

    double bodyLength() const
    {
        return _bodyLength;
    }

    double bodyWidth() const
    {
        return _bodyWidth;
    }

    double bodyHeight() const
    {
        return _bodyHeight;
    }

    int numCars() const
    {
        return _numCars;
    }

    double maxAcceleration() const
    {
        return _maxAcceleration;
    }

    double maxDeceleration() const
    {
        return _maxDeceleration;
    }

    ///@}
};

#endif //__VEHICLE_TYPE_PROPERTY_HPP__
