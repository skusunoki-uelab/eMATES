/* ******************************************************************
 * Copyright 2014-2024 ADVENTURE Project
 * All Rights Reserved
 * ****************************************************************** */
/**
 * @file RandomNumberGenerator.cpp
 */
#include "RandomNumberGenerator.hpp"
#include "AppMates.hpp"
#include "RandomSeedManager.hpp"

using namespace std;

//======================================================================
void RandomNumberGenerator::reset()
{
    if (_engine)
    {
        delete _engine;
    }
    _seed   = AppMates::getRandomSeedManager().seed();
    _engine = new RANDOM_ENGINE();
    _engine->seed(_seed);
    _numCalled = 0;
}

//======================================================================
void RandomNumberGenerator::resetForSim()
{
    if (_engine)
    {
        delete _engine;
    }
    _seed   = AppMates::getRandomSeedManager().seedForSim();
    _engine = new RANDOM_ENGINE();
    _engine->seed(_seed);
    _numCalled = 0;
}
