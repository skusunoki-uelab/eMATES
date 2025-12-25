/* ******************************************************************
 * Copyright 2014-2024 ADVENTURE Project
 * All Rights Reserved
 * ****************************************************************** */
/**
 * @file PedestrianLocation.hpp
 */
#ifdef INCLUDE_PEDESTRIANS
#ifndef __PEDESTRIAN_LOCATION_HPP__
#define __PEDESTRIAN_LOCATION_HPP__
#include "../Intersection.hpp"
#include <AmuConverter.hpp>
#include <AmuVector.hpp>
#include <AmuPoint.hpp>
#include <cassert>
#include <iostream>

class Pedestrian;
class Vehicle;
class Zebra;

//######################################################################
/**
 * @~japanese
 * 歩行者エージェントの時空間上の位置に関する変数を集約する構造体
 *
 * おもに PedestrianActor によって更新される．
 *
 * @~english
 * Struct aggregating variables related to the spatio-temporal position
 * of the pedestrian agent
 *
 * Mainly updated by PedestrianActor .
 *
 * @~
 * @ingroup Pedestrian
 */
struct PedestrianLocation
{
public:
    PedestrianLocation();
    ~PedestrianLocation() {};

    /**
     * @~japanese グローバル座標系におけるx座標を返す
     * @~english  Return x-coordinate in global coordinate system
     */
    double x() const
    {
        return _position.x();
    }

    /**
     * @~japanese グローバル座標系におけるy座標を返す
     * @~english  Return y-coordinate in global coordinate system
     */
    double y() const
    {
        return _position.y();
    }

    /**
     * @~japanese グローバル座標系におけるz座標を返す
     * @~english  Return z-coordinate in global coordinate system
     */
    double z() const
    {
        return _position.z();
    }

    /**
     * @~japanese 位置に関する変数を @p out に出力する
     * @~english  Display position variables to @p out
     */
    void print(std::ostream& out) const;

private:
    /**
     * @~japanese 所有者
     * @~english  Owner
     */
    Pedestrian* _pedestrian;

    /**
     * @~japanese この歩行者が所属する交差点
     * @~english  Intersection to which this pedestrian belongs
     */
    const Intersection* _intersection;

    /**
     * @~japanese この歩行者が所属する横断歩道
     * @~english  Crosswalk to which this pedestrian belongs
     */
    Zebra* _zebra;

    /**
     * @~japanese この歩行者が横断歩道にいるかどうか
     * @~english  Whether this pedestrian is in a crosswalk
     */
    bool _isOnZebra;

    /**
     * @~japanese グローバル座標系における位置
     * @~english  Position in global coordinate system
     */
    amu::geometry::AmuPoint _position;

    /**
     * @~japanese 生成時刻 [ms]
     *
     * @note
     * コンストラクタ内で時刻を設定する
     *
     * @~english  Generation time [ms]
     *
     * @note
     * Set the generation time in constructor.
     */
    ulint _generationTime;

    /**
     * @~japanese 出発時刻 [ms]
     * @~english  Starting time [ms]
     */
    ulint _startingTime;

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

    const Intersection* intersection() const
    {
        return _intersection;
    }

    void setIntersection(const Intersection* inter)
    {
        _intersection = inter;
    }

    Zebra* zebra() const
    {
        return _zebra;
    }

    void setZebra(Zebra* zebra)
    {
        _zebra = zebra;
    }

    bool isOnZebra() const
    {
        return _isOnZebra;
    }

    void setIsOnZebra(bool flag)
    {
        _isOnZebra = flag;
    }

    const amu::geometry::AmuPoint& position() const
    {
        return _position;
    }

    void setPosition(const amu::geometry::AmuPoint& p)
    {
        _position = p;
    }

    void setPosition(double x, double y, double z)
    {
        _position.setXYZ(x, y, z);
    }

    ulint generationTime() const
    {
        return _generationTime;
    }

    void setGenerationTime(ulint generationTime)
    {
        _generationTime = generationTime;
    }

    ulint startingTime() const
    {
        return _startingTime;
    }

    void setStartingTime(ulint time)
    {
        _startingTime = time;
    }

    ///@}
};

#endif //__PEDESTRIAN_LOCATION_HPP__
#endif //INCLUDE_PEDESTRIANS
