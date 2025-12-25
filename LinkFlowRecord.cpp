/* **************************************************************************
 * Copyright 2014-2024 ADVENTURE Project
 * All Rights Reserved
 * ************************************************************************** */
/**
 * @file LinkFlowRecord.cpp
 */
#include "LinkFlowRecord.hpp"
#include "AppMates.hpp"
#include "CustomMessage.hpp"
#include "Intersection.hpp"
#include "LinkFlowMonitor.hpp"
#include "Section.hpp"
#include "TimeManager.hpp"
#include "Vehicle.hpp"
#include <cassert>
#include <cstdlib>
#include <iostream>

using namespace std;

//==============================================================================
void LinkFlowRecord::recordPositionsInSection()
{
    // 開始時刻を更新する
    // Update the start time
    _startTime = AppMates::getTimeManager().time();

    // 車両の位置を記録する
    // Record vehicle positions
    bool isUp = _section->isUp(_section->anotherIntersection(_inter), _inter);
    for (auto itr_l : _section->lanesWithDirection(isUp))
    {
        for (auto itr_a : const_cast<Lane*>(itr_l)->agents())
        {
            const Vehicle* vehicle = dynamic_cast<Vehicle*>(itr_a);
            if (!vehicle)
            {
                continue;
            }
            _startPositionRecords.insert(
                make_pair(vehicle->id(), vehicle->distanceFromInflowBorder()));
        }
    }
}

//==============================================================================
void LinkFlowRecord::recordOutflowFromSection(const Vehicle* vehicle)
{
    // 計測時間帯における走行距離
    // Travel distance during observation period
    double distance = _section->length();

    /*
     * _startPositionsに車両が記録されているか調査する
     *   記録がある場合，車両が単路部に流入したのは過去の計測時間帯であるため，
     *   _startPositionsに記録された位置から速度を求める．
     * 
     * Check if the vehicle is recorded in _startPositions
     *   If there is a record, the vehicle flowed into the section during the
     *   past observation period, so calculate the speed from the position
     *   recorded in _startPositions.
     */
    auto itr_p = _startPositionRecords.find(vehicle->id());
    if (itr_p != _startPositionRecords.end())
    {
        assert(vehicle->location()->sectionInflowTime() <= _startTime);
        /*
         * 単路部が長方形でない場合に，実際の車両の走行距離が単路部の長さ
         * (2つの境界の中点を結ぶ線分の長さ）より長くなり，その結果 distance は
         * 負になりうることに注意．全体への影響は小さいと考えてそのままとする．
         * 
         * Note that if the shape of the road segment is not a rectangle, the 
         * distance a vehicle actually travels will be longer than the length 
         * of the road segment (the length of the line segment connecting the 
         * midpoints of the 2 borders), and consequently the distance can be 
         * negative. The effect on the overall result seems small and left as 
         * it is.
         */
        distance -= (*itr_p).second;

        /*
         * _startPositionRecords から記録を除外しないと，同じ単路部を複数回
         * 通過した場合にエラーが生じる可能性がある．
         * 
         * If the record are not excluded from _startPositionRecords, an error
         * may occur if a vehicle passed one road segment multiple times.
         */
        _startPositionRecords.erase(itr_p);
    }
    else
    {
        assert(vehicle->location()->sectionInflowTime() >= _startTime);
    }
    _sumDistanceRecordsInSection += distance;

    // 計測時間帯における単路部の走行時間
    // Travel time on the section during the observation period
    ulint duration = AppMates::getTimeManager().time()
        - max(vehicle->location()->sectionInflowTime(), _startTime)
        + AppMates::getTimeManager().unit();
    _sumTimeRecordsInSection += duration;
}

//==============================================================================
void LinkFlowRecord::recordOutflowFromIntersection(
    const Vehicle* vehicle, int to)
{
    // implement me
    unsigned int diff = (to - _dir + _inter->numNexts()) % _inter->numNexts();

    ulint travelTime = AppMates::getTimeManager().time()
        - vehicle->location()->sectionInflowTime();

    /**
     * 流出台数，および，実際の旅行時間と_estimatedTravelTimeInSectionとの差分の
     * 和を更新
     *   _estimatedTravelTimeInSectionは直前の計測時間帯の値を保持している．
     * 
     * The number of outflow vehicles and the difference between the actual
     * travel time and _estimatedTravelTimeInSection
     *   Note that _estimatedTravelTimeInSection keeps the value in the previous
     *   observation period.
     */
    _numOutflows[diff]++;
    _sumAdditionalTimeRecords[diff]
        += (travelTime - _estimatedTravelTimeInSection);
}

