/* **************************************************************************
 * Copyright 2014-2024 ADVENTURE Project
 * All Rights Reserved
 * ************************************************************************** */
/**
 * @file ObjectManager.cpp
 */
#include "ObjectManager.hpp"
#include "AppMates.hpp"
#include "Config.hpp"
#include "CustomMessage.hpp"
#include "GVManager.hpp"
#include <AmuConverter.hpp>
#include <AmuPoint.hpp>
#include <algorithm>
#include <cassert>
#include <iostream>
#include <typeinfo>

using namespace std;
using namespace amu::converter;
using namespace amu::geometry;

//==============================================================================
ObjectManager::ObjectManager()
{
    _connectors.clear();
    _trafficCounters.clear();
    _linkFlowMonitors.clear();
    _inflowMonitors.clear();
    _convoyMonitors.clear();
    _vehicles.clear();
    _numVehicles = 0;
#ifdef INCLUDE_PEDESTRIANS
    _pedestrians.clear();
    _numPedestrians = 0;
#endif //INCLUDE_PEDESTRIANS
    _className = typeid(this).name();
    AppMates::getGVManager().addDependee(this);
}

//==============================================================================
void ObjectManager::deleteAll()
{
    deleteAllConnectors();
    deleteAllTrafficCounters();
    deleteAllLinkFlowMonitors();
    deleteAllInflowMonitors();
    deleteAllConvoyMonitors();
    deleteAllAgents();
}

//==============================================================================
void ObjectManager::deleteAllAgents()
{
    deleteAllVehicles();

#ifdef INCLUDE_PEDESTRIANS
    deleteAllPedestrians();
#endif //INCLUDE_PEDESTRIANS
}

//==============================================================================
Connector* ObjectManager::createConnector(double x, double y, double z)
{
    AmuPoint   pt(x, y, z);
    Connector* ptConnector
        = new Connector(static_cast<int>(_connectors.size()), pt);
    _connectors.push_back(ptConnector);

    return ptConnector;
}

//==============================================================================
void ObjectManager::deleteAllConnectors()
{
    for (auto itr : _connectors)
    {
        delete itr;
    }
    _connectors.clear();
}

//==============================================================================
TrafficCounter* ObjectManager::trafficCounter(const string& id)
{
    for (unsigned int i = 0; i < _trafficCounters.size(); i++)
    {
        if (_trafficCounters[i]->id().compare(id) == 0)
        {
            return _trafficCounters[i];
        }
    }
    return nullptr;
}

//==============================================================================
bool ObjectManager::addTrafficCounter(TrafficCounter* counter)
{
    assert(counter);

    // duplication check
    if (find(_trafficCounters.begin(), _trafficCounters.end(), counter)
        != _trafficCounters.end())
    {
        cerr << "WARNING: traffic counter [" << counter->id()
             << "] has been already added." << endl;
        delete counter;
        return false;
    }
    _trafficCounters.emplace_back(counter);
    return true;
}

//==============================================================================
void ObjectManager::deleteAllTrafficCounters()
{
    for (auto itr : _trafficCounters)
    {
        delete itr;
    }
    _trafficCounters.clear();
}

//==============================================================================
void ObjectManager::printTrafficCounters(ostream& out) const
{
    amu::msg::title(out, "Traffic Counters");
    if (_trafficCounters.size() == 0)
    {
        amu::msg::message(out, "none");
        return;
    }
    for (auto itr : _trafficCounters)
    {
        itr->print(out);
    }
}

//==============================================================================
bool ObjectManager::addLinkFlowMonitor(LinkFlowMonitor* monitor)
{
    assert(monitor);

    // duplication check
    if (find(_linkFlowMonitors.begin(), _linkFlowMonitors.end(), monitor)
        != _linkFlowMonitors.end())
    {
        cerr << "WARNING: link flow monitor [" << monitor->id()
             << "] has been already added." << endl;
        delete monitor;
        return false;
    }
    _linkFlowMonitors.emplace_back(monitor);
    return true;
}

//==============================================================================
void ObjectManager::deleteAllLinkFlowMonitors()
{
    for (auto itr : _linkFlowMonitors)
    {
        delete itr;
    }
    _linkFlowMonitors.clear();
}

