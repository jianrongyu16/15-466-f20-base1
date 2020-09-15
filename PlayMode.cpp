#include "PlayMode.hpp"

//for the GL_ERRORS() macro:
#include "gl_errors.hpp"
#include "Load.hpp"
#include "SpriteCnk.hpp"
#include "data_path.hpp"

//for glm::value_ptr() :
#include <glm/gtc/type_ptr.hpp>

#include <random>


Load<SpriteCnk> sprites(LoadTagDefault, []()->SpriteCnk const *{
    SpriteCnk const *ret = new SpriteCnk("../sprites/sprites");

    return ret;
});

PlayMode::PlayMode() {
	//TODO:
	// you *must* use an asset pipeline of some sort to generate tiles.
	// don't hardcode them like this!
	// or, at least, if you do hardcode them like this,
	//  make yourself a script that spits out the code that you paste in here
	//   and check that script into your repository.

    ppu.tile_table[0] = sprites->lookup("backgroundpixel").tile;
    ppu.tile_table[1] = sprites->lookup("verfire").tile;
    ppu.tile_table[2] = sprites->lookup("horfire").tile;
    ppu.tile_table[3] = sprites->lookup("character").tile;
    ppu.tile_table[4] = sprites->lookup("burnt").tile;

    ppu.palette_table[0] = sprites->lookup("backgroundpixel").palette;
    ppu.palette_table[1] = sprites->lookup("verfire").palette;
    ppu.palette_table[2] = sprites->lookup("horfire").palette;
    ppu.palette_table[3] = sprites->lookup("character").palette;
    ppu.palette_table[4] = sprites->lookup("burnt").palette;

    for (uint32_t y = 0; y < PPU466::BackgroundHeight; ++y) {
        for (uint32_t x = 0; x < PPU466::BackgroundWidth; ++x) {
            ppu.background[x+PPU466::BackgroundWidth*y] = 0;
        }
    }

}

PlayMode::~PlayMode() {
}

bool PlayMode::handle_event(SDL_Event const &evt, glm::uvec2 const &window_size) {

    if (evt.type == SDL_KEYUP) {
		if (evt.key.keysym.sym == SDLK_LEFT) {
			left.pressed = true;
			return true;
		} else if (evt.key.keysym.sym == SDLK_RIGHT) {
			right.pressed = true;
			return true;
		} else if (evt.key.keysym.sym == SDLK_UP) {
			up.pressed = true;
			return true;
		} else if (evt.key.keysym.sym == SDLK_DOWN) {
			down.pressed = true;
			return true;
		}
	}

	return false;
}

