#pragma once

#include <vector>
#include <unordered_map>
#include <string>

#include "pbf/pbf.hpp"

#include "mapTile.h"
#include "tileData.h"

namespace PbfParser {
    
    void extractGeometry(protobuf::message& _geomIn, int _tileExtent, std::vector<Line>& _out, const MapTile& _tile);
    
    void extractFeature(protobuf::message& _featureIn, Feature& _out, const MapTile& _tile, std::vector<std::string>& _keys, std::vector<float>& _numericValues, std::vector<std::string>& _stringValues, int _tileExtent);
    
    void extractLayer(protobuf::message& _in, Layer& _out, const MapTile& _tile);
    
    enum pbfGeomCmd {
        moveTo = 1,
        lineTo = 2,
        closePath = 7
    };
}