//==============================================================================
void ObjectManager::printLinkFlowMonitors(std::ostream& out) const
{
    amu::msg::title(out, "Link Travel Monitors");
    if (_linkFlowMonitors.size() == 0)
    {
        amu::msg::message(out, "none");
        return;
    }
    for (auto itr : _linkFlowMonitors)
    {
        itr->print(out);
    }
}

//==============================================================================
bool ObjectManager::addInflowMonitor(InflowMonitor* monitor)
{
    assert(monitor);

    // duplication check
    if (find(_inflowMonitors.begin(), _inflowMonitors.end(), monitor)
        != _inflowMonitors.end())
    {
        cerr << "WARNING: inflow monitor [" << monitor->id()
             << "] has been already added." << endl;
        delete monitor;
        return false;
    }
    _inflowMonitors.emplace_back(monitor);
    return true;
}

//==============================================================================
void ObjectManager::deleteAllInflowMonitors()
{
    for (auto itr : _inflowMonitors)
    {
        delete itr;
    }
    _inflowMonitors.clear();
}

//==============================================================================
void ObjectManager::printInflowMonitors(ostream& out) const
{
    amu::msg::title(out, "Inflow Vehicle Monitors");
    if (_inflowMonitors.size() == 0)
    {
        amu::msg::message(out, "none");
        return;
    }
    for (auto itr : _inflowMonitors)
    {
        itr->print(out);
    }
}

//==============================================================================
bool ObjectManager::addConvoyMonitor(ConvoyMonitor* monitor)
{
    assert(monitor);

    // duplication check
    if (find(_convoyMonitors.begin(), _convoyMonitors.end(), monitor)
        != _convoyMonitors.end())
    {
        cerr << "WARNING: convoy monitor [" << monitor->id()
             << "] has been already added." << endl;
        delete monitor;
        return false;
    }
    _convoyMonitors.emplace_back(monitor);
    return true;
}

//==============================================================================
void ObjectManager::deleteAllConvoyMonitors()
{
    for (auto itr : _convoyMonitors)
    {
        delete itr;
    }
    _convoyMonitors.clear();
}

//==============================================================================
void ObjectManager::printConvoyMonitors(ostream& out) const
{
    amu::msg::title(out, "Convoy Monitors");
    if (_convoyMonitors.size() == 0)
    {
        amu::msg::message(out, "none");
        return;
    }
    for (auto itr : _convoyMonitors)
    {
        itr->print(out);
    }
}

//==============================================================================
Vehicle* ObjectManager::vehicle(const string& id)
{
    for (unsigned int i = 0; i < _vehicles.size(); i++)
    {
        if (_vehicles[i]->id() == id)
        {
            return _vehicles[i];
        }
    }
    return nullptr;
}

//==============================================================================
Vehicle* ObjectManager::createVehicle()
{
    Vehicle* tmpVehicle = new Vehicle();
    return tmpVehicle;
}

//==============================================================================
VehicleEV* ObjectManager::createVehicleEV() // [eMATES]
{
    VehicleEV* tmpVehicle = new VehicleEV();
    return tmpVehicle;
}

//==============================================================================
bool ObjectManager::addVehicleToRoadMap(Vehicle* vehicle)
{
    assert(vehicle);
    assert(vehicle->id().empty());
    bool flag = false;

#pragma omp critical(addVehicleToRoadMap)
    {
        vehicle->setId(
            formatId(to_string(_numVehicles), NUM_FIGURE_FOR_VEHICLE));
        _numVehicles++;

        // duplication check
        if (find(_vehicles.begin(), _vehicles.end(), vehicle)
            == _vehicles.end())
        {
            flag = true;
            _vehicles.emplace_back(vehicle);
        }
    }

    return flag;
}

//==============================================================================
void ObjectManager::deleteAllVehicles()
{
    for (auto itr : _vehicles)
    {
        delete itr;
    }
    _vehicles.clear();
    _numVehicles = 0;
}

//==============================================================================
void ObjectManager::deleteVehicle(Vehicle* vehicle)
{
    auto itr = find(_vehicles.begin(), _vehicles.end(), vehicle);
    if (itr != _vehicles.end())
    {
        delete *itr;
        _vehicles.erase(itr);
    }
}
