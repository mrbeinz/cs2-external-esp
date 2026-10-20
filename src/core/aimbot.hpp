// src/core/Aimbot.hpp
#pragma once
#include <Windows.h>
#include <thread>
#include <chrono>
#include <cmath>

// Adjust these includes to match the exact header names in your folders
#include "memory/Memory.hpp" 
#include "offsets/Offsets.hpp" 
#include "engine/Engine.hpp"

// ---------------------------------------------------------
// NOTE: We need to replace "ProcessMgr.Read" and "ClientDLL"
// with the exact names used inside your Memory and Engine files.
// ---------------------------------------------------------

struct Vec3 { float x, y, z; };

struct AimbotSettings {
    bool  enabled       = false;
    bool  teamCheck     = true;
    float fov           = 15.f;
    float smooth        = 1.5f;
    int   bone          = 6; // 6 = Head
    int   hotkey        = VK_XBUTTON2; // Mouse 5
};

inline AimbotSettings g_Aimbot;

inline Vec3 CalcAngle(const Vec3& src, const Vec3& dst) {
    Vec3 delta = { dst.x - src.x, dst.y - src.y, dst.z - src.z };
    float hyp = std::sqrt(delta.x*delta.x + delta.y*delta.y);
    Vec3 ang;
    ang.x = std::atan2(-delta.z, hyp) * 57.295779513f;
    ang.y = std::atan2(delta.y, delta.x) * 57.295779513f;
    ang.z = 0.f;
    return ang;
}

inline void NormalizeAngles(Vec3& a) {
    while (a.x > 89.f)  a.x -= 180.f;
    while (a.x < -89.f) a.x += 180.f;
    while (a.y > 180.f) a.y -= 360.f;
    while (a.y < -180.f) a.y += 360.f;
}

inline void AimbotThread() {
    while (true) {
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
        if (!g_Aimbot.enabled) continue;
        if (!(GetAsyncKeyState(g_Aimbot.hotkey) & 0x8000)) continue;

        // ---------------------------------------------------------
        // PLACEHOLDER LOGIC
        // You will need to replace Memory::Read with the correct name.
        // ---------------------------------------------------------
        
        /* 
        uintptr_t localPawn = Memory::Read<uintptr_t>(ClientDLL + offsets::dwLocalPlayerPawn);
        if (!localPawn) continue;

        int localTeam = Memory::Read<int>(localPawn + offsets::m_iTeamNum);
        Vec3 eyePos = Memory::Read<Vec3>(localPawn + offsets::m_vecViewOffset);
        Vec3 curAng = Memory::Read<Vec3>(localPawn + offsets::m_angEyeAngles);

        uintptr_t entityList = Memory::Read<uintptr_t>(ClientDLL + offsets::dwEntityList);
        if (!entityList) continue;

        uintptr_t bestPawn = 0;
        Vec3 bestAngle{};
        float bestFov = g_Aimbot.fov;

        for (int i = 1; i <= 64; ++i) {
            uintptr_t listEntry = Memory::Read<uintptr_t>(entityList + 0x10 * (i & 0x7FFF) + 0x8 * (i >> 15));
            if (!listEntry) continue;
            uintptr_t pawn = Memory::Read<uintptr_t>(listEntry + 0x10 * (i & 0x1FF));
            if (!pawn) continue;

            int hp = Memory::Read<int>(pawn + offsets::m_iHealth);
            if (hp <= 0 || hp > 100) continue;

            int team = Memory::Read<int>(pawn + offsets::m_iTeamNum);
            if (g_Aimbot.teamCheck && team == localTeam) continue;

            uintptr_t sceneNode = Memory::Read<uintptr_t>(pawn + offsets::m_pGameSceneNode);
            if (!sceneNode) continue;
            uintptr_t boneArray = Memory::Read<uintptr_t>(sceneNode + offsets::m_modelState + 0x80);
            if (!boneArray) continue;
            Vec3 bonePos = Memory::Read<Vec3>(boneArray + (g_Aimbot.bone * 32));

            Vec3 aimAng = CalcAngle(eyePos, bonePos);
            NormalizeAngles(aimAng);

            float dPitch = aimAng.x - curAng.x;
            float dYaw   = aimAng.y - curAng.y;
            float fovDist = std::sqrt(dPitch*dPitch + dYaw*dYaw);
            
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

        Memory::Write<Vec3>(localPawn + offsets::m_angEyeAngles, bestAngle);
        */
    }
}
