/* ******************************************************************
 * Copyright 2014-2024 ADVENTURE Project
 * All Rights Reserved
 * ****************************************************************** */
/**
 * @file PedestrianBodyProperty.hpp
 */
#ifdef INCLUDE_PEDESTRIANS
#ifndef __PEDESTRIAN_BODY_PROPERTY_HPP__
#define __PEDESTRIAN_BODY_PROPERTY_HPP__
#include "../Config.hpp"
#include <cassert>

class Pedestrian;

//######################################################################
/**
 * @~japanese 歩行者の属性を集約する構造体
 * @~english  Struct aggregating properties for a pedestrian
 * @~ @ingroup Pedestrian
 */
struct PedestrianBodyProperty
{
public:
    PedestrianBodyProperty()
        : _pedestrian(nullptr), _radius(0.5), _viewLength(2.5)
    {
        _psRadius  = _radius * _psFactor;
        _viewAngle = 160.0 * M_PI / 180.0;
    }

    ~PedestrianBodyProperty() {};

private:
    /**
     * @~japanese 所有者
     * @~english  Owner
     */
    Pedestrian* _pedestrian;

    /**
     * @~japanese 人体の半径 [m]
     * @~english  Human body radius [m]
     */
    double _radius;

    /**
     * @~japanese パーソナルスペース係数
     *
     * _radius に _psFactor を掛けてパーソナルスペース半径とする
     *
     * @~english  Personal space factor
     *
     * Get personal space radius by multiplying _radius by _psFactor
     */
    constexpr static double _psFactor = 1.2;

    /**
     * @~japanese パーソナルスペース半径 [m]
     * @~english  Personal space radius [m]
     */
    double _psRadius;

    /**
     * @~japanese 視野の半径 [m]
     * @~english  Visual field radius [m]
     */
    double _viewLength;

    /**
     * @~japanese 視野角 [rad]
     * @~english  Visual field angle [rad]
     */
    double _viewAngle;

    //==================================================================
    /**
     * @~japanese @name アクセッサ
     * @~english  @name Accessor
     */
    ///@{
public:
    void setPedestrian(Pedestrian* ped)
    {
        _pedestrian = ped;
    }

    double radius() const
    {
        return _radius;
    }

    double psRadius() const
    {
        return _psRadius;
    }

    double viewLength() const
    {
        return _viewLength;
    }

    double viewAngle() const
    {
        return _viewAngle;
    }

    ///@}
};

#endif //__PEDESTRIAN_BODY_PROPERTY_HPP__
#endif //INCLUDE_PEDESTRIANS
