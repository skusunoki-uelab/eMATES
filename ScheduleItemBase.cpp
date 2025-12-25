/* **************************************************************************
 * Copyright 2014-2024 ADVENTURE Project
 * All Rights Reserved
 * ************************************************************************** */
/**
 * @file ScheduleItemBase.cpp
 */
#include "ScheduleItemBase.hpp"
#include "CustomMessage.hpp"
#include <sstream>

using namespace std;

//==============================================================================
void ScheduleItemBase::print(ostream& out) const
{
    ostringstream oss;
    oss << "ScheduleItem(" << _className << ")";
    if (!_contents.empty())
    {
        oss << ": " << _contents;
    }

    amu::msg::message(out, oss.str());
}
