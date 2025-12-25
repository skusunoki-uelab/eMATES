/* ******************************************************************
 * Copyright 2014-2024 ADVENTURE Project
 * All Rights Reserved
 * ****************************************************************** */
/**
 * @file Connector.hpp
 */
#ifndef __CONNECTOR_HPP__
#define __CONNECTOR_HPP__
#include <AmuPoint.hpp>
#include <string>

//######################################################################
/**
 * @~japanese レーンどうしを繋げるオブジェクト
 *
 * 座標のみを扱う AmuPoint に対し，コネクタは識別番号を持ち，ユニークで
 * あることを保証する
 *
 * @~english  Object that connects lanes 
 *
 * As opposed to an AmuPoint that has only coordinates, a connector has
 * an ID number and is guaranteed to be unique.  
 *
 * @~ @ingroup RoadNetwork
 */
class Connector
{
public:
    Connector(int id, const amu::geometry::AmuPoint& point)
        : _idGlobal(id), _point(point) {};
    ~Connector() {};

private:
    /**
     * @~japanese グローバルな識別番号
     *
     * @note
     * ローカルな識別番号はどのオブジェクトを通じてアクセスするかに
     * よって異なる
     * 
     * @~english  Global ID number
     *
     * @note
     * The local ID number depends on which object is accessed through.
     */
    const int _idGlobal;

    /**
     * @~japanese コネクタの置かれる位置
     * @~english  Position of the connector
     */
    const amu::geometry::AmuPoint _point;

    /**
     * @~japanese @name アクセッサ
     * @~english  @name Accessor
     */
    ///@{
public:
    int idGlobal() const
    {
        return _idGlobal;
    }

    const amu::geometry::AmuPoint& point() const
    {
        return _point;
    }

    double x() const
    {
        return _point.x();
    }

    double y() const
    {
        return _point.y();
    }

    double z() const
    {
        return _point.z();
    }

    ///@}
};

#endif //__CONNECTOR_HPP__
