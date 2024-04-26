//
//  asset.cpp
//  toybox
//
//  Created by Fredrik on 2024-04-26.
//

#include "asset.hpp"
#include "image.hpp"
#include "tileset.hpp"
#include "font.hpp"
#include "audio.hpp"

using namespace toybox;

static unique_ptr_c<asset_manager_c> s_shared;

asset_manager_c &asset_manager_c::shared() {
    assert(s_shared.get());
    return *s_shared;
}

void asset_manager_c::set_shared(asset_manager_c *shared) {
    assert(s_shared.get() == nullptr);
    s_shared.reset(shared);
}

asset_manager_c::asset_manager_c(const char *asset_defs_path) {
    
}

asset_manager_c::asset_manager_c() {
    
}

void asset_manager_c::preload(uint32_t sets) {
    
}

void asset_manager_c::unload(uint32_t sets) {
    
}

shared_ptr_c<asset_c> &asset_manager_c::asset(int id) const {
    auto &asset = _assets[id];
    if (asset.get() == nullptr) {
        
    }
    return asset;
}

void asset_manager_c::add_asset_def(int id, const asset_def_s &def) {
    while (_asset_defs.size() <= id) {
        _asset_defs.emplace_back(asset_c::custom, 0);
    }
    _asset_defs[id] = def;
}

int asset_manager_c::add_asset_def(const asset_def_s &def) {
    int id = _asset_defs.size();
    add_asset_def(id, def);
    return id;
}

asset_c *asset_manager_c::create_asset(int id, const asset_def_s &def) {
    if (def.create) {
        return def.create(def);
    } else {
        switch (def.type) {
            case asset_c::image:
                return new image_c(def.path);
            case asset_c::tileset:
                return new tileset_c(new image_c(def.path), size_s(16, 16));
            case asset_c::font:
                return new font_c(new image_c(def.path), size_s(8, 8));
            case asset_c::sound:
                return new sound_c(def.path);
            case asset_c::music:
                return new music_c(def.path);
            default:
                hard_assert(0);
                break;
        }
    }
    return nullptr;
}
