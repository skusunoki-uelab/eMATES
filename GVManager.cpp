/* **************************************************************************
 * Copyright 2014-2024 ADVENTURE Project
 * All Rights Reserved
 * ************************************************************************** */
/**
 * @file GVManager.cpp
 */
#include "GVManager.hpp"
#include "CustomMessage.hpp"
#include "io/GVReader.hpp"
#include <AmuConverter.hpp>
#include <AmuStringOperator.hpp>
#include <algorithm>
#include <cassert>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <sstream>
#include <typeinfo>
#include <vector>

using namespace std;
using namespace amu::string_operator;

//======================================================================
GVManager::GVManager()
{
    _maxTime = 0;
    _strings.clear();
    _numerics.clear();
    _flags.clear();
    _className = typeid(this).name();
#ifdef MANAGER_DEBUG
    ostringstream oss;
    oss << _className << " constructed." << endl;
    amu::msg::status(cout, oss.str());
#endif //MANAGER_DEBUG
}

//==============================================================================
bool GVManager::setNewString(const std::string& key, const std::string& value)
{
    // キーを探し，重複がなければ設定
    // Find the key and set if there are no duplicates
    if (!_isStringKeyFound(key))
    {
        _strings.emplace(key, value);
        return true;
    }
    else
    {
        return false;
    }
}

//==============================================================================
bool GVManager::setNewNumeric(const std::string& key, const double value)
{
    // キーを探し，重複がなければ設定
    // Find the key and set if there are no duplicates
    if (!_isNumericKeyFound(key))
    {
        _numerics.emplace(key, value);
        return true;
    }
    else
    {
        return false;
    }
}

//==============================================================================
bool GVManager::setNewNumeric(const std::string& key, const int value)
{
    return setNewNumeric(key, (double)value);
}

//==============================================================================
bool GVManager::setNewFlag(const std::string& key, const bool value)
{
    // キーを探し，重複がなければ設定
    // Find the key and set if there are no duplicates
    if (!_isFlagKeyFound(key))
    {
        _flags.emplace(key, value);
        return true;
    }
    else
    {
        return false;
    }
}

//==============================================================================
bool GVManager::resetString(const std::string& key, const std::string& value)
{
    _strings[key] = value;
    return true;
}

//==============================================================================
bool GVManager::resetNumeric(const std::string& key, const double value)
{
    _numerics[key] = value;
    return true;
}

//==============================================================================
bool GVManager::resetNumeric(const std::string& key, const int value)
{
    return resetNumeric(key, (double)value);
}

//==============================================================================
bool GVManager::resetFlag(const std::string& key, const bool value)
{
    _flags[key] = value;
    return true;
}

//==============================================================================
bool GVManager::resetMaxTime(const unsigned long maxTime)
{
    _maxTime = maxTime;
    return true;
}

//==============================================================================
bool GVManager::setVariablesFromFile()
{
    GVReader reader;
    return reader.readGV(this);
}

//==============================================================================
string GVManager::stripDataDir(const std::string& filename) const
{
    string dataDir = getString("DATA_DIRECTORY");

    if (filename.find(dataDir) != 0)
    {
        ostringstream sse;
        sse << "GVManager::stripDataDir - " << "file (" << filename
            << ") doesn't begin with" << "dataDir (" << dataDir << ")." << endl;
        amu::msg::error(sse.str());
        exit(EXIT_FAILURE);
    }

    return filename.substr(dataDir.size());
}
//==============================================================================
void GVManager::print(ostream& out)
{
    amu::msg::title(cout, "Global Variables");
    ostringstream oss;
    if (_maxTime >= 100)
    {
        oss << "MAX_TIME=" << _maxTime << endl;
    }

    // key順で表示するためにmapにコピーしソートする
    // Copy to map and sort to display in key order
    map<string, bool>   mapFlags(_flags.begin(), _flags.end());
    map<string, double> mapNumerics(_numerics.begin(), _numerics.end());
    map<string, string> mapStrings(_strings.begin(), _strings.end());

    for (auto itr : mapFlags)
    {
        oss << itr.first << "=" << (itr.second ? "true" : "false") << endl;
    }
    for (auto itr : mapNumerics)
    {
        oss << itr.first << "=" << itr.second << endl;
    }
    for (auto itr : mapStrings)
    {
        oss << itr.first << ":" << itr.second << endl;
    }
    amu::msg::message(cout, oss.str());
}

//==============================================================================
bool GVManager::_isStringKeyFound(const std::string& key) const
{
    if (_strings.find(key) != _strings.end())
    {
        return true;
    }
    return false;
}

//==============================================================================
bool GVManager::_isNumericKeyFound(const std::string& key) const
{
    if (_numerics.find(key) != _numerics.end())
    {
        return true;
    }
    return false;
}

//==============================================================================
bool GVManager::_isFlagKeyFound(const std::string& key) const
{
    if (_flags.find(key) != _flags.end())
    {
        return true;
    }
    return false;
}
