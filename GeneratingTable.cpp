/* **************************************************************************
 * Copyright 2014-2024 ADVENTURE Project
 * All Rights Reserved
 * ************************************************************************** */
/**
 * @file GeneratingTable.cpp
 */
#include "GeneratingTable.hpp"
#include "AppMates.hpp"
#include "Config.hpp"
#include "TimeManager.hpp"
#include <algorithm>
#include <fstream>
#include <cassert>
#include <cstdlib>
#include <iostream>
#include <sstream>

using namespace std;

//==============================================================================
void GeneratingTable::extractActiveGTCells(
    std::vector<const GeneratingTableCell*>& result_cells)
{
    /*
     * _table は適用開始時刻昇順でソートされているので，先頭から調査する
     *
     * The container _table is sorted in ascending order of start time of
     * application, so scan from the head.
     */
    while (_table.size() > 0)
    {
        if (_table.front()->begin() <= AppMates::getTimeManager().time())
        {
            // _table の先頭から要素を取り除き _activatedTable に追加
            // Remove the head of _table and add it to _activatedTable
            GeneratingTableCell* activatedGTCell = _table.front();
            _activatedTable.emplace_back(activatedGTCell);
            result_cells.emplace_back(activatedGTCell);
            _table.pop_front();
        }
        else
        {
            break;
        }
    }
}

//==============================================================================
void GeneratingTable::extractActiveGTCellsAllAtOnce(
    std::vector<const GeneratingTableCell*>& result_cells)
{
    while (_table.size() > 0)
    {
        GeneratingTableCell* activatedGTCell = _table.front();
        _activatedTable.emplace_back(activatedGTCell);
        result_cells.emplace_back(activatedGTCell);
        _table.pop_front();
    }
}

//==============================================================================
void GeneratingTable::getValidGTCells(
    const string& id, vector<const GeneratingTableCell*>& result_cells) const
{
    for (auto itr : _table)
    {
        if (itr->origin().compare(id) == 0)
        {
            result_cells.emplace_back(itr);
        }
    }
}

//==============================================================================
const GeneratingTableCell* GeneratingTable::validGTCell(const string& id) const
{
    for (auto itr : _table)
    {
        if (itr->origin().compare(id) == 0)
        {
            return itr;
        }
    }
    return nullptr;
}

//==============================================================================
void GeneratingTable::sortGTCells()
{
    sort(
        _table.begin(), _table.end(),
        [](const GeneratingTableCell* rl, const GeneratingTableCell* rr)
        {
            if (rl->begin() < rr->begin())
            {
                return true;
            }
            else if (rl->begin() > rr->begin())
            {
                return false;
            }
            return (rl->id() < rr->id());
        });
}

//==============================================================================
void GeneratingTable::print(ostream& out) const
{
    out << "Table Unused:" << endl;
    for (auto itr : _table)
    {
        itr->print(out);
    }
    out << "Table Activated:" << endl;
    for (auto itr : _activatedTable)
    {
        itr->print(out);
    }
}
