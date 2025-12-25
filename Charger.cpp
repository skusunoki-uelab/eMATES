#include "Charger.hpp"
#include <iostream>

using namespace std;

//----------------------------------------------------------------------
// ChargerBase
//----------------------------------------------------------------------
//======================================================================
ChargerBase::ChargerBase(int id, int timeSlot)
    : _id(id),
    _volumes(  timeSlot, 0),
    _outPowers(timeSlot, 0),
    _prices(   timeSlot, 0)
{
}

//======================================================================
int ChargerBase::id() const
{
    return _id;
}
//======================================================================
vector<int>& ChargerBase::volumes()
{
    return _volumes;
}
//======================================================================
const vector<int>& ChargerBase::volumesConst() const
{
    return _volumes;
}

//======================================================================
vector<double>& ChargerBase::outPowers()
{
    return _outPowers;
}
//======================================================================
const vector<double>& ChargerBase::outPowersConst() const
{
    return _outPowers;
}

//======================================================================
vector<double>& ChargerBase::prices()
{
    return _prices;
}
//======================================================================
const vector<double>& ChargerBase::pricesConst() const
{
    return _prices;
}


//----------------------------------------------------------------------
// ChargerFast
//----------------------------------------------------------------------
//======================================================================
ChargerFast::ChargerFast(int id, int timeSlot)
    : ChargerBase(id, timeSlot)
{
    _vehicle = nullptr;
    _volumes[0] = 0;
}

//======================================================================
VehicleEV* ChargerFast::vehicle()
{
    return _vehicle;
}

//======================================================================
bool ChargerFast::setVehicle(VehicleEV* vehicle)
{
    _vehicle = vehicle;
    _volumes[0] = 1;
    return true;
}

//======================================================================
bool ChargerFast::removeVehicle(VehicleEV*)
{
    _vehicle = nullptr;
    _volumes[0] = 0;
    return true;
}

//======================================================================
bool ChargerFast::isFull() const
{
    return _vehicle;
}

//======================================================================
bool ChargerFast::isEmpty() const
{
    return ! _vehicle;
}

//----------------------------------------------------------------------
// ChargerNormal
//----------------------------------------------------------------------
//======================================================================
ChargerNormal::ChargerNormal(int id, int capacity, int timeSlot)
    : ChargerBase(id, timeSlot),
      _capacity(capacity)
{
}

//======================================================================
unordered_set<VehicleEV*>& ChargerNormal::vehicles()
{
    return _vehicles;
}

//======================================================================
bool ChargerNormal::setVehicle(VehicleEV* vehicle)
{
    if ( _vehicles.size() >= _capacity )
    {
        return false;
    }
    _vehicles.emplace(vehicle);
    _volumes[0] = _vehicles.size();
    return true;
}

//======================================================================
bool ChargerNormal::removeVehicle(VehicleEV* vehicle)
{
    if ( _vehicles.count(vehicle) == 0)
    {
        return false;
    }
    _vehicles.erase(vehicle);
    _volumes[0] = _vehicles.size();
    return true;
}

//======================================================================
bool ChargerNormal::isFull() const
{
    return _vehicles.size() >= _capacity;
}

//======================================================================
bool ChargerNormal::isEmpty() const
{
    return _vehicles.size() == 0;
}