void PlayMode::update(float elapsed) {
    if (dead - elapsed > 0) {
        dead -= elapsed;
        return;
    } else if (dead>0) {
        player_at.y = 0;
        player_at.x = 0;
        dead = 0;
        trails.insert({0,0});
        for (uint32_t y = 0; y < PPU466::BackgroundHeight; ++y) {
            for (uint32_t x = 0; x < PPU466::BackgroundWidth; ++x) {
                ppu.background[x+PPU466::BackgroundWidth*y] = 0;
            }
        }
        time = 10;
    } else {
        time-=elapsed;
    }
    if (time <= 0) {
        fire.clear();
        dead = 2.0f;
        score = trails.size();
        best_score = std::max(score, best_score);
        std::cout<< "score: " << score << std::endl;
        std::cout<< "best score: " << best_score << std::endl;
        trails.clear();
        return;
    }

	auto reset_background = [this](){
        for (uint32_t i = 0; i < PPU466::BackgroundHeight; i++) {
            ppu.background[player_at.x/8+1+PPU466::BackgroundWidth*i] = 0;
        }
        for (uint32_t i = 0; i < PPU466::BackgroundHeight; i++) {
            ppu.background[player_at.x/8-1+PPU466::BackgroundWidth*i] = 0;
        }
        for (uint32_t i = 0; i < PPU466::BackgroundWidth; i++) {
            ppu.background[i+PPU466::BackgroundWidth*(player_at.y/8+1)] = 0;
        }
        for (uint32_t i = 0; i < PPU466::BackgroundWidth; i++) {
            ppu.background[i+PPU466::BackgroundWidth*(player_at.y/8-1)] = 0;
        }
        for (auto t:trails) {
            ppu.background[t.first/8+PPU466::BackgroundWidth/8*t.second] = int16_t(
                    3 << 8
                    | 3
            );
        }
	};

	if (left.pressed) {
        left.pressed = false;
	    if (player_at.x - 8 < 0) return;
        reset_background();
	    player_at.x -= 8.0f;
	} else if (right.pressed) {
        right.pressed = false;
        if (player_at.x + 8 >= PPU466::ScreenWidth) return;
        reset_background();
	    player_at.x += 8.0f;
	} else if (down.pressed) {
        down.pressed = false;
        if (player_at.y - 8 < 0) return;
        reset_background();
	    player_at.y -= 8.0f;
	} else if (up.pressed) {
        up.pressed = false;
        if (player_at.y + 8 >= PPU466::ScreenWidth) return;
        reset_background();
	    player_at.y += 8.0f;
	} else {
        return;
    }

    auto p = std::make_pair(int32_t(player_at.x), int32_t(player_at.y));
    if (fire.find(p)!=fire.end()) {
        fire.clear();
        dead = 2.0f;
        score = trails.size();
        best_score = std::max(score, best_score);
        std::cout<< "score: " << score << std::endl;
        std::cout<< "best score: " << best_score << std::endl;
        trails.clear();
        return;
    }
    fire.clear();
    trails.insert(p);

    int r1 = player_at.x == 0? -1:rand() % 4;
    int r2 = player_at.y == 0? -1:rand() % 4;
    if (r1 == 0 || r2 == 0){
        fire.insert(std::make_pair(int32_t(player_at.x+8), int32_t(player_at.y)));
        for (uint32_t i = 0; i < PPU466::BackgroundHeight; i++) {
            ppu.background[player_at.x/8+1+PPU466::BackgroundWidth*i] = int16_t(
                    1 << 8
                    | 1
            );
        }
    }
    if (r1 == 1 || r2 == 1){
        fire.insert(std::make_pair(int32_t(player_at.x-8), int32_t(player_at.y)));
        for (uint32_t i = 0; i < PPU466::BackgroundHeight; i++) {
            ppu.background[player_at.x/8-1+PPU466::BackgroundWidth*i] = int16_t(
                    1 << 8
                    | 1
            );
        }
    }
    if (r1 == 2 || r2 == 2){
        fire.insert(std::make_pair(int32_t(player_at.x), int32_t(player_at.y+8)));
        for (uint32_t i = 0; i < PPU466::BackgroundWidth; i++) {
            ppu.background[i+PPU466::BackgroundWidth*(player_at.y/8+1)] = int16_t(
                    2 << 8
                    | 2
            );
        }
    }
    if (r1 == 3 || r2 == 3) {
        fire.insert(std::make_pair(int32_t(player_at.x), int32_t(player_at.y-8)));
        for (uint32_t i = 0; i < PPU466::BackgroundWidth; i++) {
            ppu.background[i+PPU466::BackgroundWidth*(player_at.y/8-1)] = int16_t(
                    2 << 8
                    | 2
            );
        }
    }

	//reset button press counters:
	left.downs = 0;
	right.downs = 0;
	up.downs = 0;
	down.downs = 0;
}

void PlayMode::draw(glm::uvec2 const &drawable_size) {
	//--- set ppu state based on game state ---

	//background color will be some hsv-like fade:
	ppu.background_color = glm::u8vec4(
		0xff,
		0xff,
		0xff,
		0xff
	);

    ppu.sprites[0].x = int32_t(player_at.x);
    ppu.sprites[0].y = int32_t(player_at.y);
    ppu.sprites[0].index = dead>0?4:3;
    ppu.sprites[0].attributes = dead>0?4:3;

	//--- actually draw ---
	ppu.draw(drawable_size);
}
