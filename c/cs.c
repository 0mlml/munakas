#include "cs.h"

#include <string.h>
#include <stdlib.h>

#include "process.h"
#include "config.h"

bool have_all_offsets(const struct Offsets *offsets)
{
  return offsets->player_controller.pawn && offsets->player_controller.name &&
         offsets->player_controller.color &&
         offsets->player_controller.money_services && offsets->pawn.health &&
         offsets->pawn.armor && offsets->pawn.team &&
         offsets->pawn.life_state && offsets->pawn.weapon &&
         offsets->pawn.eye_angles && offsets->pawn.item_services &&
         offsets->pawn.bullet_services && offsets->pawn.weapon_services &&
         offsets->pawn.position && offsets->pawn.observer_services &&
         offsets->money_services.money && offsets->bullet_services.total_hits &&
         offsets->weapon_services.my_weapons &&
         offsets->weapon_services.active_weapon &&
         offsets->observer_services.target &&
         offsets->item_services.has_defuser;
}

bool init_offsets(ProcessHandle *handle, struct Offsets *offsets)
{
  if (!handle || !offsets)
  {
    return false;
  }

  memset(offsets, 0, sizeof(struct Offsets));

  uint64_t client_base_address;
  if (!get_module_base_address(handle, CLIENT_LIB, &client_base_address))
  {
    errorm_print("Failed to get client base address\n");
    return false;
  }

  uint64_t engine_base_address;
  if (!get_module_base_address(handle, ENGINE_LIB, &engine_base_address))
  {
    errorm_print("Failed to get engine base address\n");
    return false;
  }

  uint64_t tier0_base_address;
  if (!get_module_base_address(handle, TIER0_LIB, &tier0_base_address))
  {
    errorm_print("Failed to get tier0 base address\n");
    return false;
  }

  uint64_t matchmaking_base_address;
  if (!get_module_base_address(handle, MATCHMAKING_LIB,
                               &matchmaking_base_address))
  {
    errorm_print("Failed to get matchmaking base address\n");
    return false;
  }

  offsets->libraries.client = client_base_address;
  offsets->libraries.engine = engine_base_address;
  offsets->libraries.tier0 = tier0_base_address;
  offsets->libraries.matchmaking = matchmaking_base_address;

  debug_print("Client base address: 0x%lx\n", client_base_address);
  debug_print("Engine base address: 0x%lx\n", engine_base_address);
  debug_print("Tier0 base address: 0x%lx\n", tier0_base_address);
  debug_print("Matchmaking base address: 0x%lx\n", matchmaking_base_address);

  if (!get_interface_offset(handle, offsets->libraries.engine,
                            "GameResourceServiceClientV0",
                            &offsets->interfaces.resource))
  {
    errorm_print("Failed to get resource offset\n");
    return false;
  }

  const uint8_t pc_pattern[] = {0x00, 0x00, 0x00, 0x00, 0x8B, 0x10, 0x85, 0xD2, 0x0F, 0x8F};
  const bool pc_mask[] = {false, false, false, false, true, true, true, true, true, true};

  uint64_t planted_c4_address;
  if (!scan_pattern(handle, offsets->libraries.client, 10, pc_pattern, pc_mask,
                    &planted_c4_address))
  {
    errorm_print("Failed to get planted c4 offset\n");
    return false;
  }

  debug_print("Planted c4 address: 0x%lx\n", planted_c4_address);

  offsets->direct.planted_c4 =
      get_rel_address(handle, planted_c4_address, 0x00, 0x07);

  const uint8_t lp_pattern[] = {0x48, 0x83, 0x3D, 0x00, 0x00, 0x00,
                                0x00, 0x00, 0x0F, 0x95, 0xC0, 0xC3};
  const bool lp_mask[] = {true, true, true, false, false, false,
                          false, false, true, true, true, true};

  uint64_t local_player_address;
  if (!scan_pattern(handle, offsets->libraries.client, 12, lp_pattern, lp_mask,
                    &local_player_address))
  {
    errorm_print("Failed to get local player offset\n");
    return false;
  }

  debug_print("Local player address: 0x%lx\n", local_player_address);

  offsets->direct.local_player =
      get_rel_address(handle, local_player_address, 0x03, 0x08);

  const uint64_t player_address = offsets->interfaces.resource + ENTITY_OFFSET;
  offsets->interfaces.entity = read_u64(handle, player_address);
  offsets->interfaces.player = offsets->interfaces.entity + 0x10;

  const uint8_t gt_pattern[] = {0x48, 0x8D, 0x05, 0x00, 0x00, 0x00, 0x00, 0xC3, 0x0F, 0x1F, 0x84, 0x00, 0x00, 0x00, 0x00,
                                0x00, 0x48, 0x8B, 0x07};
  const bool gt_mask[] = {true, true, true, false, false, false, false, true, true, true,
                          true, false, false, false, false, false, true, true, true};

  uint64_t game_type_address;
  if (!scan_pattern(handle, offsets->libraries.matchmaking, 19, gt_pattern, gt_mask,
                    &game_type_address))
  {
    errorm_print("Failed to get game type offset\n");
    return false;
  }

  debug_print("Game type address: 0x%lx\n", game_type_address);

  offsets->direct.game_types =
      get_rel_address(handle, game_type_address, 0x03, 0x07);

  if (!get_interface_offset(handle, offsets->libraries.tier0, "VEngineCvar0",
                            &offsets->interfaces.convar))
  {
    errorm_print("Failed to get cvar offset\n");
    return false;
  }

  if (!get_convar(handle, offsets->interfaces.convar,
                  "mp_teammates_are_enemies",
                  &offsets->convars.teammates_are_enemies))
  {
    errorm_print("Failed to get teammates are enemies offset\n");
    return false;
  }

  const uint64_t base = offsets->libraries.client;
  const uint64_t section_header_offset =
      read_u64(handle, base + ELF_SECTION_HEADER_OFFSET);
  const uint16_t section_header_entry_size =
      read_u16(handle, base + ELF_SECTION_HEADER_ENTRY_SIZE);
  const uint16_t section_header_num_entries =
      read_u16(handle, base + ELF_SECTION_HEADER_NUM_ENTRIES);

  const uint64_t size = section_header_offset + (section_header_entry_size *
                                                 section_header_num_entries);

  size_t client_dump_size = 0;
  if (!dump_module(handle, base, &client_dump_size, NULL))
  {
    errorm_print("Failed to get client dump size\n");
    return false;
  }

  uint8_t *client_dump = malloc(client_dump_size);
  if (!client_dump)
  {
    errorm_print("Failed to allocate memory for client dump\n");
    return false;
  }

  if (!dump_module(handle, base, &client_dump_size, client_dump))
  {
    errorm_print("Failed to dump client module\n");
    free(client_dump);
    return false;
  }

  for (size_t i = (size - 8); i > 0; i -= 8)
  {
    uint64_t entry = (uint64_t)(client_dump + i);

    bool network_enable = false;
    uint64_t network_enable_name_pointer = *(uint64_t *)entry;

    if (network_enable_name_pointer == 0)
    {
      continue;
    }

    if (network_enable_name_pointer >= base &&
        network_enable_name_pointer <= base + size)
    {
      network_enable_name_pointer =
          *(uint64_t *)(client_dump + (network_enable_name_pointer - base));
      if (network_enable_name_pointer >= base &&
          network_enable_name_pointer <= base + size)
      {
        const char *name =
            (char *)(client_dump + (network_enable_name_pointer - base));
        if (!strcmp(name, "MNetworkEnable"))
        {
          network_enable = true;
        }
      }
    }

    uint64_t name_pointer = 0;
    if (network_enable == false)
    {
      name_pointer = *(uint64_t *)(entry);
    }
    else
    {
      name_pointer = *(uint64_t *)(entry + 0x08);
    }

    if (name_pointer < base || name_pointer > (base + size))
    {
      continue;
    }

    const char *name = (char *)(client_dump + (name_pointer - base));

    if (strcmp(name, "m_hPawn") == 0)
    {
      if (!network_enable || offsets->player_controller.pawn != 0)
      {
        continue;
      }
      const int32_t offset = *(int32_t *)(entry + 0x08 + 0x10);
      offsets->player_controller.pawn = offset;
    }
    else if (strcmp(name, "m_sSanitizedPlayerName") == 0)
    {
      if (!network_enable || offsets->player_controller.name != 0)
      {
        continue;
      }
      const int32_t offset = *(int32_t *)(entry + 0x08 + 0x10);
      offsets->player_controller.name = offset;
    }
    else if (strcmp(name, "m_iCompTeammateColor") == 0)
    {
      if (offsets->player_controller.color != 0)
      {
        continue;
      }
      const int32_t offset = *(int32_t *)(entry + 0x10);
      offsets->player_controller.color = offset;
    }
    else if (strcmp(name, "m_pInGameMoneyServices") == 0)
    {
      if (offsets->player_controller.money_services != 0)
      {
        continue;
      }
      const int32_t offset = *(int32_t *)(entry + 0x10);
      offsets->player_controller.money_services = offset;
    }
    else if (strcmp(name, "m_iHealth") == 0)
    {
      if (!network_enable || offsets->pawn.health != 0)
      {
        continue;
      }
      const int32_t offset = *(int32_t *)(entry + 0x08 + 0x10);
      offsets->pawn.health = offset;
    }
    else if (strcmp(name, "m_ArmorValue") == 0)
    {
      const int32_t offset = *(int32_t *)(entry + 0x08 + 0x10);
      if (offset <= 0 || offset > 20000)
      {
        continue;
      }
      offsets->pawn.armor = offset;
    }
    else if (strcmp(name, "m_iTeamNum") == 0)
    {
      if (!network_enable || offsets->pawn.team != 0)
      {
        continue;
      }
      const int32_t offset = *(int32_t *)(entry + 0x08 + 0x10);
      offsets->pawn.team = offset;
    }
    else if (strcmp(name, "m_lifeState") == 0)
    {
      if (!network_enable || offsets->pawn.life_state != 0)
      {
        continue;
      }
      const int32_t offset = *(int32_t *)(entry + 0x08 + 0x10);
      offsets->pawn.life_state = offset;
    }
    else if (strcmp(name, "m_pClippingWeapon") == 0)
    {
      if (offsets->pawn.weapon != 0)
      {
        continue;
      }
      const int32_t offset = *(int32_t *)(entry + 0x10);
      offsets->pawn.weapon = offset;
    }
    else if (strcmp(name, "m_pBulletServices") == 0)
    {
      if (offsets->pawn.bullet_services != 0)
      {
        continue;
      }
      const int32_t offset = *(int32_t *)(entry + 0x08);
      offsets->pawn.bullet_services = offset;
    }
    else if (strcmp(name, "m_pWeaponServices") == 0)
    {
      if (offsets->pawn.weapon_services != 0)
      {
        continue;
      }
      const int32_t offset = *(int32_t *)(entry + 0x08);
      offsets->pawn.weapon_services = offset;
    }
    else if (strcmp(name, "m_vOldOrigin") == 0)
    {
      if (offsets->pawn.position != 0)
      {
        continue;
      }
      const int32_t offset = *(int32_t *)(entry + 0x08);
      offsets->pawn.position = offset;
    }
    else if (strcmp(name, "m_pObserverServices") == 0)
    {
      if (offsets->pawn.observer_services != 0)
      {
        continue;
      }
      const int32_t offset = *(int32_t *)(entry + 0x08);
      offsets->pawn.observer_services = offset;
    }
    else if (strcmp(name, "m_iAccount") == 0)
    {
      if (offsets->money_services.money != 0)
      {
        continue;
      }
      const int32_t offset = *(int32_t *)(entry + 0x10);
      offsets->money_services.money = offset;
    }
    else if (strcmp(name, "m_totalHitsOnServer") == 0)
    {
      if (offsets->bullet_services.total_hits != 0)
      {
        continue;
      }
      const int32_t offset = *(int32_t *)(entry + 0x08);
      offsets->bullet_services.total_hits = offset;
    }
    else if (strcmp(name, "m_angEyeAngles") == 0)
    {
      if (offsets->pawn.eye_angles != 0)
      {
        continue;
      }
      const int32_t offset = *(int32_t *)(entry + 0x10);
      offsets->pawn.eye_angles = offset;
    }
    else if (strcmp(name, "m_pItemServices") == 0)
    {
      if (offsets->pawn.item_services != 0)
      {
        continue;
      }
      const int32_t offset = *(int32_t *)(entry + 0x08);
      offsets->pawn.item_services = offset;
    }
    else if (strcmp(name, "m_hMyWeapons") == 0)
    {
      if (offsets->weapon_services.my_weapons != 0)
      {
        continue;
      }
      const int32_t offset = *(int32_t *)(entry + 0x08);
      offsets->weapon_services.my_weapons = offset;
    }
    else if (strcmp(name, "m_hActiveWeapon") == 0)
    {
      if (!network_enable || offsets->weapon_services.active_weapon != 0)
      {
        continue;
      }
      const int32_t offset = *(int32_t *)(entry + 0x08 + 0x10);
      offsets->weapon_services.active_weapon = offset;
    }
    else if (strcmp(name, "m_hObserverTarget") == 0)
    {
      if (offsets->observer_services.target != 0)
      {
        continue;
      }
      const int32_t offset = *(int32_t *)(entry + 0x08);
      offsets->observer_services.target = offset;
    }
    else if (strcmp(name, "m_bHasDefuser") == 0)
    {
      if (offsets->item_services.has_defuser != 0)
      {
        continue;
      }
      const int32_t offset = *(int32_t *)(entry + 0x10);
      offsets->item_services.has_defuser = offset;
    }
    else if (strcmp(name, "m_bC4Activated"))
    {
      if (offsets->planted_c4.is_activated != 0)
      {
        continue;
      }
      const int32_t offset = *(int32_t *)(entry + 0x10);
      offsets->planted_c4.is_activated = offset;
    }
    else if (strcmp(name, "m_bBombTicking") == 0)
    {
      if (offsets->planted_c4.is_ticking != 0)
      {
        continue;
      }
      const int32_t offset = *(int32_t *)(entry + 0x10);
      offsets->planted_c4.is_ticking = offset;
    }
    else if (strcmp(name, "m_flC4Blow") == 0)
    {
      if (offsets->planted_c4.blow_time != 0)
      {
        continue;
      }
      const int32_t offset = *(int32_t *)(entry + 0x10);
      offsets->planted_c4.blow_time = offset;
    }
    else if (strcmp(name, "m_nBombSite") == 0)
    {
      if (offsets->planted_c4.bomb_site != 0)
      {
        continue;
      }
      const int32_t offset = *(int32_t *)(entry + 0x18);
      offsets->planted_c4.bomb_site = offset;
    }
    else if (strcmp(name, "m_bBeingDefused") == 0)
    {
      if (offsets->planted_c4.being_defused != 0)
      {
        continue;
      }
      const int32_t offset = *(int32_t *)(entry + 0x18);
      offsets->planted_c4.being_defused = offset;
    }
    else if (strcmp(name, "m_flDefuseCountDown") == 0)
    {
      if (offsets->planted_c4.defuse_time != 0)
      {
        continue;
      }
      const int32_t offset = *(int32_t *)(entry + 0x10);
      offsets->planted_c4.defuse_time = offset;
    }

    if (have_all_offsets(offsets))
    {
      free(client_dump);
      return true;
    }
  }

  free(client_dump);

  if (!have_all_offsets(offsets))
  {
    errorm_print("Did not find all offsets\n");
    return false;
  }

  return true;
}

