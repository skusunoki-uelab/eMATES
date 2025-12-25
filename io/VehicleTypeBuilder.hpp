/* ******************************************************************
 * Copyright 2014-2024 ADVENTURE Project
 * All Rights Reserved
 * ****************************************************************** */
/**
 * @file VehicleTypeBuilder.hpp
 */
#ifndef __VEHICLE_TYPE_BUILDER_HPP__
#define __VEHICLE_TYPE_BUILDER_HPP__
#include "../VehicleTypeManager.hpp"

//######################################################################
/**
 * @~japanese
 * 車種の設定をファイルから読み込む
 *
 * @~english
 * Read vehicle type settings from file
 *
 * @~
 * @ingroup IO Initialization Vehicle
 * @see VehicleTypeManager
 */
class VehicleTypeBuilder
{
public:
    VehicleTypeBuilder() {};
    ~VehicleTypeBuilder() {};

    /**
     * @~japanese
     * ファイルから読み込んで車種設定を生成し，VehicleTypeManagerに
     * セットする
     *
     * @~english
     * Generate vehicle type settings by reading from file and set
     * them to VehicleTypeManager
     */
    void buildVehicleTypes(VehicleTypeManager* vtm);
};

#endif //__VEHICLE_TYPE_BUILDER_HPP__
