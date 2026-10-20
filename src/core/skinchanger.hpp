#pragma once
#include <Windows.h>
#include <thread>
#include <chrono>
#include <vector>
#include <string>
#include <map>
#include "memory/Memory.hpp"
#include "offsets/Offsets.hpp"
#include "engine/Engine.hpp"

constexpr std::ptrdiff_t m_iItemDefinitionIndex = 0x1BA;
constexpr std::ptrdiff_t m_nFallbackPaintKit = 0x1C0;
constexpr std::ptrdiff_t m_flFallbackWear = 0x1C4;
constexpr std::ptrdiff_t m_nFallbackSeed = 0x1C8;
constexpr std::ptrdiff_t m_iItemIDHigh = 0x1D0;
constexpr std::ptrdiff_t m_iAccountID = 0x1D4;
constexpr std::ptrdiff_t m_bInitialized = 0x1E8; 

struct WeaponInfo { int id; const char* name; };
inline const std::vector<WeaponInfo> g_WeaponList = {
    { 1,  "Desert Eagle" }, { 4,  "Glock-18" }, { 7,  "AK-47" }, { 9,  "AWP" },
    { 16, "M4A4" }, { 60, "M4A1-S" }, { 61, "USP-S" }
};

inline const std::map<int, std::string> g_SkinDatabase = {
    { 37, "Desert Eagle | Blaze" }, { 962, "Desert Eagle | Printstream" },
    { 44, "AK-47 | Case Hardened" }, { 282, "AK-47 | Redline" }, { 180, "AK-47 | Fire Serpent" },
    { 279, "AWP | Asiimov" }, { 344, "AWP | Dragon Lore" },
    { 984, "M4A1-S | Printstream" }, { 504, "USP-S | Kill Confirmed" }
};

inline const char* GetWeaponName(int id) { for (const auto& w : g_WeaponList) { if (w.id == id) return w.name; } return "Unknown"; }
inline const char* GetSkinName(int id) { auto it = g_SkinDatabase.find(id); if (it != g_SkinDatabase.end()) return it->second.c_str(); return "Unknown"; }

struct SkinchangerSettings { bool enabled = false; int target_weapon_id = 7; int skin_id = 44; float wear = 0.01f; int seed = 661; };
inline SkinchangerSettings g_Skinchanger;
inline uint32_t g_local_account_id = 0;

inline void SkinchangerThread(pProcess* proc) {
    while (true) {
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
        if (!g_Skinchanger.enabled) continue;

        uintptr_t clientBase = proc->base_client_.base;
        if (!clientBase) continue;

        uintptr_t localController = proc->read<uintptr_t>(clientBase + offsets::localPlayerController);
        if (!localController) continue;
        uint32_t localPawnHandle = proc->read<uint32_t>(localController + offsets::controller::m_hPawn);
        if (!localPawnHandle) continue;

        uintptr_t entityList = proc->read<uintptr_t>(clientBase + offsets::entityList);
        if (!entityList) continue;

        int localIndex = localPawnHandle & 0x7FFF;
        uintptr_t listEntryLocal = proc->read<uintptr_t>(entityList + 0x10 * (localIndex & 0x7FFF) + 0x8 * (localIndex >> 15));
        if (!listEntryLocal) continue;
        uintptr_t localPawn = proc->read<uintptr_t>(listEntryLocal + 0x10 * (localIndex & 0x1FF));
        if (!localPawn) continue;

        if (g_local_account_id == 0) g_local_account_id = proc->read<uint32_t>(localController + offsets::controller::m_steamID);

        uintptr_t weaponServices = proc->read<uintptr_t>(localPawn + offsets::pawn::m_pWeaponServices);
        if (!weaponServices) continue;
        uint32_t activeWeaponHandle = proc->read<uint32_t>(weaponServices + offsets::pawn::m_hActiveWeapon);
        if (!activeWeaponHandle) continue;

        int weaponIndex = activeWeaponHandle & 0x7FFF;
        uintptr_t listEntryWeapon = proc->read<uintptr_t>(entityList + 0x10 * (weaponIndex & 0x7FFF) + 0x8 * (weaponIndex >> 15));
        if (!listEntryWeapon) continue;
        uintptr_t weapon = proc->read<uintptr_t>(listEntryWeapon + 0x10 * (weaponIndex & 0x1FF));
        if (!weapon) continue;

        uintptr_t econItemView = weapon + offsets::pawn::m_AttributeManager + offsets::pawn::m_Item;
        uint16_t currentDefIndex = proc->read<uint16_t>(econItemView + m_iItemDefinitionIndex);
        if (currentDefIndex != g_Skinchanger.target_weapon_id) continue;

        proc->write<int>(econItemView + m_nFallbackPaintKit, g_Skinchanger.skin_id);
        proc->write<float>(econItemView + m_flFallbackWear, g_Skinchanger.wear);
        proc->write<int>(econItemView + m_nFallbackSeed, g_Skinchanger.seed);
        proc->write<int>(econItemView + m_iItemIDHigh, -1);
        proc->write<uint32_t>(econItemView + m_iAccountID, g_local_account_id);
        
        proc->write<bool>(econItemView + m_bInitialized, false);
        proc->write<bool>(econItemView + m_bInitialized, true);
    }
}