uint64_t get_local_controller(ProcessHandle *handle,
                              const struct Offsets *offsets)
{
  return read_u64(handle, offsets->direct.local_player);
}

bool get_client_entity(ProcessHandle *handle, const struct Offsets *offsets,
                       int32_t index, uint64_t *result)
{
  const uint64_t v2 =
      read_u64(handle, offsets->interfaces.entity + 8 * (index >> 9) + 16);
  if (v2 == 0)
    return false;

  *result = read_u64(handle, (uint64_t)(120 * (index & 0x1FF) + v2));
  return true;
}

bool get_pawn(ProcessHandle *handle, const struct Offsets *offsets,
              uint64_t controller, uint64_t *result)
{
  const int64_t v1 =
      read_u32(handle, controller + offsets->player_controller.pawn);
  if (v1 == -1)
  {
    return false;
  }

  const uint64_t v2 = read_u64(handle, offsets->interfaces.player +
                                           8 * (((uint64_t)v1 & 0x7FFF) >> 9));
  if (v2 == 0)
  {
    return false;
  }
  *result = read_u64(handle, v2 + 120 * (v1 & 0x1FF));
  return true;
}

char *get_name(ProcessHandle *handle, const struct Offsets *offsets,
               uint64_t controller)
{
  const uint64_t name_ptr =
      read_u64(handle, controller + offsets->player_controller.name);
  return read_string(handle, name_ptr);
}

