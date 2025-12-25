/* **************************************************************************
 * Copyright 2014-2024 ADVENTURE Project
 * All Rights Reserved
 * ************************************************************************** */
/**
 * @file VehicleRestriction.cpp
 */
#include "VehicleRestriction.hpp"
#include "CustomMessage.hpp"
#include <algorithm>
#include <sstream>

using namespace std;

//==============================================================================
bool VehicleRestriction::permitsPassing(const VehicleType type) const
{
    stringstream ss("");
    ss << type;

    /*
     * Allowリストが与えられている場合は，リストに列挙されている車種のみ
     * 通行可能で，他は通行不可．
     *
     * If the allow-list is given, only the vehicles with types listed
     * in the list can pass, and others cannot pass.
     */
    if (!(_allowedVehicleTypes.empty()))
    {
        if (find(
                _allowedVehicleTypes.begin(), _allowedVehicleTypes.end(),
                ss.str())
            != _allowedVehicleTypes.end())
        {
            return true;
        }
        return false;
    }

    /*
     * Allowリストが与えられていない場合は，Denyリストに列挙されている
     * 車種のみ通行不可で，他は通行可能．
     *
     * If no allow-list is given, only the vehicles listed with types
     * listed in the deny-list cannot pass, and others can pass.
     */
    else
    {
        if (find(
                _deniedVehicleTypes.begin(), _deniedVehicleTypes.end(),
                ss.str())
            != _deniedVehicleTypes.end())
        {
            return false;
        }
        return true;
    }
}

//==============================================================================
void VehicleRestriction::addPermission(const VehicleRestriction& other)
{
    // 重複しないよう同じ要素がないか検索しながら追加する．
    // Add while searching for the same element to avoid duplication.
    for (auto itr : other._allowedVehicleTypes)
    {
        if (find(_allowedVehicleTypes.begin(), _allowedVehicleTypes.end(), itr)
            == _allowedVehicleTypes.end())
        {
            _allowedVehicleTypes.emplace_back(itr);
        }
    }
    for (auto itr : other._deniedVehicleTypes)
    {
        if (find(_deniedVehicleTypes.begin(), _deniedVehicleTypes.end(), itr)
            == _deniedVehicleTypes.end())
        {
            _deniedVehicleTypes.emplace_back(itr);
        }
    }
}

//==============================================================================
void VehicleRestriction::print(ostream& out) const
{
    ostringstream oss;
    if (!_allowedVehicleTypes.empty())
    {
        oss << "AllowList: ";
        for (auto itr : _allowedVehicleTypes)
        {
            oss << itr << " ";
        }
    }
    if (!_deniedVehicleTypes.empty())
    {
        oss << "DenyList: ";
        for (auto itr : _deniedVehicleTypes)
        {
            oss << itr << " ";
        }
    }
    amu::msg::message(out, oss.str());
}
