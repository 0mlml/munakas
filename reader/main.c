#include "cs.h"
#include "process.h"
#include "config.h"

#include <string.h>
#include <stdlib.h>
#include <unistd.h>

void jsonify_player(const struct Player *player, char *json, size_t size)
{
  snprintf(
      json, size,
      "{ \"name\": \"%s\", \"health\": %d, \"armor\": %d, \"money\": %d, "
      "\"team\": %d, \"life_state\": %d, \"weapon\": \"%s\", "
      "\"color\": %d, \"position\": {\"x\": %.2f, \"y\": %.2f, \"z\": %.2f}, "
      "\"active_player\": %s }",
      player->name, player->health, player->armor, player->money, player->team,
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

int main()
{
  while (true)
  {
    const uint64_t pid = get_pid(PROCESS_NAME);
    if (pid == 0)
    {
      errorm_print("Process not found\n");
      sleep(1);
      continue;
    }

    info_print("Found process with PID: %lu\n", pid);
    ProcessHandle handle;
    if (!open_process(&handle, pid))
    {
      errorm_print("Failed to open process\n");
      sleep(1);
      continue;
    }
    info_print("Opened process with PID: %lu\n", pid);
    struct Offsets offsets;
    if (!init_offsets(&handle, &offsets))
    {
      errorm_print("Failed to initialize offsets\n");
      close(handle.memory);
      sleep(1);
      continue;
    }
    while (true)
    {
      if (!is_valid_pid(pid))
      {
        errorm_print("Process no longer exists\n");
        break;
      }
      struct Player players[64];
      int32_t player_count = get_player_list(&handle, &offsets, players, 64);
      if (player_count < 0)
      {
        errorm_print("Failed to get player list\n");
        break;
      }
      // info_print("Found %u players\n", player_count);
      char json[8192];
      jsonify_player_list(players, player_count, json, sizeof(json));

      output_print("PLAYERLIST:%s\n", json);

      usleep(REFRESH_RATE * 1000);
    }
  }
}