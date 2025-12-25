/* ******************************************************************
 * Copyright 2014-2024 ADVENTURE Project
 * All Rights Reserved
 * ****************************************************************** */
/**
 * @file IntersectionBuilder_RDTable.cpp
 */
#include "IntersectionBuilder.hpp"
#include "../RelativeDirectionTable.hpp"
#include <AmuStringOperator.hpp>
#include <AmuVector.hpp>
#include <cassert>
#include <cfloat>
#include <iostream>

using namespace std;
using namespace amu::math;
using amu::string_operator::getAdjustString;
using amu::string_operator::getTokens;

//======================================================================
RelativeDirectionTable* IntersectionBuilder::_buildRDTableFromFile(
    ifstream* fin)
{
    vector<vector<string> > vecRD;
    unsigned int            numNext = _inter->numNexts();

    // 読み込み前の位置を保存する
    // Save position before loading
    ifstream::pos_type pos = fin->tellg();

    for (unsigned int i = 0; i < numNext; i++)
    {
        string         line;
        vector<string> tokens;
        if (!getTokens(fin, &line, &tokens, ','))
        {
            break;
        }

        //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
        // 不正な行の処理
        // Invalid line handling
        if (tokens.size() != numNext)
        {
            cerr << "ERROR: file(" << _inter->id() << ".txt), " << i + 1
                 << "th border must contain " << numNext
                 << " values for relative directions." << endl;
            exit(EXIT_FAILURE);
        }

        //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
        // 正常な行の処理
        // Valid line handling
        vector<string> tmpvec;
        for (unsigned int j = 0; j < tokens.size(); j++)
        {
            assert(
                tokens[j] == "s" || tokens[j] == "t" || tokens[j] == "l"
                || tokens[j] == "r");
            if (i == j)
            {
                assert(tokens[j] == "t");
            }
            tmpvec.push_back(tokens[j]);
        }
        vecRD.push_back(tmpvec);
    }

    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    // 相対方向テーブルの生成と値の代入
    // Generate relative direction table and assign values
    RelativeDirectionTableCustom* tmpRDTable
        = new RelativeDirectionTableCustom(numNext);
    for (unsigned int i = 0; i < numNext; i++)
    {
        for (unsigned int j = 0; j < numNext; j++)
        {
            tmpRDTable->setItem(i, j, _stoRD(vecRD[i][j]));
        }
    }

    // 読み込み前の位置に戻す
    // Restore position before loading
    fin->seekg(pos);
    return tmpRDTable;
}

//======================================================================
RelativeDirectionTable* IntersectionBuilder::_buildDefaultRDTable()
{
    RelativeDirectionTable* tmpRDTable = nullptr;
    unsigned int            numNext    = _inter->numNexts();

    if (numNext == 1)
    {
        tmpRDTable = new RelativeDirectionTableTerminal();
    }
    else if (numNext == 2)
    {
        tmpRDTable = new RelativeDirectionTable2Way();
    }
    else
    {
        /**
         * @todo
         * メモリ節約のため，numNext=3や4のときも可能な限り
         * テンプレを使用すべきか
         */

        /*
         * 単路数によらず角度のみで直進，右左折を決定する．両方向が
         * 直進関係のときのみ相対方向を直進とする．
         *
         * Determine whether going straight or turning left or right
         * based only on angles, regardless of the number of sections.
         * The relative direction is set "RD::STRAIGHT" only when both
         * directions are going straight.
         */
        RelativeDirectionTableCustom* tmpRDTableCustom;
        tmpRDTableCustom = new RelativeDirectionTableCustom(numNext);

        AmuVector dv[100];
        AmuVector rdv[100];
        int       straight[100];
        assert(numNext <= 100);
        for (unsigned int i = 0; i < numNext; i++)
        {
            dv[i].setPoints(
                _inter->center(), _inter->next(i)->center());
            dv[i].setZ(0);
            dv[i].normalize();
            rdv[i].setPoints(
                _inter->next(i)->center(), _inter->center());
            rdv[i].setZ(0);
            rdv[i].normalize();
            straight[i] = -1;
        }
        for (unsigned int i = 0; i < numNext; i++)
        {
            double thetaMin = DBL_MAX;
            for (unsigned int j = 0; j < numNext; j++)
            {
                double theta = dv[i].calcAngle(rdv[j]);
                if (i == j)
                {
                    tmpRDTableCustom->setItem(i, j, RD::BACK);
                }
                else if (theta > 0)
                {
                    tmpRDTableCustom->setItem(i, j, RD::RIGHT);
                }
                else
                {
                    tmpRDTableCustom->setItem(i, j, RD::LEFT);
                }

                if (i != j && thetaMin > abs(theta))
                {
                    thetaMin    = abs(theta);
                    straight[i] = j;
                }
            }
        }
        for (unsigned int i = 0; i < numNext; i++)
        {
            int j = straight[i];
            if (j != -1 && static_cast<signed int>(i) == straight[j])
            {
                tmpRDTableCustom->setItem(i, j, RD::STRAIGHT);
                tmpRDTableCustom->setItem(j, i, RD::STRAIGHT);
            }
        }

        tmpRDTable = tmpRDTableCustom;
    }

    return tmpRDTable;
}

//======================================================================
RD_t IntersectionBuilder::_stoRD(const string& str)
{
    if (str == "s")
    {
        return RD::STRAIGHT;
    }
    else if (str == "t")
    {
        return RD::BACK;
    }
    else if (str == "l")
    {
        return RD::LEFT;
    }
    else if (str == "r")
    {
        return RD::RIGHT;
    }
    else
    {
        return RD::NONE;
    }
}
