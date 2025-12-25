/* **************************************************************************
 * Copyright 2014-2024 ADVENTURE Project
 * All Rights Reserved
 * ************************************************************************** */
/**
 * @file SignalAspect.cpp
 */
#include "SignalAspect.hpp"
#include <sstream>

using namespace std;
using namespace amu::converter;

//==============================================================================
bool SignalAspect::setStates(vector<State>& states)
{
    if (!_isValidSize(states.size()))
    {
        std::cerr << "ERROR: size of aspect(" << states.size()
                  << ") is invalid." << std::endl;
        exit(EXIT_FAILURE);
    }
    _states.resize(states.size());
    copy(states.begin(), states.end(), _states.begin());

    return true;
}


//==============================================================================
void SignalAspect::print() const
{
    for (unsigned int i = 0; i < _states.size(); i++)
    {
        cout << toUnderlying(_states[i].main) << "," //
             << toUnderlying(_states[i].sub) << ","  //
             << toUnderlying(_states[i].walker) << " ";
    }
}

