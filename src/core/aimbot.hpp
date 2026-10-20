// src/core/Aimbot.hpp
#pragma once
#include <Windows.h>
#include <thread>
#include <chrono>
#include <cmath>

#include "memory/Memory.hpp"
#include "offsets/Offsets.hpp"

// Missing offsets added for aimbot
constexpr std::ptrdiff_t m_angEyeAngles = 0x1518;
constexpr std::ptrdiff_t m_vecViewOffset = 0x120;

struct Vec3 { float x, y, z; };

struct AimbotSettings {
    bool  enabled       = false;
    bool  teamCheck     = true;
    float fov           = 15.0f;
    float smooth        = 1.5f;
    int   bone          = 6; // 6 = Head
    int   hotkey        = VK_XBUTTON2; // Default: Mouse 5
};

inline AimbotSettings g_Aimbot;

// Helper to turn Windows VK codes into readable strings
inline const char* GetKeyName(int vk) {
    switch (vk) {
        case 0: return "None";
        case VK_LBUTTON: return "Mouse 1";
        case VK_RBUTTON: return "Mouse 2";
        case VK_MBUTTON: return "Mouse 3";
        case VK_XBUTTON1: return "Mouse 4";
        case VK_XBUTTON2: return "Mouse 5";
        case VK_SHIFT: return "Shift";
        case VK_CONTROL: return "Ctrl";
        case VK_MENU: return "Alt";
        case VK_SPACE: return "Space";
        case VK_TAB: return "Tab";
        case VK_CAPITAL: return "Caps Lock";
        case VK_F1: return "F1"; case VK_F2: return "F2"; case VK_F3: return "F3"; case VK_F4: return "F4";
        case VK_F5: return "F5"; case VK_F6: return "F6"; case VK_F7: return "F7"; case VK_F8: return "F8";
        case VK_F9: return "F9"; case VK_F10: return "F10"; case VK_F11: return "F11"; case VK_F12: return "F12";
    }
    // Alpha-numeric keys
    if (vk >= 0x30 && vk <= 0x39) { // 0-9
        static char num[2]; num[0] = (char)vk; num[1] = '\0'; return num;
    }
    if (vk >= 0x41 && vk <= 0x5A) { // A-Z
        static char letter[2]; letter[0] = (char)vk; letter[1] = '\0'; return letter;
    }
    return "Unknown Key";
}

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
        
        // If hotkey is 0 (None), it's always active. Otherwise, check if held.
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

        int localTeam = proc->read<int>(localPawn + offsets::pawn::m_iTeamNum);
        
        Vec3 origin = proc->read<Vec3>(localPawn + offsets::pawn::m_vOldOrigin);
        Vec3 viewOffset = proc->read<Vec3>(localPawn + m_vecViewOffset);
        Vec3 eyePos = { origin.x + viewOffset.x, origin.y + viewOffset.y, origin.z + viewOffset.z };
        
        Vec3 curAng = proc->read<Vec3>(localPawn + m_angEyeAngles);

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

            int team = proc->read<int>(pawn + offsets::pawn::m_iTeamNum);
            if (g_Aimbot.teamCheck && team == localTeam) continue;

            uintptr_t sceneNode = proc->read<uintptr_t>(pawn + offsets::pawn::m_pGameSceneNode);
            if (!sceneNode) continue;
            uintptr_t boneArray = proc->read<uintptr_t>(sceneNode + offsets::bone::m_modelState + 0x80);
            if (!boneArray) continue;
            
            Vec3 bonePos = proc->read<Vec3>(boneArray + (g_Aimbot.bone * 32));

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

        if (g_Aimbot.smooth > 1.0f) {
            bestAngle.x = curAng.x + (bestAngle.x - curAng.x) / g_Aimbot.smooth;
            bestAngle.y = curAng.y + (bestAngle.y - curAng.y) / g_Aimbot.smooth;
            NormalizeAngles(bestAngle);
        }

        proc->write<Vec3>(localPawn + m_angEyeAngles, bestAngle);
    }
}
