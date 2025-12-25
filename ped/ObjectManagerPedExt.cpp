/* **************************************************************************
 * Copyright 2014-2024 ADVENTURE Project
 * All Rights Reserved
 * ************************************************************************** */
/**
 * @file ObjManagerPedExt.cpp
 */
#ifdef INCLUDE_PEDESTRIANS
#include "Pedestrian.hpp"
#include "../Config.hpp"
#include "../CustomMessage.hpp"
#include "../ObjectManager.hpp"
#include <AmuConverter.hpp>
#include <cassert>
#include <sstream>

using namespace std;
using namespace amu::converter;

//==============================================================================
bool ObjectManager::addInflowPedestrianMonitor(InflowPedestrianMonitor* monitor)
{
    assert(monitor);

    // duplication check
    if (find(
            _inflowPedestrianMonitors.begin(), _inflowPedestrianMonitors.end(),
            monitor)
        != _inflowPedestrianMonitors.end())
    {
        cerr << "WARNING: inflow pedestrian monitor [" << monitor->id()
             << "] has been already added." << endl;
        delete monitor;
        return false;
    }
    _inflowPedestrianMonitors.emplace_back(monitor);
    return true;
}

//==============================================================================
void ObjectManager::deleteAllInflowPedestrianMonitors()
{
    for (auto itr : _inflowPedestrianMonitors)
    {
        delete itr;
    }
    _inflowPedestrianMonitors.clear();
}

//==============================================================================
void ObjectManager::printInflowPedestrianMonitors(ostream& out) const
{
    amu::msg::title(out, "Inflow Pedestrian Monitors");
    if (_inflowPedestrianMonitors.size() == 0)
    {
        amu::msg::message(out, "none");
        return;
    }
    for (auto itr : _inflowPedestrianMonitors)
    {
        itr->print(out);
    }
}

//==============================================================================
std::vector<Pedestrian*>& ObjectManager::pedestrians()
{
    return _pedestrians;
}

//==============================================================================
Pedestrian* ObjectManager::pedestrian(const std::string& id)
{
    for (auto itr : _pedestrians)
    {
        if (itr->id().compare(id) == 0)
        {
            return itr;
        }
    }
    return nullptr;
}

//==============================================================================
Pedestrian* ObjectManager::createPedestrian()
{
    Pedestrian* ped = new Pedestrian();
    return ped;
}

//==============================================================================
bool ObjectManager::addPedestrianToRoadMap(Pedestrian* ped)
{
    assert(ped);

#ifdef PDS_DEBUG
    cerr << "addPedestrianToReal(): p$(" << ped << ") : id()=[" << ped->id()
         << "]" << endl;
    cerr.flush();
#endif

    assert(ped->id().empty());

    ped->setId(formatId(to_string(_numPedestrians), NUM_FIGURE_FOR_PEDESTRIAN));
    _numPedestrians++;

#ifdef PDS_DEBUG
    cerr << "                          -> [" << ped->id() << "]$(" << ped << ")"
         << (pedestrian(ped->id()) ? "found!" : "notfound") << endl;
    cerr.flush();
#endif

    // duplication check
    if (pedestrian(ped->id()))
    {
        cerr << "ERROR: pedestrian[" << ped->id()
             << "] is already added to road map." << endl;
        exit(EXIT_FAILURE);
    }
    _pedestrians.push_back(ped);
    return true;
}

//==============================================================================
void ObjectManager::deleteAllPedestrians()
{
    for (auto itr : _pedestrians)
    {
        delete itr;
    }
    _pedestrians.clear();
    _numPedestrians = 0;
}

//==============================================================================
void ObjectManager::deletePedestrian(Pedestrian* pedestrian)
{
    auto itr = find(_pedestrians.begin(), _pedestrians.end(), pedestrian);
    if (itr != _pedestrians.end())
    {
        delete *itr;
        _pedestrians.erase(itr);
    }
}

#endif //INCLUDE_PEDESTRIANS
