/* ******************************************************************
 * Copyright 2014-2024 ADVENTURE Project
 * All Rights Reserved
 * ****************************************************************** */
/**
 * @file VehicleTripWriter.hpp
 */
#ifndef __VEHICLE_TRIP_WRITER_HPP__
#define __VEHICLE_TRIP_WRITER_HPP__
#include "../Vehicle.hpp"
#include "../VehicleEV.hpp"
#include <fstream>

//######################################################################
/**
 * @~japanese
 * 走行を終えた車両のトリップ情報をファイルに書き込む
 *
 * @~english
 * Write trip information of vehicles finished trip to a file
 *
 * @~
 * @ingroup IO Vehicle
 */
class VehicleTripWriter
{
public:
    VehicleTripWriter() {};
    ~VehicleTripWriter() {};

    /**
     * @~japanese @p vehicle のトリップ情報をファイルに書き込む
     * @~english  Write trip information of @p vehicle to file
     */
    static void writeVehicleTrip(Vehicle* vehicle);
    static void writeVehicleTripEV(VehicleEV* vehicle); // [eMATES]

    /// 走行中の全車両の走行距離データを出力する
    /**
     * @~japanese
     * 走行中の全車両のトリップ情報をファイルに書き込む
     *
     * @note
     * シミュレーション終了時の処理で，目的地に到達していないため
     * それまでの旅行距離と旅行時間を出力する．
     *
     * @~english
     * Write trip information of all running vehicles to a file
     *
     * @note
     * At the end of the simulation, the trip distance and trip time
     * up to that point are output because they have not reached their
     * destinations.
     */
    void writeAllVehiclesTrip();
};

#endif //__VEHICLE_TRIP_WRITER_HPP__
