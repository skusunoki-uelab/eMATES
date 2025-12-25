/* **************************************************************************
 * Copyright 2014-2024 ADVENTURE Project
 * All Rights Reserved
 * ************************************************************************** */
/**
 * @file FileManager.cpp
 */
#include "FileManager.hpp"
#include "AppMates.hpp"
#include "CustomMessage.hpp"
#include <cassert>
#include <cstdlib>
#include <iostream>
#include <sstream>
#include <typeinfo>

using namespace std;

//==============================================================================
FileManager::FileManager()
{
    _ofstreams.clear();
    _className = typeid(this).name();
}

//==============================================================================
ofstream* FileManager::getOFStream(const string& filename)
{
    auto itr = _ofstreams.find(filename);
    if (itr != _ofstreams.end())
    {
        /*
         * 既にファイルが開かれていた場合はそのポインタを返す
         *   出力ストリームは作成済みだがファイルが閉じられている場合は
         *   追記モードでファイルを開く．
         *
         * If the file is already open, return the pointer.
         *   If the output stream has been created but the file is
         *   closed, open the file in append mode.
         */
        if (!(*itr).second->is_open())
        {
            (*itr).second->open(filename.c_str(), ios::app);
        }
        return (*itr).second;
    }
    else
    {
        // まだ開かれていないファイルであれば新たに開く
        // If the file has not been opened yet, open it.
        ofstream* ofs = new ofstream(filename.c_str(), ios::out);
        if (!ofs->good())
        {
            ostringstream ose;
            ose << "FileManager cannot open file (" << filename << ")." << endl;
            amu::msg::error(ose.str());
            exit(EXIT_FAILURE);
        }
        _ofstreams.insert(make_pair(filename, ofs));
        return ofs;
    }
}

//==============================================================================
void FileManager::deleteAllOFStreams()
{
    for (auto itr : _ofstreams)
    {
        if (itr.second->is_open())
        {
            ostringstream oss;
            oss << "Close file (" << itr.first << ")." << endl;
            amu::msg::message(cout, oss.str());
            itr.second->close();
        }
        delete itr.second;
    }
    _ofstreams.clear();
}

//==============================================================================
#include <cerrno>
#ifdef USE_MINGW
#include <io.h>
#else // USE_MINGW
#include <sys/types.h>
#include <sys/stat.h>
#endif // USE_MINGW

//==============================================================================
bool FileManager::prepareResultDirectories()
{
    // 必要なディレクトリ
    // Required directories
    vector<string> requiredDirectories;
    requiredDirectories.emplace_back(
        AppMates::getGVManager().getString("RESULT_OUTPUT_DIRECTORY"));
    requiredDirectories.emplace_back(
        AppMates::getGVManager().getString("RESULT_TIMELINE_DIRECTORY"));
    requiredDirectories.emplace_back(
        AppMates::getGVManager().getString("RESULT_IMG_DIRECTORY"));
    requiredDirectories.emplace_back(
        AppMates::getGVManager().getString("RESULT_INSTRUMENT_DIRECTORY"));

#ifndef USE_MINGW
    // パーミッション設定
    // Permission
    mode_t permission
        = S_IRUSR | S_IRGRP | S_IXUSR | S_IXGRP | S_IWUSR | S_IWGRP;
#endif

    // ディレクトリ作成
    // Make directory
    for (auto itr : requiredDirectories)
    {
#ifndef USE_MINGW
        if (mkdir(itr.c_str(), permission) != 0)
#else
        if (mkdir(itr.c_str()) != 0)
#endif //USE_MINGW
        {
            if (errno != EEXIST)
            {
                ostringstream sse;
                sse << "Cannot make directory (" << itr << "). - errorcode("
                    << errno << ")" << endl;
                amu::msg::error(sse.str());
                exit(EXIT_FAILURE);
            }
        }
    }
    return true;
}

//==============================================================================
bool FileManager::makeDirectories(vector<string>& paths)
{
    string tmpPath;
    for (auto itr : paths)
    {
        tmpPath += itr;
    }

#ifndef USE_MINGW
    // パーミッション設定
    // Permission
    mode_t permission
        = S_IRUSR | S_IRGRP | S_IXUSR | S_IXGRP | S_IWUSR | S_IWGRP;
#endif

    // ディレクトリ作成
    // Make directory
#ifndef USE_MINGW
    if (mkdir(tmpPath.c_str(), permission) != 0)
#else
    if (mkdir(tmpPath.c_str()) != 0)
#endif //USE_MINGW
    {
        /*
         * mkdirは成功したときに0，失敗したときに-1を返し，エラーコードが
         * 変数errno(cerrnoで宣言)に格納される．man 2 mkdir を参照．
         *
         * Function mkdir() returns 0 if it succeeds, -1 if it fails, and the
         * error code is stored in the variable errno (declared in cerrno). 
         * See man 2 mkdir.
         */
        if (errno == ENOENT && paths.size() >= 2)
        {
            /*
             * tmpPathの中のどれかのディレクトリが存在しない場合には成功するまで
             * (可能な限り) 上の階層にもどる
             *
             * If any directory in tmpPath does not exist, go up (as far as
             * possible) until success
             */
            paths.pop_back();

#ifndef USE_MINGW
            if (makeDirectories(paths)
                && mkdir(tmpPath.c_str(), permission) == 0)
#else
            if (makeDirectories(paths) && mkdir(tmpPath.c_str()) == 0)
#endif
            {
                return true;
            }
        }
        else
        {
            ostringstream sse;
            sse << "Cannot make directory (" << tmpPath << "). " << endl;
            amu::msg::error(sse.str());
            exit(EXIT_FAILURE);
        }
    }

    return true;
}

//==============================================================================
bool FileManager::exists(const string& name)
{
    ifstream fin(name.c_str());
    if (!fin)
    {
        return false;
    }
    else
    {
        fin.close();
        return true;
    }
}