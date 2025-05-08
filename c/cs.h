#pragma once

#include "process.h"

#include <stdbool.h>
#include <stdint.h>
#include <float.h>

#define PROCESS_NAME "cs2"
#define CLIENT_LIB "libclient.so"
#define ENGINE_LIB "libengine2.so"
#define TIER0_LIB "libtier0.so"
#define MATCHMAKING_LIB "libmatchmaking.so"

#define ENTITY_OFFSET 0x50

typedef struct v3
{
  float x;
  float y;
  float z;
} Vec3;

typedef struct v2
{
  float x;
  float y;
} Vec2;

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
  char *weapons[32];
  bool has_bomb;
  bool has_defuser;
  int32_t color;
  Vec3 position;
  Vec3 eye_angles;
  bool active_player;
};

enum BombSite
{
  A,
  B,
};

struct Bomb
{
  uint64_t entity;

  Vec3 position;
  bool is_planted;
  float blow_time;
  bool is_being_defused;
  float defuse_time;
  uint8_t bomb_site;
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
  uint64_t game_types;
  uint64_t planted_c4;
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
  uint64_t matchmaking;
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
  uint64_t health;            // i32 m_iHealth
  uint64_t armor;             // i32 m_ArmorValue
  uint64_t team;              // u8 m_iTeamNum
  uint64_t life_state;        // u8 m_lifeState
  uint64_t weapon;            // pointer m_pClippingWeapon
  uint64_t eye_angles;        // Vec3 m_angEyeAngles
  uint64_t bullet_services;   // pointer m_pBulletServices
  uint64_t weapon_services;   // pointer m_pWeaponServices
  uint64_t position;          // Vec3 m_vOldOrigin
  uint64_t observer_services; // pointer m_pObserverServices
  uint64_t item_services;     // pointer m_pItemServices
};

struct PlantedC4Offsets
{
  uint64_t is_activated;  // bool m_bC4Activated
  uint64_t is_ticking;    // bool m_bBombTicking
  uint64_t blow_time;     // float m_flC4Blow
  uint64_t bomb_site;     // i32 m_nBombSite
  uint64_t being_defused; // bool m_bBeingDefused
  uint64_t defuse_time;   // float m_flDefuseCountDown
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

struct ItemServicesOffsets
{
  uint64_t has_defuser; // bool m_bHasDefuser
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
  struct ItemServicesOffsets item_services;
  struct PlantedC4Offsets planted_c4;
};

bool init_offsets(ProcessHandle *handle, struct Offsets *offsets);
int get_player_list(ProcessHandle *handle, const struct Offsets *offsets,
                    struct Player *players, int max_players);
bool get_bomb_state(ProcessHandle *handle, const struct Offsets *offsets,
                    struct Bomb *bomb);
char *get_map_name(ProcessHandle *handle, const struct Offsets *offsets);