/* ******************************************************************
 * Copyright 2014-2024 ADVENTURE Project
 * All Rights Reserved
 * ****************************************************************** */
/**
 * @file PedestrianTimeSeriesWriter.hpp
 */
#ifdef INCLUDE_PEDESTRIANS
#ifndef __PEDESTRIAN_TIME_SERIES_WRITER_HPP__
#define __PEDESTRIAN_TIME_SERIES_WRITER_HPP__
#include "Pedestrian.hpp"
#include "../Config.hpp"
#include <string>
#include <vector>
#include <iostream>
#include <fstream>

#ifdef USE_ZLIB
#include <zlib.h>
#else //USE_ZLIB
#include <cstdio>
#endif //USE_ZLIB

//######################################################################
/**
 * @~japanese 歩行者の時系列データをファイルに書き込む
 * @~english* Write pedestrian time series data to file
 * @~ @ingroup IO Pedestrian PedSim 
 */
class PedestrianTimeSeriesWriter
{
public:
    PedestrianTimeSeriesWriter() {};
    ~PedestrianTimeSeriesWriter() {};

    /**
     * @~japanese 歩行者の時系列データをファイルに書き込む
     * @~english  Write pedestrian time series data to file
     */
    bool writePedestriansData();

private:
    /**
     * @~japanese 出力ファイル名を決定する
     * @~english  Decide output file name
     */
    std::vector<std::string> _decideFileName(ulint time);

#ifdef USE_ZLIB
    using FILEOUT = gzFile;
#else  //USE_ZLIB
    using FILEOUT = FILE*;
#endif //USE_ZLIB

    /**
     * @~japanese writePedestrianData()の実体
     * @~english  Substance of writePedestrianData()
     */
    bool _writePedestrianData(FILEOUT fout, Pedestrian* ped);
};

#endif //__PEDESTRIAN_TIME_SERIES_WRITER_HPP__
#endif //INCLUDE_PEDESTRIANS
