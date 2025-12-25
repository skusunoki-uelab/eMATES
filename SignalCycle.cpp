/* **************************************************************************
 * Copyright 2014-2024 ADVENTURE Project
 * All Rights Reserved
 * ************************************************************************** */
/**
 * @fle SignalCycle.hpp
 */
#include "SignalCycle.hpp"
#include "AppMates.hpp"
#include "Config.hpp"
#include "CustomMessage.hpp"
#include "ScheduleManager.hpp"
#include "Signal.hpp"
#include "TimeManager.hpp"
#include <iostream>
#include <sstream>

using namespace std;

//==============================================================================
SignalCycle::SignalCycle(
    const Signal* signal, unsigned int id, ulint begin, ulint end)
    : _signal(signal), _cycleId(id), _beginTime(begin), _endTime(end)
{
    _splits.clear();
    _cycleLength = 0;
    _nextCycle   = nullptr;

    _className = typeid(this).name();
    ostringstream oss;
    oss << "cycle[" << signal->id() << ":" << id << "]";
    _contents = oss.str();
}

//==============================================================================
SignalCycle::~SignalCycle()
{
    for (auto itr : _splits)
    {
        delete itr;
    }
    _splits.clear();
}

//==============================================================================
void SignalCycle::activate()
{
    // Signal にサイクルを登録する
    // Register cycle to Signal
    const_cast<Signal*>(_signal)->setCurrentCycle(this);

    // 次のサイクルへの切り替え時刻を ScheduleManager に登録する
    // Register the trasition time to the next cycle to ScheduleManager
    if (_nextCycle)
    {
        AppMates::getScheduleManager().addEnvironmentItem(
            _nextCycle->beginTime(), _nextCycle);
    }

    // 最初の現示を有効化する
    // Activate first aspect
    _splits[0]->activate();
}

//==============================================================================
void SignalCycle::makeSplitLoop()
{
    _cycleLength = 0;
    for (unsigned int i = 0; i < _splits.size(); i++)
    {
        assert(_splits[i]);
        _splits[i]->setNextSplit(_splits[(i + 1) % _splits.size()]);
        _cycleLength += _splits[i]->duration();
    }
}

//==============================================================================
void SignalCycle::print(ostream& out) const
{
    ostringstream oss;
    oss << "cycle[" << _signal->id() << ":" << _cycleId << "]:" //
        << "(" << _beginTime << ", " << _endTime << ")/" << _cycleLength;
    amu::msg::message(out, oss.str());
    for (auto itr : _splits)
    {
        itr->print(out);
    }
}
