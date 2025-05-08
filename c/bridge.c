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
      "\"freezetime_end_equipment_value\": %d, "
      "\"team\": %d, \"life_state\": %d, \"weapon\": \"%s\", "
      "\"color\": %d, \"position\": {\"x\": %.2f, \"y\": %.2f, \"z\": %.2f}, "
      "\"active_player\": %s }",
      player->name, player->health, player->armor, player->money, player->freezetime_end_equipment_value, player->team,
      player->life_state, player->weapon, player->color, player->position.x,
      player->position.y, player->position.z,
      player->active_player ? "true" : "false");
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

void cleanup_game_connection()
{
  if (g_initialized)
  {
    close(g_handle.memory);
    g_initialized = false;
  }
}