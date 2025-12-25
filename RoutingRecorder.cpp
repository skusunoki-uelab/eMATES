/* ******************************************************************
 * Copyright 2014-2024 ADVENTURE Project
 * All Rights Reserved
 * ****************************************************************** */
/**
 * @file RoutingRecorder.cpp
 */
#include "RoutingRecorder.hpp"
#include "RoutingNode.hpp"
#include <AmuConverter.hpp>
#include <iostream>

using namespace std;
using namespace amu::converter;

//======================================================================
void RoutingRecorder::addLog(RouterBase::NodeStatusBase* snode)
{
    RouterBase::NodeStatusBase* newNode
        = new RouterBase::NodeStatusBase;
    newNode->setId(snode->id());
    newNode->setRoutingNode(snode->routingNode());
    newNode->setPrevId(snode->prevId());
    newNode->setCost(snode->cost());
    newNode->setLabel(snode->label());
    _log.push_back(newNode);
}

//======================================================================
void RoutingRecorder::print(ostream& out) const
{
    out << "*** Router Log ***" << endl;
    int i = 0;
    for (auto itr : _log)
    {
        out << i << ": " << itr->id() << " <- " << itr->prevId() << "("
            << itr->cost() << ")/" << toUnderlying(itr->label())
            << endl;
        i++;
    }
    out << endl;
}