int32_t get_health(ProcessHandle *handle, const struct Offsets *offsets,
                   uint64_t pawn)
{
  const int32_t health = read_i32(handle, pawn + offsets->pawn.health);
  if (health < 0 || health > 100)
  {
    return 0;
  }
  return health;
}

int32_t get_armor(ProcessHandle *handle, const struct Offsets *offsets,
                  uint64_t pawn)
{
  const int32_t armor = read_i32(handle, pawn + offsets->pawn.armor);
  if (armor < 0 || armor > 100)
  {
    return 0;
  }
  return armor;
}

int32_t get_money(ProcessHandle *handle, const struct Offsets *offsets,
                  uint64_t controller)
{
  const uint64_t money_services =
      read_u64(handle, controller + offsets->player_controller.money_services);
  if (money_services == 0)
  {
    return 0;
  }
  return read_i32(handle, money_services + offsets->money_services.money);
}

uint8_t get_team(ProcessHandle *handle, const struct Offsets *offsets,
                 uint64_t pawn)
{
  return read_u8(handle, pawn + offsets->pawn.team);
}

uint8_t get_life_state(ProcessHandle *handle, const struct Offsets *offsets,
                       uint64_t pawn)
{
  return read_u8(handle, pawn + offsets->pawn.life_state);
}

