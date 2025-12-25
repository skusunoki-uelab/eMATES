/* ******************************************************************
 * Copyright 2014-2024 ADVENTURE Project
 * All Rights Reserved
 * ****************************************************************** */
/**
 * @file VehicleTypeManager.hpp
 */
#ifndef __VEHICLE_TYPE_MANAGER_HPP__
#define __VEHICLE_TYPE_MANAGER_HPP__
#include "ManagerBase.hpp"
#include "VehicleType.hpp"
#include "VehicleTypeProperty.hpp"
#include <AmuConverter.hpp>
#include <iostream>
#include <map>
#include <sstream>

//######################################################################
/**
 * @~japanese 車種を管理する
 * @~english  Manager vehicle type
 * @~ @ingroup Manager
 */
class VehicleTypeManager : public ManagerBase
{
    friend class ManagerPool;

private:
    VehicleTypeManager();
    ~VehicleTypeManager()
    {
        for (auto itr : _properties)
        {
            delete itr.second;
        }
        _properties.clear();
    }

    /**
     * @~japanese 親クラスの関数のオーバーライド
     * @~english  Override functions of parent class
     */
    void _finalizeInside() override {}

public:
    /**
     * @~japanese 車種を用意する
     * @~english  Prepare vehicle type
     */
    void prepareVehicleType();

    /**
     * @~japanese 車種の属性 @p vtp を追加する
     * @~english  Add vehicle type property @p vtp
     */
    void addProperty(VehicleTypeProperty* vtp);

    /**
     * @~japanese 車種属性のコンテナへの参照を返す
     * @~english  Return reference to vehicle type property container
     */
    std::map<VehicleType, VehicleTypeProperty*>& properties()
    {
        return _properties;
    }

    /**
     * @~japanese 車種 @p type の属性を返す
     * @~english  Return property of vehicle type @p type
     */
    VehicleTypeProperty* property(VehicleType type);

    /**
     * @~japanese 車両属性を表示する
     * @~english  Display vehicle type properties
     */
    void print(std::ostream& out) const;

private:
    /**
     * @~japanese 車種属性のメインコンテナ
     *
     * @note
     * unordered_mapでもよいが，その場合はハッシュ関数が必要．
     *
     * @~english  Main container of vehicle type property
     *
     * @note
     * An unordered_map is also acceptable, but in that case a hash
     * function is required.
     */
    std::map<VehicleType, VehicleTypeProperty*> _properties;
};

#endif //__VEHICLE_TYPE_MANAGER_HPP__
