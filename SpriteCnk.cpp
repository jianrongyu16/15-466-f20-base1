//
// Created by Jianrong Yu on 2020-09-13.
//

#include "SpriteCnk.hpp"

#include "read_write_chunk.hpp"
#include "load_save_png.hpp"
#include "data_path.hpp"

#include <fstream>

//using namespace std;

SpriteCnk::SpriteCnk(const std::string &filebase) {
    struct SpriteData {
        uint32_t name_begin, name_end;
        PPU466::Tile tile;
        PPU466::Palette palette = {
                glm::u8vec4(0x00, 0x00, 0x00, 0x00),
                glm::u8vec4(0x00, 0x00, 0x00, 0x00),
                glm::u8vec4(0x00, 0x00, 0x00, 0x00),
                glm::u8vec4(0x00, 0x00, 0x00, 0x00),
        };
    };

    std::ifstream in(data_path(filebase)+".cnk", std::ios::binary);
    std::vector< SpriteData > datas;
    std::vector< char > strs;
    read_chunk(in, "str0", &strs);
    read_chunk(in, "spr0", &datas);
    sprites.reserve(datas.size());
    for (auto data: datas) {
        Sprite sp;
        sp.tile = data.tile;
        sp.palette = data.palette;
        std::string name(strs.begin()+data.name_begin, strs.begin()+data.name_end);
        sprites.insert(std::make_pair(name, sp));
    }
}

Sprite const &SpriteCnk::lookup(std::string const &name) const {
    auto f = sprites.find(name);
    return f->second;
}