/* ******************************************************************
 * Copyright 2014-2024 ADVENTURE Project
 * All Rights Reserved
 * ****************************************************************** */
/**
 * @file VehicleLocation.hpp
 */
#ifndef __VEHICLE_LOCATION_HPP__
#define __VEHICLE_LOCATION_HPP__
#include "Config.hpp"
#include "RoadMap.hpp"
#include "Intersection.hpp"
#include "Lane.hpp"
#include "LaneBundle.hpp"
#include "Section.hpp"
#include "TimeManager.hpp"
#include <AmuVector.hpp>
#include <AmuPoint.hpp>
#include <cassert>
#include <iostream>

class Vehicle;

//######################################################################
/**
 * @~japanese
 * 自動車エージェントの時空間上の位置に関する変数を集約する構造体
 *
 * おもに VehicleActor , VehicleLaneChangeActor によって更新される．
 *
 * @~english
 * Struct aggregating variables related to the spatio-temporal position
 * of the car agent
 *
 * Mainly updated by VehicleActor and VehicleLaneChangeActor.
 *
 * @~
 * @ingroup Vehicle 
 */
struct VehicleLocation
{
public:
    VehicleLocation();
    ~VehicleLocation() {};

    /**
     * @~japanese 現在所属するレーン束オブジェクトを戻す
     * @~english  Return laneBundle object currently belonging
     */
    const LaneBundle* laneBundle() const
    {
        assert(
            (!_section && _intersection)
            || (_section && !_intersection));
        if (_section)
        {
            return _section;
        }
        else
        {
            return _intersection;
        }
    }

    /**
     * @~japanese 速度の単位ベクトルを戻す
     * @~english  Return unit vector of velocity
     */
    const amu::math::AmuVector directionVector() const
    {
        assert(_lane);
        return _lane->directionVector(_distance);
    }

    /**
     * @~japanese 車線始点からの距離に @p gain を加える
     * @~english  Add @p gain to distance from lane starting point
     */
    void addDistance(double gain)
    {
        _distance += gain;
    }

    /**
     * @~japanese
     * 前ステップにおける車線始点からの距離に @p gain を加える
     *
     * @~english
     * Add @p gain to distance from lane starting point in previous step
     */
    void addOldDistance(double gain)
    {
        _oldDistance += gain;
    }

    /**
     * @~japanese 単路部始点からの距離に @p gain を加える
     * @~english  Add @p gain to distance from section starting point
     */
    void addDistanceFromInflowBorder(double gain)
    {
        _distanceFromInflowBorder += gain;
    }

    /**
     * @~japanese トリップ長に @p gain を加える
     * @~english  Add @p gain to trip length
     */
    void addTripLength(double gain)
    {
        _tripLength += gain;
    }

    /**
     * @~japanese 出発ステップを戻す
     * @~english  Return starting step
     */
    ulint startingStep() const;

    /**
     * @~japanese 現在位置をグローバル座標系で返す
     * @~english  Return current position in global coordinate system
     */
    amu::geometry::AmuPoint position() const;

    /**
     * @~japanese グローバル座標系におけるx座標を返す
     * @~english  Return x-coordinate in global coordinate system
     */
    double x() const
    {
        return position().x();
    }

    /**
     * @~japanese グローバル座標系におけるy座標を返す
     * @~english  Return y-coordinate in global coordinate system
     */
    double y() const
    {
        return position().y();
    }

    /**
     * @~japanese グローバル座標系におけるz座標を返す
     * @~english  Return z-coordinate in global coordinate system
     */
    double z() const
    {
        return position().z();
    }

    /**
     * @~japanese 位置に関する変数を @p out に出力する
     * @~english  Display position variables to @p out
     */
    void print(std::ostream& out) const;

    //====================================================================
private:
    /**
     * @~japanese 持ち主
     * @~english  Owner
     */
    const Vehicle* _vehicle;

    /**
     * @~japanese 自車が所属する道路地図
     * @~english  Road map to which the subject car belongs
     */
    const RoadMap* _roadMap;

    /**
     * @~japanese 自車が所属する交差点
     * @~english  Intersection to which the subject car belongs
     */
    const Intersection* _intersection;

    /**
     * @~japanese
     * 自車が通過した交差点
     *
     * @~english
     * Intersection through which the subject car lastly passed
     */
    const Intersection* _prevIntersection;

    /**
     * @~japanese 自車が所属する単路部
     * @~english  Section to which the subject car belongs
     */
    const Section* _section;

