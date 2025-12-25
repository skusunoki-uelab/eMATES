/* ******************************************************************
 * Copyright 2014-2024 ADVENTURE Project
 * All Rights Reserved
 * ****************************************************************** */
/**
 * @file VehicleGlobalRoute.cpp
 */
#include "VehicleGlobalRoute.hpp"
#include "Intersection.hpp"
#include <iostream>
#include <algorithm>
#include <cassert>

using namespace std;

//======================================================================
VehicleGlobalRoute::VehicleGlobalRoute()
{
    _route.clearIntersections();
    _lastPassedIntersectionIndex = 0;
    _lastPassedGateIndex         = 0;
    _gates.clear();
    _failedGoals.clear();
    _numRerouting = 0;
}

//======================================================================
void VehicleGlobalRoute::setLastPassedIntersectionIndex(
    const Intersection* prev, const Intersection* curr)
{
    if (!prev || _lastPassedIntersectionIndex < 0)
    {
        setLastPassedIntersectionIndex(curr);
        return;
    }

    const vector<const Intersection*>& intersections = _route.intersections();
    for (unsigned int i = _lastPassedIntersectionIndex + 1;
         i < intersections.size(); i++)
    {
        if (intersections[i - 1] == prev && intersections[i] == curr)
        {
            _lastPassedIntersectionIndex = i;
            break;
        }
    }

    // 必要に応じて最後に通過したゲートのインデックスも更新する
    // Also update index of last passed gate if necessary
    setLastPassedGateIndex(curr);
}

//======================================================================
void VehicleGlobalRoute::setLastPassedIntersectionIndex(
    const Intersection* curr)
{
    const vector<const Intersection*>& intersections = _route.intersections();
    for (unsigned int i = _lastPassedIntersectionIndex + 1;
         i < intersections.size(); i++)
    {
        if (intersections[i] == curr)
        {
            _lastPassedIntersectionIndex = i;
            break;
        }
    }

    // 必要に応じて最後に通過したゲートのインデックスも更新する
    // Also update index of last passed gate if necessary
    setLastPassedGateIndex(curr);
}

//======================================================================
const Intersection* VehicleGlobalRoute::next(
    const Intersection* prev, const Intersection* curr) const
{
    assert(prev && curr);

    unsigned int                       offset;
    const vector<const Intersection*>& intersections = _route.intersections();

    if (_lastPassedIntersectionIndex < 1)
    {
        offset = 0;
    }
    else if (
        _lastPassedIntersectionIndex < static_cast<int>(intersections.size()))
    {
        offset = _lastPassedIntersectionIndex - 1;
    }
    else
    {
        // ここに到達することはないはずだが...
        // Although it should not be reached here...
        offset = 0;
    }

    for (int i = offset; i < static_cast<int>(intersections.size()) - 2; i++)
    {
        if (intersections[i] == prev && intersections[i + 1] == curr)
        {
            return intersections[i + 2];
        }
    }

    return nullptr;
}

//======================================================================
const Intersection* VehicleGlobalRoute::next(const Intersection* curr) const
{
    assert(curr);

    unsigned int                       offset;
    const vector<const Intersection*>& intersections = _route.intersections();

    if (_lastPassedIntersectionIndex < 1)
    {
        offset = 0;
    }
    else if (
        _lastPassedIntersectionIndex < static_cast<int>(intersections.size()))
    {
        offset = _lastPassedIntersectionIndex - 1;
    }
    else
    {
        // ここに到達することはないはずだが...
        // Although it should not be reached here...
        offset = 0;
    }

    for (unsigned int i = offset; i < intersections.size() - 1; i++)
    {
        if (intersections[i] == curr)
        {
            return intersections[i + 1];
        }
    }

    return nullptr;
}

//======================================================================
void VehicleGlobalRoute::setLastPassedGateIndex(const Intersection* gate)
{
    for (unsigned int i = _lastPassedGateIndex + 1; i < _gates.size(); i++)
    {
        if (_gates[i] == gate)
        {
            _lastPassedGateIndex = i;
            return;
        }
    }
}

//======================================================================
void VehicleGlobalRoute::print(std::ostream& out) const
{
    out << "Gate : lastly passed gate index = " << _lastPassedGateIndex << endl;
    int index = 0;
    for (auto itr : _gates)
    {
        out << "\t" << itr->id();
        if (index == _lastPassedGateIndex)
        {
            out << "<<";
        }
        out << endl;
        index++;
    }

    out << "Route: lastly passed intersection index = "
        << _lastPassedIntersectionIndex << endl;
    index = 0;
    for (auto itr : _route.intersections())
    {
        out << "\t" << itr->id();
        if (index == _lastPassedIntersectionIndex)
        {
            out << "<<";
        }
        out << endl;
        index++;
    }
}
