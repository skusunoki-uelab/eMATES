/* **************************************************************************
 * Copyright 2014-2024 ADVENTURE Project
 * All Rights Reserved
 * ************************************************************************** */
/**
 * @file InflowMonitor.hpp
 */
#ifndef __INFLOW_MONITOR_HPP__
#define __INFLOW_MONITOR_HPP__
#include "Config.hpp"
#include "ObserverBase.hpp"
#include "VehicleType.hpp"
#include <string>
#include <vector>

class ODNode;
class Lane;
class Section;
class Vehicle;

//##############################################################################
/**
 * @~japanese 流入車両検出器
 *
 * ODノードに設置され，流入する車両を記録する
 *
 * @~english  Inflow vehicle monitor
 *
 * Installed on OD nodes and records inflow vehicles.
 *
 * @~ @ingroup Monitoring
 */
class InflowMonitor : public ObserverBase
{
public:
    //==========================================================================
    /**
     * @~japanese 記録を格納する構造体
     * @~english  Struct to store record
     * @~ @ingroup Monitoring
     */
    struct Record
    {
    private:
        /**
         * @~japanese ODNodeの識別番号
         * @~english  ID number of ODNode
         */
        const std::string _odNodeId;

        /**
         * @~japanese 車両が発生したレーンの識別番号
         * @~english  ID number of lane vehicle generated
         */
        const std::string _laneId;

        /**
         * @~japanese 発生車両の識別番号
         * @~english  ID number of generated vehicle
         */
        const std::string _vehicleId;

        /**
         * @~japanese 車種
         * @~english  Vehicle type
         */
        const VehicleType _vehicleType;

        /**
         * @~japanese 車頭時間間隔 [ms]
         * @~english  Time-headway [ms]
         */
        ulint _headway;

        /**
         * @~japanese 発生時刻 [ms]
         * @~english  Time of generation [ms]
         */
        ulint _genTime;

        /**
         * @~japanese 車両生成時間間隔 [ms]
         * @~english  Vehicle generation time interval [ms]
         */
        ulint _genInterval;

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

        //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    public:
        Record(
            const std::string& odNodeId, const std::string& laneId,
            const std::string& vehicleId, const VehicleType& type,
            ulint headway, ulint genTime, ulint genInterval,
            const std::string& startId, const std::string& goalId)
            : _odNodeId(odNodeId),
              _laneId(laneId),
              _vehicleId(vehicleId),
              _vehicleType(type),
              _headway(headway),
              _genTime(genTime),
              _genInterval(genInterval),
              _startId(startId),
              _goalId(goalId) {};

        //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
        /**
         * @~japanese @name アクセッサ
         * @~english  @name Accessor
         */
        ///@{
    public:
        const std::string& odNodeId() const
        {
            return _odNodeId;
        }

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

        ulint headway() const
        {
            return _headway;
        }

        ulint genTime() const
        {
            return _genTime;
        }

        ulint genInterval() const
        {
            return _genInterval;
        }

        const std::string& startId() const
        {
            return _startId;
        }

        const std::string& goalId() const
        {
            return _goalId;
        }

        ///@}
    };

    //==========================================================================
public:
    explicit InflowMonitor(const std::string& id) : _id(id)
    {
        _records.clear();
        _odNode  = nullptr;
        _section = nullptr;
    }

    ~InflowMonitor()
    {
        clearRecords();
    }

    /**
     * @~japanese 流入車両を記録する
     * @~english  Record inflow vehicle
     */
    void recordInflowVehicle(
        const Lane* lane, const Vehicle* vehicle, ulint headway, ulint genTime,
        ulint genInterval);

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

    /**
     * @~japanese 属性を @p out に出力する
     * @~english  Output attributes to @p out
     */
    void print(std::ostream& out) const;

    //==========================================================================
private:
    /**
     * @~japanese 識別番号
     * @~english  ID number
     */
    std::string _id;

    /**
     * @~japanese 観測対象のODNode
     * @~english  ODNode to be monitored
     */
    ODNode* _odNode;

    /**
     * @~japanese 車両が流入する単路部
     * @~english  Section into which vehicle flows
     */
    Section* _section;

    /**
     * @~japanese 現在のタイムステップで流入した車両の記録
     * @~english  Record of inflow vehicle at the current timestep
     */
    std::vector<InflowMonitor::Record*> _records;

    //==========================================================================
    /**
     * @~japanese @name アクセッサ
     * @~english  @name Accessor
     */
    ///@{
public:
    virtual const std::string& id() const override
    {
        return _id;
    }

    const ODNode* odNode() const
    {
        return _odNode;
    }

    void setODNode(ODNode* odNode)
    {
        _odNode = odNode;
    }

    const Section* section() const
    {
        return _section;
    }

    void setSection(Section* section)
    {
        _section = section;
    }

    const std::vector<InflowMonitor::Record*>& records() const
    {
        return _records;
    }

    ///@}
};

#endif //__INFLOW_MONITOR_HPP__
