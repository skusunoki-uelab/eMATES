/* **************************************************************************
 * Copyright 2014-2024 ADVENTURE Project
 * All Rights Reserved
 * ************************************************************************** */
/**
 * @fle SignalSplit.hpp
 */
#include "SignalSplit.hpp"
#include "AppMates.hpp"
#include "CustomMessage.hpp"
#include "ScheduleManager.hpp"
#include "Signal.hpp"
#include "TimeManager.hpp"
#include <iostream>
#include <sstream>
#include <typeinfo>

using namespace std;

//==============================================================================
SignalSplit::SignalSplit(
    const Signal* signal, const SignalCycle* cycle, unsigned int id,
    ulint duration)
    : _signal(signal), _cycle(cycle), _splitId(id), _duration(duration)
{
    _nextSplit = nullptr;

    _className = typeid(this).name();
    ostringstream oss;
    oss << "split[" << _signal->id() << ":" << _cycle->id() << ":" << _splitId
        << "] " << _duration;
    _contents = oss.str();
}

//==============================================================================
void SignalSplit::activate()
{
    const_cast<Signal*>(_signal)->setCurrentAspect(&_aspect);

    ulint nextTransitionTime = AppMates::getTimeManager().time() + _duration;
    if (nextTransitionTime < _cycle->endTime())
    {
        AppMates::getScheduleManager().addEnvironmentItem(
            nextTransitionTime, _nextSplit);
    }

    /*
     * 最初のスプリットが始まる (1つ前のサイクルが終わった)ときに交通流の
     * 巨視的諸量を更新し経路探索に反映させる．
     *
     * When the first split begins (the previous cycle ends), the macroscopic
     * quantities of traffic flow are updated and reflected in routing.
     */
    if (_splitId == 0)
    {
        Intersection* inter
            = const_cast<Intersection*>(_signal->intersection());
        inter->observeLinkFlow();
        inter->routeCacheContainer()->removeDynamicComponent();
    }
}

//==============================================================================
void SignalSplit::print(ostream& out) const
{
    ostringstream oss;
    oss << "split[" << _signal->id() << ":" << _cycle->id() << ":" << _splitId
        << "] " << _duration << " | " << _aspect;
    amu::msg::message(out, oss.str());
}
