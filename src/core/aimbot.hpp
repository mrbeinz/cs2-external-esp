#pragma once
#include <Windows.h>
#include <thread>
#include <chrono>
#include <cmath>
#include "memory/Memory.hpp"
#include "offsets/Offsets.hpp"

// Standard CS2 offsets
constexpr std::ptrdiff_t m_vecViewOffset = 0x120; 

struct Vec3 { float x, y, z; };

struct AimbotSettings {
    bool  enabled       = false;
    bool  teamCheck     = true;
    float fov           = 15.0f;
    float smooth        = 2.0f;
    int   bone          = 6; // 6 = Head
    int   hotkey        = 0; // 0 = Always On (for testing)
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
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
        
        if (!g_Aimbot.enabled) continue;
        if (g_Aimbot.hotkey != 0 && !(GetAsyncKeyState(g_Aimbot.hotkey) & 0x8000)) continue;
        
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
        
        // --- FIX: Calculate actual Eye Position ---
        Vec3 origin = proc->read<Vec3>(localPawn + offsets::pawn::m_vOldOrigin);
        Vec3 viewOffset = proc->read<Vec3>(localPawn + m_vecViewOffset);
        Vec3 eyePos = { origin.x + viewOffset.x, origin.y + viewOffset.y, origin.z + viewOffset.z };
        
        uintptr_t viewAnglesPtr = proc->read<uintptr_t>(clientBase + offsets::viewAngles);
        if (!viewAnglesPtr) continue;
        Vec3 curAng = proc->read<Vec3>(viewAnglesPtr);

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
            
            // --- FIX: Use eyePos, not curAng ---
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

        // --- FIX: Smoothing and Mouse Movement ---
        float dX = bestAngle.y - curAng.y;
        float dYaw = bestAngle.x - curAng.x; // Pitch
        
        // Normalize delta
        if (dX > 180.0f) dX -= 360.0f;
        if (dX < -180.0f) dX += 360.0f;
        
        // Apply smoothing
        dX /= g_Aimbot.smooth;
        dYaw /= g_Aimbot.smooth;

        // Move the mouse using SendInput (Bypasses view angle overwrite)
        if (std::abs(dX) > 0.1f || std::abs(dYaw) > 0.1f) {
            INPUT input = { 0 };
            input.type = INPUT_MOUSE;
            input.mi.dx = (LONG)(dX); 
            input.mi.dy = (LONG)(dYaw);
            input.mi.dwFlags = MOUSEEVENTF_MOVE;
            SendInput(1, &input, sizeof(INPUT));
        }
    }
}