//==============================================================================
void LinkFlowRecord::calcMacroQuantities()
{
    /*
     * 計測時間帯中に単路部から流出した車両の情報
     *
     * Information on vehicles that flowed out from the section during
     * the observation period
    * */
    double sumDistances = _sumDistanceRecordsInSection; //[m]
    ulint  sumTimes     = _sumTimeRecordsInSection;     //[ms]

    /*
     * 現時点で単路部を走行中の車両の状態を参照する
     *
     * Refer to the status on vehicles currently traveling on the
     * section
     */
    bool isUp = _section->isUp(_section->anotherIntersection(_inter), _inter);
    for (auto itr_l : _section->lanesWithDirection(isUp))
    {
        for (auto itr_a : const_cast<Lane*>(itr_l)->agents())
        {
            const Vehicle* vehicle = dynamic_cast<Vehicle*>(itr_a);
            if (!vehicle)
            {
                continue;
            }
            double d = vehicle->distanceFromInflowBorder();

            //_startPositionsに車両が記録されているか調査する
            // Check if the vehicle is recorded in _startPositions
            auto itr_p = _startPositionRecords.find(vehicle->id());
            if (itr_p != _startPositionRecords.end())
            {
                d -= (*itr_p).second;
            }
            sumDistances += d;

            sumTimes += AppMates::getTimeManager().time()
                - max(_startTime, vehicle->location()->sectionInflowTime());
        }
    }

    // 時空間領域の面積 [m ms]
    // area of ​​spatiotemporal region [m ms]
    double a
        = _section->length() * (AppMates::getTimeManager().time() - _startTime);

    // 交通量，空間平均速度，車両密度の算出
    // Calculate traffic volume, space mean speed, vehicle density
    _volumeInSection = (a > 0.0 ? sumDistances / a : 0.0); //[veh/ms]
    if (sumTimes == 0)
    {
        _meanVelocityInSection = _section->speedLimit(isUp) / 3600.0; //[m/ms]
    }
    else
    {
        _meanVelocityInSection = sumDistances / sumTimes; //[m/ms]
    }
    _densityInSection = _volumeInSection / _meanVelocityInSection; //[veh/m]

    // 単路部における予想リンク旅行時間の算出 [ms]
    // Calculate estimated link travel time in section [ms]
    _estimatedTravelTimeInSection = _section->length() / _meanVelocityInSection;

    /*
     * 交差点を含めた方向別予想リンク旅行時間 [ms]
     *
     * Estimated link travel time including intersections by
     * direction [ms]
     */
    for (unsigned int i = 0; i < _numOutflows.size(); i++)
    {
        // 旅行時間の平均補正量
        // Average correction in travel time
        double aveAdditionalTime
            = _sumAdditionalTimeRecords[i] / _numOutflows[i];

        /*
         * 平均補正量が正の場合は予想リンク旅行時間を補正する
         *
         * If the average correction is positive, correct the expected link
         * travel time.
         */
        _estimatedTravelTimes[i]
            = _estimatedTravelTimeInSection + max(0.0, aveAdditionalTime);
    }
}

//==============================================================================
void LinkFlowRecord::instructOutput() const
{
    bool isUp = _section->isUp(_section->anotherIntersection(_inter), _inter);
    LinkFlowMonitor* monitor = _section->linkFlowMonitor(isUp);
    if (!monitor)
    {
        return;
    }
    else
    {
        monitor->setOutputsInCurrentStep(true);
    }
}

//==============================================================================
void LinkFlowRecord::print(ostream& out) const
{
    ostringstream oss;
    oss << "LinkFlowRecord[" << _inter->id() << "/" << _dir << "/"
        << _section->id() << "]";
    amu::msg::message(out, oss.str());
}

//==============================================================================
void LinkFlowRecord::printRecords(ostream& out) const
{
    ostringstream oss;
    oss << "StartTime [ms]   = " << _startTime << endl;
    oss << "SumDistance [m]  = " << _sumDistanceRecordsInSection << endl;
    oss << "SumTime [ms]     = " << _sumTimeRecordsInSection << endl;
    oss << "Volume [veh/ms]  = " << _volumeInSection << endl;
    oss << "Density [veh/m]  = " << _densityInSection << endl;
    oss << "Velocity [m/ms]  = " << _meanVelocityInSection << endl;
    oss << "EstTimeSect [ms] = " << _estimatedTravelTimeInSection;
    for (unsigned int i = 0; i < _numOutflows.size(); i++)
    {
        oss << endl
            << "EstTime(" << i << ")  [ms] = " << _estimatedTravelTimes[i];
    }
    amu::msg::message(out, oss.str());
}
