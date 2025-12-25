/* ******************************************************************
 * Copyright 2014-2024 ADVENTURE Project
 * All Rights Reserved
 * ****************************************************************** */
/**
 * @file VehicleTypeWriter.hpp
 */
#ifndef __VEHICLE_TYPE_WRITER_HPP__
#define __VEHICLE_TYPE_WRITER_HPP__
#include "../Vehicle.hpp"
#include <fstream>

//######################################################################
/**
 * @~japanese
 * 発生した車両の車種情報をファイルに書き込む
 *
 * @note
 * 静的クラスとする
 *
 * @~english
 * Write generated vehicle property to the file
 *
 * @note
 * Static class
 *
 * @~
 * @ingroup IO Vehicle
 * @see VehicleTypeManager
 */
class VehicleTypeWriter
{
public:
    VehicleTypeWriter() {};
    ~VehicleTypeWriter() {};

    /**
     * @~japanese 車両の属性をファイルに書きこむ
     * @~english  Write vehicle property to file
     */
    void writeVehicleProperty(Vehicle* vehicle);
};

#endif //__VEHICLE_TYPE_WRITER_HPP__
