/* ******************************************************************
 * Copyright 2014-2024 ADVENTURE Project
 * All Rights Reserved
 * ****************************************************************** */
/**
 * @file RoutingProbability.cpp
 */
#include "RoutingProbability.hpp"

using namespace std;

//======================================================================
double RoutingProbability::probability(const VehicleType& type) const
{
    double result = 1.0;
    auto   itr    = _probabilities.find(type);
    if (itr != _probabilities.end())
    {
        result = (*itr).second;
    }
    return result;
}

//======================================================================
void RoutingProbability::addProbability(const RoutingProbability& prob)
{
    // 重複しないよう同じ要素がないか検索しながら追加する．
    // Add while searching for the same element to avoid duplication.
    for (auto itr_p : prob.probabilities())
    {
        auto itr = _probabilities.find(itr_p.first);
        if (itr == _probabilities.end())
        {
            _probabilities.insert(make_pair(itr_p.first, itr_p.second));
        }
        // 同じキーが見つかった場合はより低い確率を適用する．
        // Apply lower probability if the same key found.
        else if ((*itr).second < itr_p.second)
        {
            _probabilities[(*itr).first] = itr_p.second;
        }
    }
}

//======================================================================
void RoutingProbability::print(ostream& out) const
{
    if (!_probabilities.empty())
    {
        out << "  RoutingProbability: ";
        for (auto itr : _probabilities)
        {
            out << "(" << itr.first << ", " << itr.second << "), ";
        }
        out << endl;
    }
}
