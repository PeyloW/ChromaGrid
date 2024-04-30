//
//  tileset.cpp
//  toybox
//
//  Created by Fredrik on 2024-04-17.
//

#include "tileset.hpp"

using namespace toybox;


tileset_c::tileset_c(const shared_ptr_c<image_c> &image, size_s tile_size) :
    _image(image),
    _max_tile(image->size().width / tile_size.width, image->size().height / tile_size.height),
    _rects()
{
    assert(_max_tile.x > 0 && _max_tile.y > 0);
    _rects.reset((rect_s*)_malloc(sizeof(rect_s) * max_index()));
    int i = 0;
    for (int y = 0; y < _max_tile.y; y++) {
        for (int x = 0; x < _max_tile.x; x++) {
            _rects[i] = rect_s(
                x * tile_size.width, y * tile_size.height,
                tile_size.width, tile_size.height
            );
            i++;
        }
    }
}

int16_t tileset_c::max_index() const {
    return _max_tile.x * _max_tile.y;
};
point_s tileset_c::max_tile() const {
    return _max_tile;
}

const rect_s &tileset_c::tile_rect(const int16_t i) const {
    assert(i >= 0 && i < max_index());
    return _rects[i];
}
const rect_s &tileset_c::tile_rect(const point_s tile) const {
    assert(tile.x >= 0 && tile.x < _max_tile.x && tile.y >= 0 && tile.y < _max_tile.y);
    return _rects[tile.x + _max_tile.x * tile.y];
}
