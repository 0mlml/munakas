#pragma once

#include "cs.h"

#include <stdbool.h>
#include <stdlib.h>

void jsonify_player(const struct Player *player, char *json, size_t size);
void jsonify_player_list(const struct Player *players, size_t count, char *json,
                         size_t size);

bool init();
bool still_connected();
char *get_player_list_json();
char *get_bomb_state_json();
char *get_map_name_string();
void cleanup_game_connection();