    /**
     * @~japanese 自車が所属する車線
     * @~english  Lane to which the subject car belongs
     */
    const Lane* _lane;

    /**
     * @~japanese 自車が次に進入する車線
     * @~english  Lane in which the subject car will enter next
     */
    const Lane* _nextLane;

    /**
     * @~japanese 自車が最後に通過した車線
     * @~english  Lane through which the subject car lastly passed
     */
    const Lane* _prevLane;

    /**
     * @~japanese 車線の始点からの距離 [m]
     * @~english  Distance from lane starting point [m]
     */
    double _distance;

    /**
     * @~japanese 車線中心線からのずれ [m]
     *
     * @note
     * 車線変更時にゼロ以外の値をとる．中心線に対して左を正とする．
     *
     * @~english  Deviation from lane centerline [m]
     *
     * @note
     * Take a non-zero value when lane-changing. The left side of
     * the centerline is positive.
     */
    double _error;

    /**
     * @~japanese 前ステップにおける車線始点からの距離 [m]
     *
     * @note
     * 特定の点を通過したステップを検知する際に用いる
     *
     * @~english  Distance from lane starting point in previous step [m]
     *
     * @note
     * Used to detect the step when the subject car passed through
     * a specific point
     */
    double _oldDistance;

    /**
     * @~japanese 単路部始点からの距離 [m]
     *
     * @note
     * VehicleActor::runSection2** 内でクリアする
     *
     * @~english  Distance from section starting point [m]
     *
     * @note
     * Clear in VehicleActor::runSection2**.
     */
    double _distanceFromInflowBorder;

    /**
     * @~japanese トリップ長 [m]
     * @~english  Trip length [m]
     */
    double _tripLength;

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
     *
     * @note
     * ODNode::pushVehicleToReal() 内で時刻を設定する
     *
     * @~english  Starting time [ms]
     *
     * @note
     * Set the starting time in ODNode::pushVehicleToReal() .
     */
    ulint _startingTime;

    /**
     * @~japanese
     * 車両が単路部に流入した時刻 [ms]
     *
     * @note
     * VehicleActor::runIntersection2Section() 内でリセットされる
     *
     * @~english
     * Time when the subject car entered the section [ms]
     *
     * @note
     * Reset in VehicleActor::runIntersection2Section() .
     */
    ulint _sectionInflowTime;

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

    const RoadMap* roadMap() const
    {
        return _roadMap;
    }

    void setRoadMap(RoadMap* roadMap)
    {
        _roadMap = roadMap;
    }

    const Intersection* intersection() const
    {
        return _intersection;
    }

    void setIntersection(const Intersection* inter)
    {
        _intersection = inter;
    }

    const Intersection* prevIntersection() const
    {
        return _prevIntersection;
    }

    void setPrevIntersection(const Intersection* inter)
    {
        _prevIntersection = inter;
    }

    const Section* section() const
    {
        return _section;
    }

    void setSection(const Section* section)
    {
        _section = section;
    }

    const Lane* lane() const
    {
        return _lane;
    }

    void setLane(const Lane* lane)
    {
        _lane = lane;
    }

    const Lane* nextLane() const
    {
        return _nextLane;
    }

    void setNextLane(const Lane* lane)
    {
        _nextLane = lane;
    }

    const Lane* prevLane() const
    {
        return _prevLane;
    }

    void setPrevLane(const Lane* lane)
    {
        _prevLane = lane;
    }

    double distance() const
    {
        return _distance;
    }

    void setDistance(double distance)
    {
        _distance = distance;
    }

    double error() const
    {
        return _error;
    }

    void setError(double error)
    {
        _error = error;
    }

    double oldDistance() const
    {
        return _oldDistance;
    }

    void setOldDistance(double distance)
    {
        _oldDistance = distance;
    }

    double distanceFromInflowBorder() const
    {
        return _distanceFromInflowBorder;
    }

    void setDistanceFromInflowBorder(double distance)
    {
        _distanceFromInflowBorder = distance;
    }

    double tripLength() const
    {
        return _tripLength;
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

    void setStartingTime(ulint startingTime)
    {
        _startingTime = startingTime;
    }

    ulint sectionInflowTime() const
    {
        return _sectionInflowTime;
    }

    void setSectionInflowTime(ulint sectionInflowTime)
    {
        _sectionInflowTime = sectionInflowTime;
    }

    ///@}
};

#endif //__VEHICLE_LOCATION_HPP__
