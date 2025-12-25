#include "CSNodeNormalBuilder.hpp"
#include "../AppMates.hpp"
#include "../CSNodeNormal.hpp"

//======================================================================
CSNodeNormalBuilder::CSNodeNormalBuilder(RoadMapBuilder* roadMapBuilder) : ODNodeBuilder(roadMapBuilder)
{
    _inter = nullptr;
    _rdTable = NULL;
    _connectorIds.clear();
    _roadwayVertexes.clear();
    _borderPoints.clear();

    // デフォルト値であり上書きされうる
    // Default values, may be overwritten.
    _sidewalkWidth
        = AppMates::getGVManager()
        .getNumeric("DEFAULT_SIDEWALK_WIDTH");
    _crosswalkWidth
        = AppMates::getGVManager()
        .getNumeric("DEFAULT_CROSSWALK_WIDTH");

#ifdef INCLUDE_TRAMS
     _builderTramExt = nullptr;
#endif //INCLUDE_TRAMS
}

//======================================================================
Intersection* CSNodeNormalBuilder::build(
    const std::string& fmId, const std::string& type, RoadMap* roadMap)
{
    _inter  = new CSNodeNormal(fmId, type, roadMap);
    _bundle = _inter;

#ifdef INCLUDE_TRAMS
    _builderTramExt
        = new ODNodeBuilderTramExt(_inter);
#endif //INCLUDE_TRAMS

    return _inter;
}
