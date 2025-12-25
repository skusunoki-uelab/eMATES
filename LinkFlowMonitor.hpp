/* **************************************************************************
 * Copyright 2014-2024 ADVENTURE Project
 * All Rights Reserved
 * ************************************************************************** */
/**
 * @file LinkFlowMonitor.hpp
 */
#ifndef __LINK_FLOW_MONITOR_HPP__
#define __LINK_FLOW_MONITOR_HPP__
#include "Config.hpp"
#include "ObserverBase.hpp"
#include "VehicleType.hpp"
#include <string>

class Section;
class Vehicle;

//##############################################################################
/**
 * @~japanese リンク旅行時間観測器
 * 
 * このクラスのインスタンスは，観測器が設置された単路部を通じて，交差点が持つ
 * LinkFlowRecordインスタンスにアクセス可能．LinkFlowRecordに保持されない
 * 観測情報をこのクラスで保持する．
 * 
 * @~english  Link travel time monitor
 * 
 * An instance of this class can access the LinkFlowRecord object of the
 * intersection via the section where the monitor is installed. It holds
 * observation results that LinkFlowRecord does not hold.
 * 
 * @~ @ingroup Monitoring
 */
class LinkFlowMonitor : public ObserverBase
{
public:
    //==========================================================================
    /**
     * @~japanese リンク通過車両データを格納する構造体
     * @~english  Struct storing link passing vehicle record
     * @~ @ingroup Monitoring
     */
    struct PassRecord
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
         * @~japanese リンク旅行時間
         * @~english  Link travel time;
         */
        const ulint _travelTime;

        /**
         * @~japanese リンク内での停止回数
         * @~english  Number of stops within the link
         */
        const unsigned int _numStops;

        /**
         * @~japanese リンク内での停止時間
         * @~english  Pause duration within the link
         */
        const ulint _stopTime;

    public:
        PassRecord(
            const std::string& laneId, const std::string& vehicleId,
            const VehicleType& type, const std::string& startId,
            const std::string& goalId, const ulint travelTime,
            unsigned int numStops, const ulint stopTime)
            : _laneId(laneId),
              _vehicleId(vehicleId),
              _vehicleType(type),
              _startId(startId),
              _goalId(goalId),
              _travelTime(travelTime),
              _numStops(numStops),
              _stopTime(stopTime)
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

        ulint travelTime()
        {
            return _travelTime;
        }

        unsigned int numStops()
        {
            return _numStops;
        }

        ulint stopTime()
        {
            return _stopTime;
        }

        ///@}
    };

    //==========================================================================
public:
    explicit LinkFlowMonitor(const std::string& id) : _id(id)
    {
        _section              = nullptr;
        _isUp                 = false;
        _outputsInCurrentStep = false;
    }

    ~LinkFlowMonitor()
    {
        clearRecords();
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
        for (auto itr : _passRecords)
        {
            delete itr;
        }
        _passRecords.clear();
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
     * @~japanese 観測対象の単路部
     * @~english  Section to be monitored
     */
    Section* _section;

    /**
     * @~japanese
     * 観測対象が単路部の上り方向であるかどうか
     *
     * @~english
     * Whether the measurement target is the ascending direction
     * ob the section
     */
    bool _isUp;

    /**
     * @~japanese 現在のタイムステップで通過した車両の記録
     * @~english  Record of vehicle passed in the present timestep
     */
    std::vector<LinkFlowMonitor::PassRecord*> _passRecords;

    /**
     * @~japanese このステップで統計量を出力するかどうか
     * @~english  Whether to output statistics in this step
     */
    bool _outputsInCurrentStep;

    //==========================================================================
    /**
     * @~japanese @name アクセッサ
     * @~english  @name Accessor
     */
    ///@{
public:
    const std::string& id() const override
    {
        return _id;
    }

    Section* section() const
    {
        return _section;
    }

    void setSection(Section* section)
    {
        _section = section;
    }

    bool isUp() const
    {
        return _isUp;
    }

    void setIsUp(bool isUp)
    {
        _isUp = isUp;
    }

    const std::vector<LinkFlowMonitor::PassRecord*>& passRecords() const
    {
        return _passRecords;
    }

    bool outputsInCurrentStep() const
    {
        return _outputsInCurrentStep;
    }

    void setOutputsInCurrentStep(bool outputsInCurrentStep)
    {
        _outputsInCurrentStep = outputsInCurrentStep;
    }

    ///@}
};

#endif //__LINK_FLOW_MONITOR_HPP__
