/* **************************************************************************
 * Copyright 2014-2024 ADVENTURE Project
 * All Rights Reserved
 * ************************************************************************** */
/**
 * @file VehicleRestriction.hpp
 */
#ifndef __VEHICLE_RESTRICTION_HPP__
#define __VEHICLE_RESTRICTION_HPP__
#include "VehicleType.hpp"
#include <iostream>
#include <string>
#include <vector>

//##############################################################################
/**
 * @~japanese 単路部や交差点の通行権を格納する構造体
 * @~english  Struct storing the right-of-way for a section and an intersection
 * @~ @ingroup routing  
 */
struct VehicleRestriction
{
public:
    VehicleRestriction()
    {
        _allowedVehicleTypes.clear();
        _deniedVehicleTypes.clear();
    };
    ~VehicleRestriction() {};

    /**
     * @~japanese 車種 @p type の通行を許可するか
     * @~english  Whether to permit vehicles of type @p type to pass
     */
    bool permitsPassing(const VehicleType type) const;

    /**
     * @~japanese 車種 @p type をAllowリストに追加する
     * @~english  Add vehicle type @p type to allow-list
     */
    void addAllowedVehicleType(const VehicleType& type)
    {
        _allowedVehicleTypes.emplace_back(type);
    }

    /**
     * @~japanese 車種 @p type をDenyリストに追加する
     * @~english  Add vehicle type @p type to deny-list
     */
    void addDeniedVehicleType(const VehicleType& type)
    {
        _deniedVehicleTypes.emplace_back(type);
    }

    /**
     * @~japanese 他の通行権設定をコピーする
     * @note コピーする際に重複をチェックする
     *
     * @~english  Copy other right-of-way property
     * @note Check for duplicates when copying
     */
    void addPermission(const VehicleRestriction& other);

    /**
     * @~japanese Allowリスト，Denyリストを @p out に出力する
     * @~english  Output allow-list and deny-list to @p out
     */
    void print(std::ostream& out) const;

    //==========================================================================
private:
    /**
     * @~japanese 通行権のAllowリスト (通行許可)
     * @attention Allowリストが優先される
     *
     * @~english  Allow-list of right-of-way (permission to pass)
     * @attention Allow-list takes precedence.
     */
    std::vector<VehicleType> _allowedVehicleTypes;

    /**
     * @~japanese 通行権のDenyリスト (通行禁止)
     * @attention Allowリストが優先される
     *
     * @~english  Deny-list of right-of-way (prohibition to pass)
     * @attention Allow-list takes precedence.
     */
    std::vector<VehicleType> _deniedVehicleTypes;

    //==================================================================
    /**
     * @~japanese @name アクセッサ
     * @~english  @name Accessor
     */
    ///@{
public:
    const std::vector<VehicleType>& allowedVehicleTypes() const
    {
        return _allowedVehicleTypes;
    }

    const std::vector<VehicleType>& deniedVehicleTypes() const
    {
        return _deniedVehicleTypes;
    }

    ///@}
};

#endif //__VEHICLE_RESTRICTION_HPP__
