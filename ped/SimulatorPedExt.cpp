/* ******************************************************************
 * Copyright 2014-2024 ADVENTURE Project
 * All Rights Reserved
 * ****************************************************************** */
/**
 * @file SimulatorPedExt.cpp
 */
#ifdef INCLUDE_PEDESTRIANS
#include "SimulatorPedExt.hpp"
#include "PedestrianGenerator.hpp"
#include "PedestrianGeneratorBuilder.hpp"
#include "../Simulator.hpp"
#include <cassert>

using namespace std;

//======================================================================
SimulatorPedExt::SimulatorPedExt(Simulator* simulator)
{
    _simulator = simulator;
}

//======================================================================
SimulatorPedExt::~SimulatorPedExt()
{
    if (_pedestrianGenerator)
    {
        delete _pedestrianGenerator;
    }
}

//======================================================================
bool SimulatorPedExt::getReadyPedestrians()
{
    assert(_simulator->_roadMap);

    PedestrianGeneratorBuilder builder(_simulator->_roadMap);
    _pedestrianGenerator = builder.buildPedestrianGenerator();

    return true;
}

//======================================================================
void SimulatorPedExt::generatePedestrian()
{
    _pedestrianGenerator->generatePedestrians();
}

#endif //INCLUDE_PEDESTRIANS
