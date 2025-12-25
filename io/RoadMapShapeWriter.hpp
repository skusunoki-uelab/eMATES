/* ******************************************************************
 * Copyright 2014-2024 ADVENTURE Project
 * All Rights Reserved
 * ****************************************************************** */
/**
 * @file RoadMapShapeWriter.hpp
 */
#ifndef __ROAD_MAP_SHAPE_WRITER_HPP__
#define __ROAD_MAP_SHAPE_WRITER_HPP__
#include <fstream>

class RoadMap;

//######################################################################
/**
 * @~japanese 地図オブジェクトの形状をファイル出力する
 * @~english  Output road map shape to file
 * @~ @ingroup IO
 */
class RoadMapShapeWriter
{
public:
    RoadMapShapeWriter() {};
    ~RoadMapShapeWriter() {};

public:
    /**
     * @~japanese 地図オブジェクト @p roadMap の形状をファイルに出力する
     * @~english  Output the shape of road map object @p roadMap to file
     */
    void writeRoadMapShape(const RoadMap* roadMap) const;

private:
    /**
     * @~japanese 交差点の形状をファイル @p fout に出力する
     * @~english  Output intersection shape to file @p fout
     */
    void _writeIntersectionShape(
        std::ofstream* fout, const RoadMap* roadMap) const;

    /**
     * @~japanese 単路部の形状をファイル @p fout に出力する
     * @~english  Output section shape to file @p fout
     */
    void _writeSectionShape(
        std::ofstream* fout, const RoadMap* roadMap) const;
};

#endif //__ROAD_MAP_SHAPE_WRITER_HPP__
