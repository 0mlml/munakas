#pragma once

#include "process.h"

#include <stdbool.h>
#include <stdint.h>

#define PROCESS_NAME "cs2"
#define CLIENT_LIB "libclient.so"
#define ENGINE_LIB "libengine2.so"
#define TIER0_LIB "libtier0.so"

#define ENTITY_OFFSET 0x50

typedef struct v3
{
  float x;
  float y;
  float z;
} Vec3;

enum Team
{
  TEAM_NONE = 0,
  TEAM_SPECTATOR = 1,
  TEAM_TERRORIST = 2,
  TEAM_COUNTERTERRORIST = 3,
};

struct Player
{
  char name[32];
  int32_t health;
  int32_t armor;
  int32_t money;
  uint8_t team;
  uint8_t life_state;
  char weapon[32];
  int32_t color;
  Vec3 position;
  bool active_player;
  uint16_t freezetime_end_equipment_value;
};

struct InterfaceOffsets
{
  uint64_t resource;
  uint64_t entity;
  uint64_t convar;
  uint64_t player;
};

struct DirectOffsets
{
  uint64_t local_player;
};

struct ConvarOffsets
{
  uint64_t teammates_are_enemies;
};

struct LibraryOffsets
{
  uint64_t client;
  uint64_t engine;
  uint64_t tier0;
};

struct PlayerControllerOffsets
{
  uint64_t pawn;           // pointer m_hPawn
  uint64_t name;           // string m_sSanitizedPlayerName
  uint64_t color;          // i32 m_iCompTeammateColor
  uint64_t money_services; // pointer m_pInGameMoneyServices
};

struct PawnOffsets
{
  uint64_t health;                         // i32 m_iHealth
  uint64_t armor;                          // i32 m_ArmorValue
  uint64_t team;                           // u8 m_iTeamNum
  uint64_t life_state;                     // u8 m_lifeState
  uint64_t weapon;                         // pointer m_pClippingWeapon
  uint64_t bullet_services;                // pointer m_pBulletServices
  uint64_t weapon_services;                // pointer m_pWeaponServices
  uint64_t position;                       // Vec3 m_vOldOrigin
  uint64_t observer_services;              // pointer m_pObserverServices
  uint64_t freezetime_end_equipment_value; // u16 m_unFreezetimeEndEquipmentValue
};

struct MoneyServicesOffsets
{
  uint64_t money; // i32 m_iAccount
};

struct BulletServicesOffsets
{
  uint64_t total_hits; // i32 m_totalHitsOnServer
};

struct WeaponServicesOffsets
{
  uint64_t my_weapons;    // vector m_hMyWeapons (length at 0x00, data at 0x08)
  uint64_t active_weapon; // pointer m_hActiveWeapon
};

struct ObserverServicesOffsets
{
  uint64_t target; // pointer m_hObserverTarget
};

struct Offsets
{
  struct InterfaceOffsets interfaces;
  struct DirectOffsets direct;
  struct ConvarOffsets convars;
  struct LibraryOffsets libraries;

  struct PlayerControllerOffsets player_controller;
  struct PawnOffsets pawn;
  struct MoneyServicesOffsets money_services;
  struct BulletServicesOffsets bullet_services;
  struct WeaponServicesOffsets weapon_services;
  struct ObserverServicesOffsets observer_services;
};

bool init_offsets(ProcessHandle *handle, struct Offsets *offsets);
int get_player_list(ProcessHandle *handle, const struct Offsets *offsets,
                    struct Player *players, int max_players);