char *get_weapon_name(ProcessHandle *handle, uint64_t weapon_instance)
{
  const uint64_t weapon_entity_identity =
      read_u64(handle, weapon_instance + 0x10);
  if (weapon_entity_identity == 0)
  {
    return strdup("unknown");
  }

  const uint64_t weapon_name_pointer =
      read_u64(handle, weapon_entity_identity + 0x20);
  if (weapon_name_pointer == 0)
  {
    return strdup("unknown");
  }

  return read_string(handle, weapon_name_pointer);
}

char *get_weapon(ProcessHandle *handle, const struct Offsets *offsets,
                 uint64_t pawn)
{
  uint64_t weapon_entity_instance =
      read_u64(handle, pawn + offsets->pawn.weapon);
  if (weapon_entity_instance == 0)
  {
    return strdup("unknown");
  }

  return get_weapon_name(handle, weapon_entity_instance);
}

int get_weapons(ProcessHandle *handle, const struct Offsets *offsets,
                char *weapons[], int max_weapons, uint64_t pawn)
{
  const uint64_t weapon_service = read_u64(handle, pawn + offsets->pawn.weapon_services);
  if (weapon_service == 0)
  {
    return 0;
  }
  const uint64_t length = read_u64(handle, weapon_service + offsets->weapon_services.my_weapons);
  const uint64_t weapon_list = read_u64(handle, weapon_service + offsets->weapon_services.my_weapons + 0x08);

  int weapon_count = 0;
  for (int i = 0; i < length && weapon_count < max_weapons; i++)
  {
    const uint64_t index = read_u32(handle, weapon_list + 0x04 * i) & 0xFFF;
    uint64_t weapon_entity_instance;
    if (!get_client_entity(handle, offsets, index, &weapon_entity_instance))
    {
      continue;
    }
    char *weapon_name = get_weapon_name(handle, weapon_entity_instance);
    if (weapon_name == NULL)
    {
      continue;
    }

    weapons[weapon_count++] = weapon_name;
  }
  return weapon_count;
}

