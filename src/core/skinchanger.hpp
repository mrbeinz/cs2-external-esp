// src/core/Skinchanger.hpp
#pragma once
#include <Windows.h>
#include <thread>
#include <chrono>
#include <vector>
#include "memory/Memory.hpp"
#include "offsets/Offsets.hpp"
#include "core/engine/Engine.hpp"

// Offsets for skin-changing fields (can be updated via cs2-dumper)
constexpr std::ptrdiff_t m_iItemDefinitionIndex = 0x1BA;
constexpr std::ptrdiff_t m_nFallbackPaintKit = 0x1BC;
constexpr std::ptrdiff_t m_flFallbackWear = 0x1C0;
constexpr std::ptrdiff_t m_nFallbackSeed = 0x1C4;
constexpr std::ptrdiff_t m_iItemIDHigh = 0x1D0;
constexpr std::ptrdiff_t m_iAccountID = 0x1D4;

// Settings for the skinchanger
struct SkinchangerSettings {
    bool enabled = false;
    int target_weapon_id = 7; // AK-47
    int skin_id = 44;         // Case Hardened
    float wear = 0.01f;       // Factory New
    int seed = 0;
    int stattrak = -1;        // -1 to disable
};

inline SkinchangerSettings g_Skinchanger;
inline uint32_t g_local_account_id = 0;

// Helper to get the active weapon entity
inline uintptr_t GetActiveWeapon(pProcess* proc, uintptr_t localPawn) {
    if (!localPawn) return 0;
    uintptr_t weaponServices = proc->read<uintptr_t>(localPawn + offsets::pawn::m_pWeaponServices);
    if (!weaponServices) return 0;
    uint32_t weaponHandle = proc->read<uint32_t>(weaponServices + offsets::pawn::m_hActiveWeapon);
    if (!weaponHandle) return 0;
    
    // Resolve the handle to an entity pointer
    uint32_t index = weaponHandle & 0x7FFF;
    uintptr_t entityList = proc->read<uintptr_t>(proc->base_client_.base + offsets::entityList);
    if (!entityList) return 0;
    uintptr_t listEntry = proc->read<uintptr_t>(entityList + 0x10 * (index & 0x7FFF) + 0x8 * (index >> 15));
    if (!listEntry) return 0;
    return proc->read<uintptr_t>(listEntry + 0x10 * (index & 0x1FF));
}

// Thread that applies the skin
inline void SkinchangerThread(pProcess* proc) {
    while (true) {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        if (!g_Skinchanger.enabled) continue;

        // Get local pawn
        uintptr_t clientBase = proc->base_client_.base;
        uintptr_t localController = proc->read<uintptr_t>(clientBase + offsets::localPlayerController);
        if (!localController) continue;
        uint32_t localPawnHandle = proc->read<uint32_t>(localController + offsets::controller::m_hPawn);
        if (!localPawnHandle) continue;
        int localIndex = localPawnHandle & 0x7FFF;
        uintptr_t entityList = proc->read<uintptr_t>(clientBase + offsets::entityList);
        uintptr_t listEntryLocal = proc->read<uintptr_t>(entityList + 0x10 * (localIndex & 0x7FFF) + 0x8 * (localIndex >> 15));
        uintptr_t localPawn = proc->read<uintptr_t>(listEntryLocal + 0x10 * (localIndex & 0x1FF));
        if (!localPawn) continue;

        // Get account ID once
        if (g_local_account_id == 0) {
            g_local_account_id = proc->read<uint32_t>(localController + offsets::controller::m_steamID);
        }

        // Get active weapon and check its ID
        uintptr_t weapon = GetActiveWeapon(proc, localPawn);
        if (!weapon) continue;
        uint16_t currentDefIndex = proc->read<uint16_t>(weapon + m_iItemDefinitionIndex);
        if (currentDefIndex != g_Skinchanger.target_weapon_id) continue;

        // Apply skin values
        proc->write<int>(weapon + m_nFallbackPaintKit, g_Skinchanger.skin_id);
        proc->write<float>(weapon + m_flFallbackWear, g_Skinchanger.wear);
        proc->write<int>(weapon + m_nFallbackSeed, g_Skinchanger.seed);
        proc->write<int>(weapon + m_iItemIDHigh, -1); // Required to force update
        proc->write<uint32_t>(weapon + m_iAccountID, g_local_account_id);
    }
}
