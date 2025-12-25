/* **************************************************************************
 * Copyright 2014-2024 ADVENTURE Project
 * All Rights Reserved
 * ************************************************************************** */
/**
 * @file TrafficCounterComponent.hpp
 */
#ifndef __TRAFFIC_COUNTER_COMPONENT_HPP__
#define __TRAFFIC_COUNTER_COMPONENT_HPP__
#include "VehicleType.hpp"
#include <AmuPoint.hpp>
#include <AmuVector.hpp>
#include <vector>
#include <string>

class Lane;
class Section;
class TrafficCounter;
class Vehicle;

//##############################################################################
/**
 * @~japanese
 * 1つの車両感知器を構成する，個々のレーン担当のコンポーネント
 * 
 * @~english
 * Component for individual lane that compose one traffic counter
 *
 * @~ @ingroup Monitoring
 */
class TrafficCounterComponent
{
public:
    //==========================================================================
    /**
     * @~japanese 通過車両データを格納する構造体
     * @~english  Struct storing passed vehicle record
     * @~ @ingroup Monitoring
     */
    struct Record
    {
    private:
        /**
         * @~japanese 車両が通過した車線の識別番号
         * @~english  ID number of Lane passed by vehicle
         */
        const std::string _laneId;

        /**
         * @~japanese 通過した車両の識別番号
         * @~english  ID number of Passing vehicle
         */
        const std::string _vehicleId;

        /**
         * @~japanese 車種
         * @~english  Vehicle type
         */
        const VehicleType _vehicleType;

        /**
         * @~japanese 始点の識別番号
         * @~english  ID number of origin
         */
        const std::string& _startId;

        /**
         * @~japanese 終点の識別番号
         * @~english  ID number of destination
         */
        const std::string& _goalId;

        /**
         * @~japanese 通過時の速度
         * @~english  Velocity when passing
         */
        const double _velocity;

    public:
        Record(
            const std::string& laneId, const std::string& vehicleId,
            const VehicleType& type, const std::string& startId,
            const std::string& goalId, const double velocity)
            : _laneId(laneId),
              _vehicleId(vehicleId),
              _vehicleType(type),
              _startId(startId),
              _goalId(goalId),
              _velocity(velocity)
        {
        }

        //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
        /**
         * @~japanese @name アクセッサ
         * @~english  @name Accessor
         */
        ///@{
    public:
        const std::string& laneId() const
        {
            return _laneId;
        }

        const std::string& vehicleId() const
        {
            return _vehicleId;
        }

        const VehicleType& vehicleType() const
        {
            return _vehicleType;
        }

        const std::string& startId() const
        {
            return _startId;
        }

        const std::string& goalId() const
        {
            return _goalId;
        }

        double velocity()
        {
            return _velocity;
        }

        ///@}
    };

    //==========================================================================
public:
    explicit TrafficCounterComponent(TrafficCounter* parent) : _parent(parent)
    {
        _lane     = nullptr;
        _distance = 0;
        _records.clear();
    }

    ~TrafficCounterComponent()
    {
        clearRecords();
    }

    /**
     * @~japanese
     * コンポーネントに車線 @p lane と観測位置 @p distance をセットする
     *
     * @~english
     * Set lane @p lane and monitoring position @p distance to component
     */
    void setPosition(Lane* lane, double distance)
    {
        _lane     = lane;
        _distance = distance;
    }

    /**
     * @~japanese 通過車両を記録する
     * @~english  Record passed vehicle
     */
    void recordPassedVehicle(Vehicle* vehicle);

    /**
     * @~japanese 記録を消去する
     * @~english  Clear records
     */
    void clearRecords()
    {
        for (auto itr : _records)
        {
            delete itr;
        }
        _records.clear();
    }

    //==========================================================================
private:
    /**
     * @~japanese このコンポーネントが所属する感知器
     * @~english  Traffic counter to which this component belongs 
     */
    const TrafficCounter* _parent;

    /**
     * @~japanese センサが設置された車線
     * @~english  Lane that this is installed
     */
    Lane* _lane;

    /**
     * @~japanese 車線始点からの距離
     * @~english  Distance from start point of the lane
     */
    double _distance;

    /**
     * @~japanese 現在のタイムステップで通過した車両の記録
     * @~english  Record of vehicle passed in the present timestep
     */
    std::vector<TrafficCounterComponent::Record*> _records;

    //==========================================================================
    /**
     * @~japanese @name アクセッサ
     * @~english  @name Accessor
     */
    ///@{
public:
    const TrafficCounter* parent() const
    {
        return _parent;
    }

    Lane* lane() const
    {
        return _lane;
    }

    double distance() const
    {
        return _distance;
    }

    amu::geometry::AmuPoint position();

    const std::vector<TrafficCounterComponent::Record*>& records() const
    {
        return _records;
    }

    ///@}
};

#endif //__TRAFFIC_COUNTER_COMPONENT_HPP__