int32_t get_total_hits(ProcessHandle *handle, const struct Offsets *offsets,
                       uint64_t pawn)
{
  const uint64_t bullet_services =
      read_u64(handle, pawn + offsets->pawn.bullet_services);
  if (bullet_services == 0)
  {
    return 0;
  }
  return read_i32(handle,
                  bullet_services + offsets->bullet_services.total_hits);
}

int32_t get_color(ProcessHandle *handle, const struct Offsets *offsets,
                  uint64_t controller)
{
  return read_i32(handle, controller + offsets->player_controller.color);
}

struct v3 get_position(ProcessHandle *handle, const struct Offsets *offsets,
                       uint64_t pawn)
{
  const uint64_t position = pawn + offsets->pawn.position;
  struct v3 vec = {read_f32(handle, position),
                   read_f32(handle, position + 0x04),
                   read_f32(handle, position + 0x08)};
  return vec;
}

struct v3 get_eye_angles(ProcessHandle *handle, const struct Offsets *offsets,
                         uint64_t pawn)
{
  const uint64_t eye_angles = pawn + offsets->pawn.eye_angles;
  struct v3 vec = {read_f32(handle, eye_angles),
                   read_f32(handle, eye_angles + 0x04),
                   read_f32(handle, eye_angles + 0x08)};
  return vec;
}

