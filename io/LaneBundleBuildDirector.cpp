/* ******************************************************************
 * Copyright 2014-2024 ADVENTURE Project
 * All Rights Reserved
 * ****************************************************************** */
/**
 * @file LaneBundleBuildDirector.cpp
 */
#include "LaneBundleBuildDirector.hpp"
#include "LaneBundleBuilder.hpp"
#include "RoadMapBuilder.hpp"

using namespace std;

//======================================================================
bool LaneBundleBuildDirector::createInternalStructure()
{
    for (auto itr : _builders)
    {
        itr->createInternalStructure();
    }
    return true;
}

//======================================================================
bool LaneBundleBuildDirector::setLaneConnection()
{
    for (auto itr : _builders)
    {
        if (!(itr->setLaneConnection()))
        {
            return false;
        }
    }
    return true;
}

//======================================================================
bool LaneBundleBuildDirector::setLaneCollision()
{
    for (auto itr : _builders)
    {
        if (!(itr->setLaneCollision()))
        {
            return false;
        }
    }
    return true;
}

//======================================================================
bool LaneBundleBuildDirector::createSubnetwork()
{
    for (auto itr : _builders)
    {
        if (!(itr->createSubnetwork()))
        {
            return false;
        }
    }
    return true;
}

//======================================================================
bool LaneBundleBuildDirector::assignLanesToSubLaneBundles()
{
    for (auto itr : _builders)
    {
        if (!itr->assignLanesToSubLaneBundles())
        {
            return false;
        }
    }
    return true;
}
