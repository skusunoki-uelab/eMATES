/* ******************************************************************
 * Copyright 2014-2024 ADVENTURE Project
 * All Rights Reserved
 * ****************************************************************** */
/**
 * @file Pedestrian.cpp
 */
#ifdef INCLUDE_PEDESTRIANS
#include "Pedestrian.hpp"
#include "Zebra.hpp"
#include "../Intersection.hpp"
#include <cassert>

using namespace std;
using namespace amu::geometry;
using namespace amu::math;

//#

//======================================================================
Pedestrian::Pedestrian() : _id()
{
    _rng.reset();

    _body.setPedestrian(this);
    _behavior.setPedestrian(this);
    _location.setPedestrian(this);
    _scene.setPedestrian(this);

    _perceiver.setPedestrian(this, &_location, &_scene);
    _determiner.setPedestrian(
        this, &_behavior, &_body, &_decision, &_location, &_perceiver,
        &_scene);
    _actor.setPedestrian(this, &_behavior, &_decision, &_location);
}

//======================================================================
Pedestrian::~Pedestrian() {}

//======================================================================
void Pedestrian::perceive()
{
    _perceiver.perceive();
}

//======================================================================
void Pedestrian::determine()
{
    _determiner.determine();
}

//======================================================================
void Pedestrian::act()
{
    _actor.act();
}

//======================================================================
bool Pedestrian::addToZebra(
    Intersection* inter, Zebra* zebra, int dir, AmuPoint position,
    AmuVector desiredDirection)
{
    _location.setIntersection(inter);
    _location.setZebra(zebra);
    _location.setIsOnZebra(true);
    _behavior.setCrossingDirection(dir);
    _location.setPosition(position);
    _behavior.setDesiredDirection(desiredDirection);
    return true;
}

//======================================================================
void Pedestrian::print(ostream& out) const
{
    stringstream ss;
    ss.str("");

    ss << "--- Pedestrian Information ---" << endl;
    ss << "ID:" << _id << endl;

#pragma omp critical(out_critical)
    out << ss.str();

    _location.print(out);
    _behavior.print(out);

    _scene.print(out);
}

#endif //INCLUDE_PEDESTRIANS
