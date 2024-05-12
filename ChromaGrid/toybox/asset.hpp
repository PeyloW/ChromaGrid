//
//  asset.hpp
//  toybox
//
//  Created by Fredrik on 2024-04-26.
//

#ifndef asset_hpp
#define asset_hpp

#include "stream.hpp"
#include "vector.hpp"
#include "memory.hpp"


namespace toybox {

    using namespace toystd;
    
    class asset_c : nocopy_c {
    public:
        enum type_e : uint8_t {
            custom, image, tileset, font, sound, music
        };
        virtual type_e asset_type() const __pure { return custom; }
        virtual size_t memory_cost() const = 0;
    };
    
    class image_c;
    class tileset_c;
    class font_c;
    class sound_c;
    class music_c;

    class asset_manager_c : nocopy_c {
    public:
        static asset_manager_c &shared();
        static void set_shared(asset_manager_c *shared);
        
        asset_manager_c(const char *asset_defs_path);
        virtual ~asset_manager_c() {}

        typedef void(*progress_f)(int loaded, int total);
        void preload(uint32_t sets, progress_f progress = nullptr);
        void unload(uint32_t sets);
        size_t memory_cost() const;
        
        asset_c &asset(int id) const;
        
        image_c &image(int id) const { return (image_c&)(asset(id)); }
        tileset_c &tileset(int id) const { return (tileset_c&)(asset(id)); }
        font_c &font(int id) const { return (font_c&)(asset(id)); }
        sound_c &sound(int id) const { return (sound_c&)(asset(id)); }
        music_c &music(int id) const { return (music_c&)(asset(id)); }

        virtual unique_ptr_c<char> data_path(const char *file) const;
        virtual unique_ptr_c<char> user_path(const char *file) const;
    protected:
        struct asset_def_s {
            typedef asset_c*(*asset_create_f)(const asset_manager_c &manager, const char *path);
            asset_def_s(asset_c::type_e type, uint32_t sets, const char *file = nullptr, asset_create_f create = nullptr) :
                type(type), sets(sets), file(file), create(create) {}
            asset_c::type_e type;
            uint32_t sets;
            const char *file;
            asset_create_f create;
        };
        asset_manager_c();

        void add_asset_def(int id, const asset_def_s &def);
        int add_asset_def(const asset_def_s &def);
        
        virtual asset_c *create_asset(int id, const asset_def_s &def) const;
    private:
        vector_c<asset_def_s, TOYBOX_ASSET_COUNT> _asset_defs;
        mutable vector_c<unique_ptr_c<asset_c>, TOYBOX_ASSET_COUNT> _assets;
    };
    
}

#endif /* asset_hpp */
