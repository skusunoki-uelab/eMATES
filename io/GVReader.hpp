/* ******************************************************************
 * Copyright 2014-2024 ADVENTURE Project
 * All Rights Reserved
 * ****************************************************************** */
/**
 * @file GVReader.hpp
 */
#ifndef __GV_READER_HPP__
#define __GV_READER_HPP__
#include "../GVManager.hpp"

//######################################################################
/**
 * @~japanese 
 * GVManagerのうちファイル入力を担当する
 *
 * @~english
 * Responsible for file input of GVManager
 *
 * @~
 * @ingroup IO Initialization
 * @see     GVManager
 */
class GVReader
{
public:
    GVReader() {};
    ~GVReader() {};

    /**
     * @~japanese グローバル変数をファイルから読み込む
     * @~english  Read global variables from file
     */
    bool readGV(GVManager* gv);
};

#endif //__GV_READER_HPP__
