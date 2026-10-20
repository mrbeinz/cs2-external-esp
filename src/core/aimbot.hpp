// src/core/Aimbot.hpp
#pragma once
#include <Windows.h>
#include <thread>
#include <chrono>
#include <cmath>

#include "memory/Memory.hpp"
#include "offsets/Offsets.hpp"

// -- ADDED MISSING OFFSETS FOR AIMBOT --
constexpr std::ptrdiff_t m_angEyeAngles = 0x1518;
constexpr std::ptrdiff_t m_vecViewOffset = 0x120; // Eye height

struct Vec3 { float x, y, z; };

struct AimbotSettings {
    bool  enabled       = false;
    bool  teamCheck     = true;
    float fov           = 15.0f;
    float smooth        = 1.5f;
    int   bone          = 6; // 6 = Head
    int   hotkey        = VK_XBUTTON2; // Mouse 5
};

inline AimbotSettings g_Aimbot;

// Math for CS2 angles
inline Vec3 CalcAngle(const Vec3& src, const Vec3& dst) {
    Vec3 delta = { dst.x - src.x, dst.y - src.y, dst.z - src.z };
    float hyp = std::sqrt(delta.x * delta.x + delta.y * delta.y);
    Vec3 ang;
    ang.x = std::atan2(-delta.z, hyp) * 57.295779513f;
    ang.y = std::atan2(delta.y, delta.x) * 57.295779513f;
    ang.z = 0.0f;
    return ang;
}

inline void NormalizeAngles(Vec3& a) {
    while (a.x > 89.f)  a.x -= 180.f;
    while (a.x < -89.f) a.x += 180.f;
    while (a.y > 180.f) a.y -= 360.f;
    while (a.y < -180.f) a.y += 360.f;
}

// Pass your pProcess instance into the thread
inline void AimbotThread(pProcess* proc, uintptr_t clientBase) {
    while (true) {
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
        
        // If aimbot is off or hotkey not pressed, skip
        if (!g_Aimbot.enabled) continue;
        if (!(GetAsyncKeyState(g_Aimbot.hotkey) & 0x8000)) continue;
        if (!clientBase) continue;

        // 1. Get Local Player Controller -> Local Pawn
        uintptr_t localController = proc->read<uintptr_t>(clientBase + offsets::localPlayerController);
        if (!localController) continue;
        
        uint32_t localPawnHandle = proc->read<uint32_t>(localController + offsets::controller::m_hPawn);
        if (!localPawnHandle) continue;

        // Entity list resolution
        uintptr_t entityList = proc->read<uintptr_t>(clientBase + offsets::entityList);
        if (!entityList) continue;

        // Resolve local pawn from handle
        int localIndex = localPawnHandle & 0x7FFF;
        uintptr_t listEntryLocal = proc->read<uintptr_t>(entityList + 0x10 * (localIndex & 0x7FFF) + 0x8 * (localIndex >> 15));
        if (!listEntryLocal) continue;
        uintptr_t localPawn = proc->read<uintptr_t>(listEntryLocal + 0x10 * (localIndex & 0x1FF));
        if (!localPawn) continue;

        int localTeam = proc->read<int>(localPawn + offsets::pawn::m_iTeamNum);
        
        // Get Eye Position and Current Angles
        Vec3 origin = proc->read<Vec3>(localPawn + offsets::pawn::m_vOldOrigin);
        Vec3 viewOffset = proc->read<Vec3>(localPawn + m_vecViewOffset);
        Vec3 eyePos = { origin.x + viewOffset.x, origin.y + viewOffset.y, origin.z + viewOffset.z };
        
        Vec3 curAng = proc->read<Vec3>(localPawn + m_angEyeAngles);

        uintptr_t bestPawn = 0;
        Vec3 bestAngle{};
        float bestFov = g_Aimbot.fov;

        // 2. Loop through entities (Max 64 players in CS2)
        for (int i = 1; i <= 64; ++i) {
            uintptr_t listEntry = proc->read<uintptr_t>(entityList + 0x10 * (i & 0x7FFF) + 0x8 * (i >> 15));
            if (!listEntry) continue;
            
            uintptr_t pawn = proc->read<uintptr_t>(listEntry + 0x10 * (i & 0x1FF));
            if (!pawn || pawn == localPawn) continue;

            int hp = proc->read<int>(pawn + offsets::pawn::m_iHealth);
            if (hp <= 0 || hp > 100) continue;

            int team = proc->read<int>(pawn + offsets::pawn::m_iTeamNum);
            if (g_Aimbot.teamCheck && team == localTeam) continue;

            // 3. Get Bone Position (Head)
            uintptr_t sceneNode = proc->read<uintptr_t>(pawn + offsets::pawn::m_pGameSceneNode);
            if (!sceneNode) continue;
            uintptr_t boneArray = proc->read<uintptr_t>(sceneNode + offsets::bone::m_modelState + 0x80);
            if (!boneArray) continue;
            
            Vec3 bonePos = proc->read<Vec3>(boneArray + (g_Aimbot.bone * 32));

            // 4. Calculate FOV
            Vec3 aimAng = CalcAngle(eyePos, bonePos);
            NormalizeAngles(aimAng);

            float dPitch = aimAng.x - curAng.x;
            float dYaw   = aimAng.y - curAng.y;
            float fovDist = std::sqrt(dPitch * dPitch + dYaw * dYaw);

            if (fovDist < bestFov) {
                bestFov   = fovDist;
                bestPawn  = pawn;
                bestAngle = aimAng;
            }
        }

        if (!bestPawn) continue;

        // 5. Smoothing
        if (g_Aimbot.smooth > 1.0f) {
            bestAngle.x = curAng.x + (bestAngle.x - curAng.x) / g_Aimbot.smooth;
            bestAngle.y = curAng.y + (bestAngle.y - curAng.y) / g_Aimbot.smooth;
            NormalizeAngles(bestAngle);
        }

        // 6. Write back to memory
        proc->write<Vec3>(localPawn + m_angEyeAngles, bestAngle);
    }
}
