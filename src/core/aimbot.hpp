#pragma once
#include <Windows.h>
#include <thread>
#include <chrono>
#include <cmath>
#include <cstdio>
#include "memory/Memory.hpp"
#include "offsets/Offsets.hpp"

constexpr std::ptrdiff_t m_vecViewOffset = 0x120; 
constexpr std::ptrdiff_t m_angEyeAngles = 0x1518;

struct Vec3 { float x, y, z; };

struct AimbotSettings {
    bool  enabled       = true;  // Force enabled
    bool  teamCheck     = false; // Force off for testing
    float fov           = 360.0f;// Force 360 FOV
    float smooth        = 1.0f;  // Instant snap
    int   bone          = 6;     // Head
    int   hotkey        = 0;     // Always on
};

inline AimbotSettings g_Aimbot;

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

inline void AimbotThread(pProcess* proc) {
    if (!proc) return;
    while (true) {
        std::this_thread::sleep_for(std::chrono::milliseconds(5)); // Slight delay to save CPU
        
        if (!g_Aimbot.enabled) continue;
        
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

        uint8_t localTeam = proc->read<uint8_t>(localPawn + offsets::pawn::m_iTeamNum);
        
        // Calculate Eye Position properly
        Vec3 origin = proc->read<Vec3>(localPawn + offsets::pawn::m_vOldOrigin);
        Vec3 viewOffset = proc->read<Vec3>(localPawn + m_vecViewOffset);
        Vec3 eyePos = { origin.x + viewOffset.x, origin.y + viewOffset.y, origin.z + viewOffset.z };

        uintptr_t bestPawn = 0;
        Vec3 bestAngle{};
        float bestFov = g_Aimbot.fov;

        for (int i = 1; i <= 64; ++i) {
            uintptr_t listEntry = proc->read<uintptr_t>(entityList + 0x10 * (i & 0x7FFF) + 0x8 * (i >> 15));
            if (!listEntry) continue;
            uintptr_t pawn = proc->read<uintptr_t>(listEntry + 0x10 * (i & 0x1FF));
            if (!pawn || pawn == localPawn) continue;

            int hp = proc->read<int>(pawn + offsets::pawn::m_iHealth);
            if (hp <= 0 || hp > 100) continue;

            uint8_t team = proc->read<uint8_t>(pawn + offsets::pawn::m_iTeamNum);
            if (g_Aimbot.teamCheck && team == localTeam) continue;

            uintptr_t sceneNode = proc->read<uintptr_t>(pawn + offsets::pawn::m_pGameSceneNode);
            if (!sceneNode) continue;
            uintptr_t boneArray = proc->read<uintptr_t>(sceneNode + offsets::bone::m_modelState + 0x80);
            if (!boneArray) continue;
            
            Vec3 bonePos = proc->read<Vec3>(boneArray + (g_Aimbot.bone * 32));
            
            Vec3 aimAng = CalcAngle(eyePos, bonePos);
            NormalizeAngles(aimAng);

            // Direct delta calculation (Bypasses FOV check if FOV is 360)
            float dPitch = aimAng.x;
            float dYaw   = aimAng.y;
            float fovDist = std::sqrt(dPitch * dPitch + dYaw * dYaw);

            if (fovDist < bestFov) {
                bestFov   = fovDist;
                bestPawn  = pawn;
                bestAngle = aimAng;
            }
        }

        if (!bestPawn) continue;

        // --- DEBUG PRINT ---
        printf("[AIMBOT] Target found! Writing ViewAngles...\n");

        // Write to the global ViewAngles pointer
        uintptr_t viewAnglesPtr = proc->read<uintptr_t>(clientBase + offsets::viewAngles);
        if (viewAnglesPtr) {
            proc->write<Vec3>(viewAnglesPtr, bestAngle);
        }

        // ALSO write to the Local Pawn's Eye Angles as a fallback
        proc->write<Vec3>(localPawn + m_angEyeAngles, bestAngle);
    }
}
