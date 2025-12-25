/* ******************************************************************
 * Copyright 2014-2024 ADVENTURE Project
 * All Rights Reserved
 * ****************************************************************** */
/**
 * @file ObjectManager.cpp
 */
#ifdef INCLUDE_TRAMS

#include "VehicleTram.hpp"
#include "../ObjectManager.hpp"

using namespace std;

//======================================================================
VehicleTram* ObjectManager::createTram()
{
    VehicleTram* tmpTram = new VehicleTram();
    return tmpTram;
}

#endif //INCLUDE_TRAMS