bool get_has_bomb(ProcessHandle *handle, char *weapons[], int weapon_count)
{
  for (int i = 0; i < weapon_count; i++)
  {
    if (weapons[i] && strstr(weapons[i], "c4") != NULL)
    {
      return true;
    }
  }
  return false;
}

bool get_has_defuser(ProcessHandle *handle, const struct Offsets *offsets,
                     uint64_t pawn)
{
  const uint64_t item_services =
      read_u64(handle, pawn + offsets->pawn.item_services);
  if (item_services == 0)
  {
    return false;
  }
  return read_u8(handle, item_services + offsets->item_services.has_defuser) != 0;
}

uint64_t get_spectator_target(ProcessHandle *handle,
                              const struct Offsets *offsets, uint64_t pawn)
{
  const uint64_t observer_services =
      read_u64(handle, pawn + offsets->pawn.observer_services);
  if (observer_services == 0)
  {
    return 0;
  }

  const uint32_t target =
      read_u32(handle, observer_services + offsets->observer_services.target) &
      0x7FFF;
  if (target == 0)
  {
    return 0;
  }

  const uint64_t v2 =
      read_u64(handle, offsets->interfaces.player + 8 * (target >> 9));
  if (v2 == 0)
  {
    return 0;
  }

  return read_u64(handle, v2 + 120 * (target & 0x1FF));
}

bool is_ffa(ProcessHandle *handle, const struct Offsets *offsets)
{
  return read_i32(handle, offsets->convars.teammates_are_enemies + 0x40) != 0;
}

