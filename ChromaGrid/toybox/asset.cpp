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

void asset_manager_c::preload(uint32_t sets, progress_f progress) {
    int ids[_asset_defs.size()];
    int count = 0;
    int id = 0;
    for (auto &def : _asset_defs) {
        if ((def.sets & sets) && (_assets[id].get() == nullptr)) {
            ids[count++] = id;
        }
        id++;
    }
    for (int i = 0; i < count; i++) {
        asset(ids[i]);
        if (progress) {
            progress(i + 1, count);
        }
    }
}

void asset_manager_c::unload(uint32_t sets) {
    int id = 0;
    for (auto &def : _asset_defs) {
        if ((def.sets & ~sets) == 0) {
            _assets[id].reset();
        }
        id++;
    }
}

asset_c &asset_manager_c::asset(int id) const {
    auto &asset = _assets[id];
    if (asset.get() == nullptr) {
        asset.reset(create_asset(id, _asset_defs[id]));
    }
    return *asset;
}

void asset_manager_c::add_asset_def(int id, const asset_def_s &def) {
    assert(def.sets != 0);
    while (_asset_defs.size() <= id) {
        _asset_defs.emplace_back(asset_c::custom, 0);
    }
    _asset_defs[id] = def;
    while (_assets.size() < _asset_defs.size()) {
        _assets.emplace_back();
    }
}

int asset_manager_c::add_asset_def(const asset_def_s &def) {
    int id = _asset_defs.size();
    add_asset_def(id, def);
    return id;
}

unique_ptr_c<char> asset_manager_c::data_path(const char *file) const {
    unique_ptr_c<char> path((char *)_malloc(128));
    strstream_c str(path.get(), 128);
#ifdef __M68000__
    str << "data\\";
#endif
    str << file << ends;
    return path;
}

unique_ptr_c<char> asset_manager_c::user_path(const char *file) const {
    unique_ptr_c<char> path((char *)_malloc(128));
    strstream_c str(path.get(), 128);
#ifndef __M68000__
    str << "/tmp/";
#endif
    str << file << ends;
    return path;
}

asset_c *asset_manager_c::create_asset(int id, const asset_def_s &def) const {
    auto path = def.file ? data_path(def.file) : nullptr;
    if (def.create) {
        return def.create(*this, path.get());
    } else {
        switch (def.type) {
            case asset_c::image:
                return new image_c(path.get());
            case asset_c::tileset:
                return new tileset_c(new image_c(path.get()), size_s(16, 16));
            case asset_c::font:
                return new font_c(new image_c(path.get()), size_s(8, 8));
            case asset_c::sound:
                return new sound_c(path.get());
            case asset_c::music:
#if TOYBOX_TARGET_ATARI
                return new ymmusic_c(path.get());
#endif
            default:
                hard_assert(0);
                break;
        }
    }
    return nullptr;
}
