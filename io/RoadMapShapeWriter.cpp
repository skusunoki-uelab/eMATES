/* ******************************************************************
 * Copyright 2014-2024 ADVENTURE Project
 * All Rights Reserved
 * ****************************************************************** */
/**
 * @file RoadMapShapeWriter.cpp
 */
#include "RoadMapShapeWriter.hpp"
#include "../AppMates.hpp"
#include "../Border.hpp"
#include "../GVManager.hpp"
#include "../Intersection.hpp"
#include "../RoadMap.hpp"
#include "../Section.hpp"
#include <AmuLineSegment.hpp>
#include <AmuPoint.hpp>
#include <fstream>
#include <iostream>
#include <string>

using namespace std;
using namespace amu::geometry;

//======================================================================
void RoadMapShapeWriter::writeRoadMapShape(const RoadMap* roadMap) const
{
    string fnameNode
        = AppMates::getGVManager().getString("RESULT_NODE_SHAPE_FILE");
    ofstream foutNode(fnameNode.c_str(), ios::out);
    if (!foutNode)
    {
        cerr << "no intersection shape file:" << endl
             << "  " << fnameNode << endl;
    }
    else
    {
        _writeIntersectionShape(&foutNode, roadMap);
        foutNode.close();
    }

    string fnameLink
        = AppMates::getGVManager().getString("RESULT_LINK_SHAPE_FILE");
    ofstream foutLink(fnameLink.c_str(), ios::out);
    if (!foutLink)
    {
        cerr << "no section shape file:" << endl
             << "  " << fnameLink << endl;
    }
    else
    {
        _writeSectionShape(&foutLink, roadMap);
        foutLink.close();
    }
}

//======================================================================
void RoadMapShapeWriter::_writeIntersectionShape(
    ofstream* fout, const RoadMap* roadMap) const
{
    // 出力用にソートする
    // sort for output
    map<string, Intersection*> ordered(
        roadMap->intersections().begin(),
        roadMap->intersections().end());
    *fout << ordered.size() << endl;
    for (auto itr : ordered)
    {
        // 識別番号
        // ID number
        *fout << itr.second->id() << endl;

        // 中心点の座標
        // Coordinates of the center point
        *fout << itr.second->center().x() << ","
              << itr.second->center().y() << ","
              << itr.second->center().z() << endl;

        // 頂点の個数
        // Number of vertexes
        *fout << itr.second->numVertexes() << endl;

        // 各頂点の座標
        // Coordinates of each vertex
        for (auto itr_v : itr.second->vertexes())
        {
            *fout << itr_v.x() << "," << itr_v.y() << "," << itr_v.z()
                  << endl;
        }
    }
}

//======================================================================
void RoadMapShapeWriter::_writeSectionShape(
    ofstream* fout, const RoadMap* roadMap) const
{
    // 出力用にソートする
    // sort for output
    auto                  sections = roadMap->sections();
    map<string, Section*> ordered(sections.begin(), sections.end());
    *fout << ordered.size() << endl;
    for (auto itr : ordered)
    {
        // 識別番号
        // ID number
        *fout << itr.second->id() << endl;

        /*
         * 右レーン数
         *   上り方向の終点交差点からの流入レーン数
         *
         * Number of right lanes
         *   Number of inflow lanes from the end intersection in the
         *   up direction 
         */
        *fout << itr.second->numIn(1) << ",";

        /*
         * 左レーン数
         *   上り方向の始点交差点からの流入レーン数
         *
         * Number of left lanes
         *   Number of inflow lanes from the end intersection in the
         *   up direction 
         */
        *fout << itr.second->numIn(0) << endl;

        // 上り方向の始点交差点, 終点交差点の識別番号
        // ID numbers of start and end intersection in the up direction
        const Intersection* inter0 = itr.second->intersection(0);
        const Intersection* inter1 = itr.second->intersection(1);
        *fout << inter0->id() << "," << inter1->id() << endl;

        /*
         * 上り方向の始点交差点，終点交差点に信号が設置されているか
         *
         * Whether a signal is installed at the start and end
         * intersection in the up direction
         */
        *fout << (inter0->signal() ? "1" : "0") << ",";
        *fout << (inter1->signal() ? "1" : "0") << endl;

        /*
         * 道路中心線を構成する点
         *   簡単のため，サイズは2に固定する．将来は vector に対応する
         *   必要があるかもしれないが，順番に注意しなければならない．
         *
         * Points that make up the center line of the section
         *   The size is fixed at 2 for simplicity. In the future it
         *   may be needed to support vectors, but be careful with the
         *   order.
          */
        const AmuLineSegment& line0
            = inter0->border(inter0->direction(itr.second))
                  ->lineSegment();
        const AmuLineSegment& line1
            = inter1->border(inter1->direction(itr.second))
                  ->lineSegment();

        *fout << "2" << endl;
        AmuPoint tmpP;
        tmpP = line0.createInteriorPoint(1, 1);
        *fout << tmpP.x() << "," << tmpP.y() << "," << tmpP.z() << endl;
        tmpP = line1.createInteriorPoint(1, 1);
        *fout << tmpP.x() << "," << tmpP.y() << "," << tmpP.z() << endl;

        // 右境界を構成する点
        // Points that make up the right boundary
        *fout << "2" << endl;
        tmpP = line0.pointBegin();
        *fout << tmpP.x() << "," << tmpP.y() << "," << tmpP.z() << endl;
        tmpP = line1.pointEnd();
        *fout << tmpP.x() << "," << tmpP.y() << "," << tmpP.z() << endl;

        // 左境界を構成する点
        // Points that make up the left boundary
        *fout << "2" << endl;
        tmpP = line0.pointEnd();
        *fout << tmpP.x() << "," << tmpP.y() << "," << tmpP.z() << endl;
        tmpP = line1.pointBegin();
        *fout << tmpP.x() << "," << tmpP.y() << "," << tmpP.z() << endl;
    }
}
