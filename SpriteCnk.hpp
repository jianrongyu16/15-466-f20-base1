#pragma once

//
// Created by Jianrong Yu on 2020-09-13.
// Draws inpiration from https://github.com/15-466/15-466-f19-base1/blob/master/Sprite.hpp
//

#include "PPU466.hpp"

#include <unordered_map>
#include <string>

struct Sprite {
    PPU466::Tile tile;
    PPU466::Palette palette;
};

struct SpriteCnk {
    SpriteCnk(std::string const &filebase);
    ~SpriteCnk();

    Sprite const &lookup(std::string const &name) const;

    std::unordered_map< std::string, Sprite > sprites;
};

