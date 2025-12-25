/* **************************************************************************
 * Copyright 2014-2024 ADVENTURE Project
 * All Rights Reserved
 * ************************************************************************** */
/**
 * @file GVInitializer.hpp
 */
#ifndef __GV_INITIALIZER_HPP__
#define __GV_INITIALIZER_HPP__
#include <string>

//##############################################################################
/**
 * @~japanese シミュレーションに用いる定数を初期化する
 * @note このクラスは静的クラスとして扱う
 *
 * @~english  Initialize constants used for simulation
 * @note Handle this class as a static class
 *
 * @~
 * @ingroup Initialization
 * @see GVManager
 */
class GVInitializer
{
private:
    GVInitializer();
    ~GVInitializer() {};

public:
    /**
     * @~japanese 定数の初期値を与える
     * @param dataPath データディレクトリのパス
     * @param loopNum  ループ回数 (出力ディレクトリの切り替えに用いる)
     *
     * @~english　 Give constant initial values
     * @param dataPath Path to data directory
     * @param loopNum  Loop count (used for switching output directory)
     */
    static void initialize(const std::string& dataPath, unsigned int loopNum);
};

#endif //__GV_INITIALIZER_HPP__
