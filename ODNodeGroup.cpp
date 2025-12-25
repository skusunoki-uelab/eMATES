/* ******************************************************************
 * Copyright 2014-2024 ADVENTURE Project
 * All Rights Reserved
 * ****************************************************************** */
/**
 * @file ODNodeGroup.cpp
 */
#include "ODNodeGroup.hpp"
#include "CustomMessage.hpp"
#include "ODNode.hpp"
#include <iomanip>
#include <sstream>

using namespace std;

//======================================================================
void ODNodeGroup::addODSolo(ODNode* node, double weight)
{
    WeightedODSolo* solo = new WeightedODSolo(node, weight);
    _odSolos.push_back(solo);
    _totalWeight += weight;

    if (_type == GroupType::UNDEFINED)
    {
        _type = GroupType::SOLO;
    }
    else if (_type == GroupType::PAIR)
    {
        cerr << "WARNING: ODSolo added to ODPair container." << endl;
    }
}

//======================================================================
void ODNodeGroup::getODSolo(ODNode** result_node)
{
    double rnd   = _rng.uniform(0.0, _totalWeight);
    *result_node = nullptr;

    for (auto itr : _odSolos)
    {
        if (rnd <= itr->weight())
        {
            *result_node = itr->node();
            break;
        }
        rnd -= itr->weight();
    }
    if (!(*result_node))
    {
        *result_node = _odSolos[0]->node();
    }
}

//======================================================================
void ODNodeGroup::addODPair(ODNode* start, ODNode* goal, double weight)
{
    WeightedODPair* pair = new WeightedODPair(start, goal, weight);
    _odPairs.push_back(pair);
    _totalWeight += weight;

    if (_type == GroupType::UNDEFINED)
    {
        _type = GroupType::PAIR;
    }
    else if (_type == GroupType::SOLO)
    {
        cerr << "WARNING: ODPair added to ODSolo container." << endl;
    }
}

//======================================================================
void ODNodeGroup::getODPair(ODNode** result_start, ODNode** result_goal)
{
    double rnd    = _rng.uniform(0.0, _totalWeight);
    *result_start = nullptr;
    *result_goal  = nullptr;

    for (auto itr : _odPairs)
    {
        if (rnd <= itr->weight())
        {
            *result_start = itr->start();
            *result_goal  = itr->goal();
            break;
        }
        rnd -= itr->weight();
    }
    if (!(*result_start) || !(*result_goal))
    {
        *result_start = _odPairs[0]->start();
        *result_goal  = _odPairs[0]->goal();
    }
}

//======================================================================
void ODNodeGroup::print(ostream& out) const
{
    ostringstream ss;
    ss << "Group ID: " << _id << ", Group Type: ";
    switch (_type)
    {
    case GroupType::UNDEFINED:
        ss << "UNDEFINED";
        break;
    case GroupType::SOLO:
        ss << "SOLO";
        break;
    case GroupType::PAIR:
        ss << "PAIR";
        break;
    }
    ss << endl;
    ss << "Members:";
    amu::msg::message(out, ss.str());

    if (_type == GroupType::SOLO)
    {
        for (auto itr : _odSolos)
        {
            ostringstream ss2;
            ss2 << "  " << setw(9) << itr->weight() << ", "
                << itr->node()->id();
            amu::msg::message(out, ss2.str());
        }
    }
    else if (_type == GroupType::PAIR)
    {
        for (auto itr : _odPairs)
        {
            ostringstream ss2;
            ss2 << "  " << setw(9) << itr->weight() << ", "
                << itr->start()->id() << ", " << itr->goal()->id();
            amu::msg::message(out, ss2.str());
        }
    }
}
