/* **************************************************************************
 * Copyright 2014-2024 ADVENTURE Project
 * All Rights Reserved
 * ************************************************************************** */
/**
 * @file RelativeDirectionTable.cpp
 */
#include "RelativeDirectionTable.hpp"
#include <cassert>

using namespace std;

//##############################################################################
RelativeDirectionTable::~RelativeDirectionTable() {}

//##############################################################################
const RelativeDirection RelativeDirectionTableTerminal::_table[2]
    = {RD(RD::BACK), RD(RD::STRAIGHT)};

//==============================================================================
const RelativeDirection& RelativeDirectionTableTerminal::operator()(
    int i, int j) const
{
    assert(i == 0 && (j == 0 || j == 1));
    return _table[j];
}

//##############################################################################
const RelativeDirection RelativeDirectionTable2Way::_table[2]
    = {RD(RD::BACK), RD(RD::STRAIGHT)};

//==============================================================================
const RelativeDirection& RelativeDirectionTable2Way::operator()(
    int i, int j) const
{
    assert(i >= 0 && i < 2 && j >= 0 && j < 2);

    // i==j -> (i+j)%2==0, i!=j -> (i+j)%2==1
    return _table[(i + j) % 2];
}

//##############################################################################
const RelativeDirection RelativeDirectionTableTJunction::_table[3][3] = {
    {RD(RD::BACK),     RD(RD::RIGHT), RD(RD::STRAIGHT)},
    {RD(RD::LEFT),     RD(RD::BACK),  RD(RD::RIGHT)   },
    {RD(RD::STRAIGHT), RD(RD::LEFT),  RD(RD::BACK)    }
};

//==============================================================================
RelativeDirectionTableTJunction ::RelativeDirectionTableTJunction(int start)
    : _start(start)
{
    assert(0 <= start && start < 3);
}

//==============================================================================
const RelativeDirection& RelativeDirectionTableTJunction::operator()(
    int i, int j) const
{
    assert(i >= 0 && i < 3 && j > 0 && j < 3);
    i = (i + 3 - _start) % 3;
    j = (j + 3 - _start) % 3;
    return _table[i][j];
}

//##############################################################################
const RelativeDirection RelativeDirectionTableCross::_table[4]
    = {RD(RD::BACK), RD(RD::RIGHT), RD(RD::STRAIGHT), RD(RD::LEFT)};

//==============================================================================
const RelativeDirection& RelativeDirectionTableCross::operator()(
    int i, int j) const
{
    assert(0 <= i && i < 4 && 0 <= j && j < 4);

    // j=i   -> (j+4-i)%4=0 -> RD(RD::BACK
    // j=i+1 -> (j+4-i)%4=1 -> RD(RD::RIGHT
    // j=i+2 -> (j+4-i)%4=2 -> RD(RD::STRAIGHT
    // j=i+3 -> (j+4-9)%4=3 -> RD(RD::LEFT
    return _table[(j + 4 - i) % 4];
}

//##############################################################################
RelativeDirectionTableCustom::RelativeDirectionTableCustom(int size)
    : _size(size)
{
    _table.resize(size * size, RD(RD::NONE));
}

//==============================================================================
void RelativeDirectionTableCustom::setItem(int i, int j, RD_t dir)
{
    assert(0 <= i && i < _size && 0 <= j && j < _size);
    _table[i * _size + j] = dir;
}

//==============================================================================
const RelativeDirection& RelativeDirectionTableCustom::operator()(
    int i, int j) const
{
    assert(0 <= i && i < _size && 0 <= j && j < _size);
    return (_table[i * _size + j]);
}
