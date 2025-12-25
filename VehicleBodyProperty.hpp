/* ******************************************************************
 * Copyright 2014-2024 ADVENTURE Project
 * All Rights Reserved
 * ****************************************************************** */
/**
 * @file VehicleBodyProperty.hpp
 */
#ifndef __VEHICLE_BODY_PROPERTY_HPP__
#define __VEHICLE_BODY_PROPERTY_HPP__
#include "VehicleTypeManager.hpp"
#include <iostream>

class Vehicle;

//######################################################################
/**
 * @~japanese 車体の属性を集約する構造体
 * @~english  Struct aggregating properties for a vehicle body
 * @~ @ingroup Vehicle
 */
struct VehicleBodyProperty
{
public:
    VehicleBodyProperty() : _type(VehicleType())
    {
        _vehicle = nullptr;

        _bodyLength = 4.400;
        _bodyWidth  = 1.830;
        _bodyHeight = 1.315;

        _numCars = 1;

        _bodyColorR = 1.0;
        _bodyColorG = 0.0;
        _bodyColorB = 0.0;

        _maxAcceleration = 3.0;
        _maxDeceleration = -5.0;
        _jamDistance     = 2.0;
    }

    ~VehicleBodyProperty() {};

    /**
     * @~japanese 車体サイズをまとめて設定する
     * @~english  Set body size collectively
     */
    void setBodySize(double l, double w, double h)
    {
        _bodyLength = l;
        _bodyWidth  = w;
        _bodyHeight = h;
    }

    /**
     * @~japanese 描画色をまとめて設定する
     * @~english  Set color for drawing collectively
     */
    void setBodyColor(double r, double g, double b)
    {
        _bodyColorR = r;
        _bodyColorG = g;
        _bodyColorB = b;
    }

private:
    /**
     * @~japanese 持ち主
     * @~english  Owner
     */
    const Vehicle* _vehicle;

    /**
     * @~japanese 車種
     * @~english  Vehicle type
     */
    VehicleType _type;

    /**
     * @~japanese 車長 [m]
     * @~english  Body length [m]
     */
    double _bodyLength;

    /**
     * @~japanese 車幅 [m]
     * @~english  Body width [m]
     */
    double _bodyWidth;

    /**
     * @~japanese 車高 [m]
     * @~english  Body height [m]
     */
    double _bodyHeight;

    /**
     * @~japanese 車体の連接数
     * @~english  Number of cars articulated
     */
    int _numCars;

    /**
     * @~japanese 最大加速度 [m/(ms^2)]
     * @~english  Maximum acceleration [m/(ms^2)]
     */
    double _maxAcceleration;

    /**
     * @~japanese 最大減速度 [m/(ms^2)]
     * @~english  Maximum deceleration [m/(ms^2)]
     */
    double _maxDeceleration;

    /**
     * @~japanese 停車時の先行車との車間距離 [m]
     * @~english  Gap from preceding car when stopped [m]
     */
    double _jamDistance;

    /**
     * @~japanese 描画の赤成分 [0, 1]
     * @~english  Red value of the color for drawing [0, 1]
     */
    double _bodyColorR;

    /**
     * @~japanese 描画色の緑色成分 [0, 1]
     * @~english  Green value the color for drawing [0, 1]
     */
    double _bodyColorG;

    /**
     * @~japanese 描画色の青色成分 [0, 1]
     * @~english  Blue value of the color for drawing [0, 1]
     */
    double _bodyColorB;

    //==================================================================
    /**
     * @~japanese @name アクセッサ
     * @~english  @name Accessor
     */
    ///@{
public:
    void setVehicle(const Vehicle* vehicle)
    {
        _vehicle = vehicle;
    }

    const VehicleType* type() const
    {
        return &_type;
    }

    void setType(VehicleType type)
    {
        _type = type;
    }

    double bodyLength() const
    {
        return _bodyLength;
    }

    void setBodyLength(double bodyLength)
    {
        _bodyLength = bodyLength;
    }

    double bodyWidth() const
    {
        return _bodyWidth;
    }

    void setBodyWidth(double bodyWidth)
    {
        _bodyWidth = bodyWidth;
    }

    double bodyHeight() const
    {
        return _bodyHeight;
    }

    void setBodyHeight(double bodyHeight)
    {
        _bodyHeight = bodyHeight;
    }

    int numCars() const
    {
        return _numCars;
    }

    void setNumCars(int numCars)
    {
        _numCars = numCars;
    }

    double maxAcceleration() const
    {
        return _maxAcceleration;
    }

    void setMaxAcceleration(double accel)
    {
        _maxAcceleration = accel;
    }

    double maxDeceleration() const
    {
        return _maxDeceleration;
    }

    void setMaxDeceleration(double decel)
    {
        _maxDeceleration = decel;
    }

    double jamDistance() const
    {
        return _jamDistance;
    }

    void setJamDistance(double jamDistance)
    {
        _jamDistance = jamDistance;
    }

    double bodyColorR() const
    {
        return _bodyColorR;
    }

    double bodyColorG() const
    {
        return _bodyColorG;
    }

    double bodyColorB() const
    {
        return _bodyColorB;
    }

    ///@}
};

#endif //__VEHICLE_BODY_PROPERTY_HPP__
