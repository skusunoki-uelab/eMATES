/* **************************************************************************
 * Copyright 2014-2024 ADVENTURE Project
 * All Rights Reserved
 * ************************************************************************** */
/**
 * @file Signal.cpp
 */
#include "Signal.hpp"
#include "AppMates.hpp"
#include "CustomMessage.hpp"
#include "Intersection.hpp"
#include "TimeManager.hpp"
#include <cassert>
#include <sstream>
#include <string>

#include "TimeManager.hpp"

using namespace std;

//==============================================================================
Signal::Signal(const Intersection* inter) : _id(inter->id()), _inter(inter)
{
    _cycles.clear();
    _currentCycle  = nullptr;
    _previousCycle = nullptr;

    _currentAspect  = nullptr;
    _previousAspect = nullptr;
}

//==============================================================================
Signal::~Signal()
{
    for (auto itr : _cycles)
    {
        delete itr;
    }
    _cycles.clear();
}

//==============================================================================
void Signal::addCycle(SignalCycle* cycle)
{
    _cycles.emplace_back(cycle);
}

//==============================================================================
void Signal::makeCycleSequence()
{
    for (int i = 0; i < _cycles.size() - 1; i++)
    {
        _cycles[i]->setNextCycle(_cycles[i + 1]);
    }
}

//==============================================================================
void Signal::print(ostream& out) const
{
    ostringstream oss;
    oss << "signal[" << _id << "]";
    amu::msg::message(out, oss.str());

    for (auto itr : _cycles)
    {
        itr->print(cout);
    }
}
