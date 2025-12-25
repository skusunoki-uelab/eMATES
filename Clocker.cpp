/* **************************************************************************
 * Copyright 2014-2024 ADVENTURE Project
 * All Rights Reserved
 * ************************************************************************** */
/**
 * @file Clocker.cpp
 */
#include "Clocker.hpp"
#include "CustomMessage.hpp"
#include <iostream>
#include <sstream>

using namespace std;

//==============================================================================
void Clocker::print() const
{
    ostringstream oss;
    oss << _name << "/ " << "num. called: " << _numCalled
        << ", total cpu time [s]: " << _totalCpuTime
        << ", total wallclock time [s]:" << _totalWallclockTime;
    amu::msg::message(cout, oss.str());
}
