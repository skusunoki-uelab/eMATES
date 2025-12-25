/* ******************************************************************
 * Copyright 2014-2024 ADVENTURE Project
 * All Rights Reserved
 * ****************************************************************** */
/**
 * @file ODNodeBuilder.hpp
 */
#ifndef __OD_NODE_INTERNAL_BUILDER_HPP__
#define __OD_NODE_INTERNAL_BUILDER_HPP__
#include "IntersectionBuilder.hpp"
#include "../ODNode.hpp"
#include <string>

//######################################################################
/**
 * @~japanese ODNodeを生成し内部構造を作成する
 *
 * @note
 * IntersectionBuilderを継承し，ODNode特有の処理で上書きする
 *
 * @~english  Generate an ODNode
 *
 * @note
 * Inherit IntersectionBuilder and override with specific processing
 * for ODNodes
 *
 * @~
 * @ingroup Initialization IO RoadNetwork
 * @see IntersectionBuilder LaneBundleBuilder
 */
class ODNodeBuilder : public IntersectionBuilder
{
public:
    ODNodeBuilder(RoadMapBuilder* roadMapBuilder)
        : IntersectionBuilder(roadMapBuilder) {};
    virtual ~ODNodeBuilder() {};

    virtual Intersection* build(
        const std::string& fmId, const std::string& type,
        RoadMap* roadMap) override;

protected:
    /**
     * @~japanese レーンを生成する
     * @~english  Generate lanes
     */
    virtual bool _generateLanes() override;

    /**
     * @~japanese デフォルトルールに従って車道頂点を生成する
     * @~english  Generate roadway vertexes according to default rules
     */
    virtual bool _generateDefaultRoadwayVertexes() override;

    /**
     * @~japanese 交差点の多角形頂点を生成する
     * @~english  Generate intersection polygon vertexes
     */
    virtual bool _generateVertexes() override;

    /**
     * @~japanese 境界を生成する
     * @~english  Generate borders
     */
    virtual bool _generateBorders() override;

    /**
     * @~japanese サブセクションを生成する
     * @~english  Generate subsections
     */
    virtual bool _generateSubsections() override;
};

#endif //__OD_NODE_INTERNAL_BUILDER_HPP__
