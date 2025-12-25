/* ******************************************************************
 * Copyright 2014-2024 ADVENTURE Project
 * All Rights Reserved
 * ****************************************************************** */
/**
 * @file VehicleTimeSeriesWriter.hpp
 */
#ifndef __VEHICLE_TIME_SERIES_WRITER_HPP__
#define __VEHICLE_TIME_SERIES_WRITER_HPP__
#include "../Config.hpp"
#include "../Vehicle.hpp"
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
 * @~japanese 車両の時系列データをファイルに書き込む
 * @~english  Write vehicle historical data to file
 * @~ @ingroup IO Vehicle 
 */
class VehicleTimeSeriesWriter
{
public:
    VehicleTimeSeriesWriter() {};
    ~VehicleTimeSeriesWriter() {};

    /**
     * @~japanese 車両の時系列データをファイルに書き込む
     * @~english  Write vehicle historical data to file
     */
    bool writeVehiclesData();

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
     * @~japanese writeVehiclesData()の実体
     * @~english  Substance of writeVehiclesData()
     */
    bool _writeVehicleData(FILEOUT fout, Vehicle* vehicle);
};

#endif //__VEHICLE_TIME_SERIES_WRITER_HPP__
