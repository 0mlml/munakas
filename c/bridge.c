#include "cs.h"
#include "bridge.h"
#include "process.h"
#include "config.h"

#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <unistd.h>

static ProcessHandle g_handle;
static struct Offsets g_offsets;
static bool g_initialized = false;

void jsonify_player(const struct Player *player, char *json, size_t size)
{
  snprintf(
      json, size,
      "{ \"name\": \"%s\", \"health\": %d, \"armor\": %d, \"money\": %d, "
      "\"team\": %d, \"life_state\": %d, \"weapon\": \"%s\", "
      "\"color\": %d, \"position\": {\"x\": %.2f, \"y\": %.2f, \"z\": %.2f}, "
      "\"eye_angles\": {\"x\": %.2f, \"y\": %.2f, \"z\": %.2f}, "
      "\"active_player\": %s, "
      "\"has_bomb\": %s, \"has_defuser\": %s, "
      "\"weapons\": [",
      player->name, player->health, player->armor, player->money, player->team,
      player->life_state, player->weapon, player->color, player->position.x,
      player->position.y, player->position.z,
      player->eye_angles.x, player->eye_angles.y, player->eye_angles.z,
      player->active_player ? "true" : "false", player->has_bomb ? "true" : "false",
      player->has_defuser ? "true" : "false");

  for (int i = 0; i < 10; ++i)
  {
    if (player->weapons[i] != NULL)
    {
      strncat(json, "\"", size - strlen(json) - 1);
      strncat(json, player->weapons[i], size - strlen(json) - 1);
      strncat(json, "\"", size - strlen(json) - 1);
      if (i < 9 && player->weapons[i + 1] != NULL)
      {
        strncat(json, ",", size - strlen(json) - 1);
      }
    }
  }
  strncat(json, "]}", size - strlen(json) - 1);
}

void jsonify_player_list(const struct Player *players, size_t count, char *json,
                         size_t size)
{
  char player_json[512];
  snprintf(json, size, "[");
  for (size_t i = 0; i < count; ++i)
  {
    jsonify_player(&players[i], player_json, sizeof(player_json));
    strncat(json, player_json, size - strlen(json) - 1);
    if (i < count - 1)
    {
      strncat(json, ",", size - strlen(json) - 1);
    }
  }
  strncat(json, "]", size - strlen(json) - 1);
}

bool init()
{
  g_initialized = false;

  uint64_t pid = get_pid(PROCESS_NAME);
  if (pid == 0)
  {
    return false;
  }

  if (!open_process(&g_handle, pid))
  {
    return false;
  }

  if (!init_offsets(&g_handle, &g_offsets))
  {
    close(g_handle.memory);
    return false;
  }

  g_initialized = true;
  return true;
}

bool still_connected()
{
  if (!g_initialized)
  {
    return init();
  }

  uint64_t pid = get_pid(PROCESS_NAME);
  if (pid == 0 || !is_valid_pid(pid))
  {
    if (g_initialized)
    {
      close(g_handle.memory);
    }
    return init();
  }

  return true;
}

#define JSON_BUFFER_SIZE 8192
char *get_player_list_json()
{
  char *json = (char *)malloc(JSON_BUFFER_SIZE * sizeof(char));

  if (!still_connected())
  {
    snprintf(json, JSON_BUFFER_SIZE, "[]");
    return json;
  }

  struct Player players[64];
  int32_t player_count = get_player_list(&g_handle, &g_offsets, players, 64);
  if (player_count < 0)
  {
    snprintf(json, JSON_BUFFER_SIZE, "[]");
    return json;
  }

  jsonify_player_list(players, player_count, json, JSON_BUFFER_SIZE);
  return json;
}

char *get_bomb_state_json()
{
  char *json = (char *)malloc(JSON_BUFFER_SIZE * sizeof(char));

  if (!still_connected())
  {
    snprintf(json, JSON_BUFFER_SIZE, "{}");
    return json;
  }

  struct Bomb bomb;
  if (!get_bomb_state(&g_handle, &g_offsets, &bomb))
  {
    snprintf(json, JSON_BUFFER_SIZE, "{}");
    return json;
  }

  snprintf(json, JSON_BUFFER_SIZE,
           "{ \"is_planted\": %s, \"position\": {\"x\": %.2f, \"y\": %.2f, "
           "\"z\": %.2f}, \"blow_time\": %.2f, \"is_being_defused\": %s, "
           "\"defuse_time\": %.2f, \"bomb_site\": %d }",
           bomb.is_planted ? "true" : "false", bomb.position.x,
           bomb.position.y, bomb.position.z, bomb.blow_time,
           bomb.is_being_defused ? "true" : "false", bomb.defuse_time,
           bomb.bomb_site);

  return json;
}

char *get_map_name_string()
{
  if (!still_connected())
  {
    return "";
  }

  char *map_name = get_map_name(&g_handle, &g_offsets);
  if (map_name == NULL)
  {
    return "";
  }

  return map_name;
}

void cleanup_game_connection()
{
  if (g_initialized)
  {
    close(g_handle.memory);
    g_initialized = false;
  }
}