struct Player get_player_info(ProcessHandle *handle,
                              const struct Offsets *offsets,
                              uint64_t controller)
{
  struct Player player;
  memset(&player, 0, sizeof(struct Player));

  uint64_t pawn;
  if (!get_pawn(handle, offsets, controller, &pawn))
  {
    return player;
  }

  char *name_ptr = get_name(handle, offsets, controller);
  if (name_ptr)
  {
    strncpy(player.name, name_ptr, sizeof(player.name) - 1);
    free(name_ptr);
  }

  player.health = get_health(handle, offsets, pawn);
  player.armor = get_armor(handle, offsets, pawn);
  player.money = get_money(handle, offsets, controller);
  player.team = get_team(handle, offsets, pawn);
  player.life_state = get_life_state(handle, offsets, pawn);

  char *weapon_ptr = get_weapon(handle, offsets, pawn);
  if (weapon_ptr)
  {
    strncpy(player.weapon, weapon_ptr, sizeof(player.weapon) - 1);
    free(weapon_ptr);
  }

  char *weapons[10] = {0};
  int weapon_count = get_weapons(handle, offsets, weapons, 10, pawn);
  for (int i = 0; i < weapon_count; i++)
  {
    if (weapons[i])
    {
      player.weapons[i] = weapons[i];
    }
  }
  player.has_bomb = get_has_bomb(handle, player.weapons, weapon_count);
  player.has_defuser = get_has_defuser(handle, offsets, pawn);

  player.color = get_color(handle, offsets, controller);
  player.position = get_position(handle, offsets, pawn);
  player.eye_angles = get_eye_angles(handle, offsets, pawn);
  player.active_player = false;

  return player;
}

char *get_map_name(ProcessHandle *handle, const struct Offsets *offsets)
{
  const uint64_t map_name_pointer = read_u64(handle, offsets->direct.game_types + 0x120);
  const char *map_name = read_string(handle, map_name_pointer);
  if (map_name == NULL || strlen(map_name) < 5)
  {
    return strdup("unknown");
  }
  return strdup(map_name + 4);
}

bool get_bomb_state(ProcessHandle *handle, const struct Offsets *offsets,
                    struct Bomb *bomb)
{
  const uint64_t c4_handle = read_u64(handle, offsets->direct.planted_c4);
  if (c4_handle == 0)
  {
    return false;
  }
  const uint64_t planted_c4 = read_u64(handle, c4_handle);

  bomb->entity = planted_c4;
  bomb->position = get_position(handle, offsets, planted_c4);
  bomb->is_planted = read_u8(handle, planted_c4 + offsets->planted_c4.is_activated) != 0;
  bomb->blow_time = read_f32(handle, planted_c4 + offsets->planted_c4.blow_time);
  bomb->is_being_defused = read_u8(handle, planted_c4 + offsets->planted_c4.being_defused) != 0;
  bomb->defuse_time = read_f32(handle, planted_c4 + offsets->planted_c4.defuse_time);
  bomb->bomb_site = read_u8(handle, planted_c4 + offsets->planted_c4.bomb_site);

  return true;
}

int get_player_list(ProcessHandle *handle, const struct Offsets *offsets,
                    struct Player *players, int max_players)
{
  const uint64_t local_controller = get_local_controller(handle, offsets);

  uint64_t local_pawn;
  if (!get_pawn(handle, offsets, local_controller, &local_pawn))
  {
    return 0;
  }

  struct Player local_player =
      get_player_info(handle, offsets, local_controller);
  const uint64_t spectator_target =
      get_spectator_target(handle, offsets, local_pawn);

  int player_count = 0;

  for (int32_t i = 1; i <= 64 && player_count < max_players; i++)
  {
    uint64_t controller;
    if (!get_client_entity(handle, offsets, i, &controller))
    {
      continue;
    }

    if (controller == local_controller)
    {
      continue;
    }

    uint64_t pawn;
    if (!get_pawn(handle, offsets, controller, &pawn))
    {
      continue;
    }

    struct Player player = get_player_info(handle, offsets, controller);
    if (player.team != TEAM_TERRORIST && player.team != TEAM_COUNTERTERRORIST)
    {
      continue;
    }

    if (spectator_target == pawn)
    {
      player.active_player = true;
    }

    players[player_count++] = player;
  }

  if (!spectator_target)
  {
    local_player.active_player = true;
  }

  if ((local_player.team == TEAM_TERRORIST ||
       local_player.team == TEAM_COUNTERTERRORIST) &&
      player_count < max_players)
  {
    players[player_count++] = local_player;
  }

  return player_count;
}