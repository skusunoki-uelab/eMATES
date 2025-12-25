/* ******************************************************************
 * Copyright 2014-2024 ADVENTURE Project
 * All Rights Reserved
 * ****************************************************************** */
/**
 * @file VehicleTypeManager.cpp
 */
#include "VehicleTypeManager.hpp"
#include "AppMates.hpp"
#include "CustomMessage.hpp"
#include "GVManager.hpp"
#include "io/VehicleTypeBuilder.hpp"
#include <iostream>
#include <map>
#include <sstream>
#include <string>
#include <utility>
#include <vector>
#include <typeinfo>

using namespace std;

//======================================================================
VehicleTypeManager::VehicleTypeManager()
{
    _properties.clear();
    _className = typeid(this).name();
}

//======================================================================
void VehicleTypeManager::prepareVehicleType()
{
    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    // デフォルトで普通車, 大型車を作成する
    // Create PASSENGER, TRUCK by default
    double length, width, height;
    double weight = 0.0;
    double accel, decel;
    double r, g, b;

    length = AppMates::getGVManager().getNumeric("VEHICLE_LENGTH_PASSENGER");
    width  = AppMates::getGVManager().getNumeric("VEHICLE_WIDTH_PASSENGER");
    height = AppMates::getGVManager().getNumeric("VEHICLE_HEIGHT_PASSENGER");
    accel  = AppMates::getGVManager().getNumeric("MAX_ACCELERATION_PASSENGER");
    decel  = AppMates::getGVManager().getNumeric("MAX_DECELERATION_PASSENGER");
    r      = 1.0;
    g      = 0.0;
    b      = 0.0;

    addProperty(new VehicleTypeProperty(
        VehicleType(VehicleCategory::PASSENGER, 0), length, width, height,
        weight, 1, accel, decel, r, g, b));
    addProperty(new VehicleTypeProperty(
        VehicleType(VehicleCategory::PASSENGER, 1), length, width, height,
        weight, 1, accel, decel, r, g, b));
    length = AppMates::getGVManager().getNumeric("VEHICLE_LENGTH_TRUCK");
    width  = AppMates::getGVManager().getNumeric("VEHICLE_WIDTH_TRUCK");
    height = AppMates::getGVManager().getNumeric("VEHICLE_HEIGHT_TRUCK");
    accel  = AppMates::getGVManager().getNumeric("MAX_ACCELERATION_TRUCK");
    decel  = AppMates::getGVManager().getNumeric("MAX_DECELERATION_TRUCK");
    r      = 0.3;
    g      = 0.7;
    b      = 1.0;

    addProperty(new VehicleTypeProperty(
        VehicleType(VehicleCategory::TRUCK, 0), length, width, height, weight,
        1, accel, decel, r, g, b));
    addProperty(new VehicleTypeProperty(
        VehicleType(VehicleCategory::TRUCK, 1), length, width, height, weight,
        1, accel, decel, r, g, b));

    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    // 車両情報をファイルから読み込む
    // Read vehicle type setting from file
    VehicleTypeBuilder builder;
    builder.buildVehicleTypes(this);
}

//======================================================================
void VehicleTypeManager::addProperty(VehicleTypeProperty* vtp)
{
    // 重複した場合はメッセージを表示して上書きする
    // Show message and overwrite if duplicate
    auto itr = _properties.find(*(vtp->type()));
    if (itr != _properties.end())
    {
        ostringstream ssw;
        ssw << "VehicleTypeProperty[" << *(vtp->type())
            << "] has been overwritten";
        amu::msg::warn(ssw.str());
        _properties.erase(itr);
    }
    _properties.insert(make_pair(*(vtp->type()), vtp));
}

//======================================================================
VehicleTypeProperty* VehicleTypeManager::property(VehicleType type)
{
    auto itr = _properties.find(type);

    if (itr != _properties.end())
    {
        return (*itr).second;
    }
    else
    {
        ostringstream ssw;
        ssw << "VehicleTypeProperty[" << type << "] not found.";
        amu::msg::error(ssw.str());
        return nullptr;
    }
}

//======================================================================
void VehicleTypeManager::print(ostream& out) const
{
    amu::msg::title(out, "Vehicle Type Parameters");

    for (auto itr : _properties)
    {
        const VehicleType* type;
        double             length, width, height;
        double             weight = 0.0;
        int                numCars;
        double             accel, decel;
        double             r, g, b;
        ostringstream      ss;

        type = itr.second->type();
        itr.second->getSize(&length, &width, &height);
        numCars = itr.second->numCars();
        itr.second->getPerformance(&accel, &decel);
        itr.second->getBodyColor(&r, &g, &b);

        ss << *type << ": " << "(" << length << ", " << width << ", " << height
           << "/" << weight << "), " << numCars << ", (" << accel << ", "
           << decel << "), (" << r << ", " << g << ", " << b << ")";
        amu::msg::message(out, ss.str());
    }
}
