/* ******************************************************************
 * Copyright 2014-2024 ADVENTURE Project
 * All Rights Reserved
 * ****************************************************************** */
/**
 * @file GVReader.cpp
 */
#include "GVReader.hpp"
#include "../CustomMessage.hpp"
#include "../GVManager.hpp"
#include <AmuConverter.hpp>
#include <AmuStringOperator.hpp>
#include <algorithm>
#include <cassert>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

using namespace std;
using namespace amu::string_operator;

//======================================================================
bool GVReader::readGV(GVManager* gv)
{
    string fname = gv->getString("GV_INIT_FILE");

    ostringstream ss;
    ss << "read init file (" << gv->stripDataDir(fname) << ") ... ";

    // ファイルを読み込む
    // Load file
    ifstream fin(fname.c_str(), ios::in);
    if (!fin)
    {
        ss << "not found";
        amu::msg::status(cout, ss.str());
        return false;
    }
    amu::msg::status(cout, ss.str());

    while (fin.good())
    {
        string         line;
        vector<string> tokens;
        if (!getTokens(&fin, &line, &tokens, '='))
        {
            break;
        }

        //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
        // 不正な行の処理
        // Invalid line handling
        if (tokens.size() != 2)
        {
            cerr << "WARNING: invalid global variable format - " << line
                 << endl;
            cerr << "format is <key = value>" << endl;
            continue;
        }

        //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
        // 正常な行の処理
        // Valid line handling

        // DIRECTORYは再設定禁止
        // Do not reset DIRECTORY
        if (tokens[0].find("DIRECTORY") != string::npos && tokens[0].find("EV_COMM") == string::npos)
        {
            cerr << "directory name cannot not be reset" << endl;
            continue;
        }

        // MAX_TIME
        if (tokens[0] == "MAX_TIME")
        {
            gv->resetMaxTime(stoul(tokens[1]));
            continue;
        }

        /*
         * 数値を登録する．tokens[1]が数値でなければ，例外処理として
         * isNumeric が false に設定される．
         *
         * Register a numerical value. If tokens[1] is not numeric,
         * isNumeric is set to false as an exception.
         */
        bool isNumeric = true;
        try
        {
            double numeric = stod(tokens[1]);
            gv->resetNumeric(tokens[0], numeric);
        }
        catch (const invalid_argument& e)
        {
            isNumeric = false;
        }

        // 文字列あるいはフラグを登録する
        // Register a string or a flag
        if (!isNumeric)
        {
            string lowerStr = tokens[1];
            transform(
                lowerStr.begin(), lowerStr.end(), lowerStr.begin(),
                ::tolower);
            if (lowerStr == "true" || lowerStr == "false")
            {
                gv->resetFlag(tokens[0], (lowerStr == "true"));
            }
            else
            {
                gv->resetString(tokens[0], tokens[1]);
            }
        }
    }
    fin.close();

    ss << "done";
    amu::msg::status(cout, ss.str());
    return true;
}
