//
// Created by Jianrong Yu on 2020-09-11.
//

#include "load_save_png.hpp"
#include "read_write_chunk.hpp"
#include "PPU466.hpp"
#include "data_path.hpp"

#include <glm/glm.hpp>
#include <iostream>
#include <fstream>
#include <iomanip>
#include <unordered_map>
#include <algorithm>

using namespace std;

int main(int argc, char **argv) {
    vector<string> paths = {data_path("../data/character")+".png", data_path("../data/burnt")+".png",
                            data_path("../data/verfire")+".png",data_path("../data/horfire")+".png",
                            data_path("../data/backgroundpixel")+".png"};

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

    glm::uvec2 char_size = glm::uvec2(0);
    vector< glm::u8vec4 > char_data;

    vector<SpriteData> v;
    vector<char> strings;

    for (auto path: paths) {
        load_png(path, &char_size, &char_data, LowerLeftOrigin);

        v.emplace_back();
        auto &spr0 = v.back();
        unordered_map<string, pair<int, int>> colors;
        int b0 = 1;
        int b1 = 0;
        int c = 1;
        for (int i = 0; i < 8; i++) {
            spr0.tile.bit0[i] = 0;
            spr0.tile.bit1[i] = 0;
            for (int j = 0; j < 8; j++) {
                spr0.tile.bit0[i] = (spr0.tile.bit0[i] << 1);
                spr0.tile.bit1[i] = (spr0.tile.bit1[i] << 1);
                if (char_data[i*8+j][3] > 0x40) {
                    char_data[i*8+j][3] = 0xff;
                    vector<uint8_t> color{uint8_t(char_data[i*8+j][0]), uint8_t(char_data[i*8+j][1]), uint8_t(char_data[i*8+j][2])};
                    string s = string()+char(char_data[i*8+j][0])+char(char_data[i*8+j][1])+char(char_data[i*8+j][2]);
                    if (colors.find(s)==colors.end()) {
                        spr0.palette[c++] = char_data[i*8+j];
                        if (b0==1) spr0.tile.bit0[i]+=1;
                        if (b1==1) spr0.tile.bit1[i]+=1;
                        colors[s] = make_pair(b0, b1);
                        if (b0 == 0) {
                            b0++;
                        }
                        if (b1 == 0) {
                            b0=0;
                            b1=1;
                        }
                    } else {
                        if (colors[s].first==1) spr0.tile.bit0[i]+=1;
                        if (colors[s].second==1) spr0.tile.bit1[i]+=1;
                    }
                }
            }

            std::bitset<8> x(spr0.tile.bit0[i]);
        }
        string name = path.substr(path.rfind('/') + 1, path.rfind('.')-path.rfind('/')-1);
        spr0.name_begin = uint32_t(strings.size());
        strings.insert(strings.end(), name.begin(), name.end());
        spr0.name_end = uint32_t(strings.size());
    }
    ofstream out(data_path("sprites")+".cnk", ios::binary);
    write_chunk("str0", strings, &out);
    write_chunk("spr0", v, &out);
}