#pragma once
#include <Windows.h>
#include <string>
#include <cstddef>

namespace offsets
{
	// Hardcoded fallback offsets from the latest dump
	inline DWORD entityList = 0x271A908;
	inline DWORD viewMatrix = 0x256B020;
	inline DWORD localPlayerController = 0x253CB78;
	inline DWORD globalVars = 0x2230090;
	inline DWORD plantedC4 = 0x24CB3C0;
	inline DWORD weaponC4 = 0x24C9B30;
	inline DWORD viewAngles = 0x257BCC8;
	inline DWORD buildNumber = 0x61CFEC;

	namespace controller {
		constexpr std::ptrdiff_t m_iPing = 0x838;
		constexpr std::ptrdiff_t m_hPawn = 0x6BC;
		constexpr std::ptrdiff_t m_steamID = 0x788;
		constexpr std::ptrdiff_t m_iszPlayerName = 0x6FC;
		constexpr std::ptrdiff_t m_bIsLocalPlayerController = 0x790;
		constexpr std::ptrdiff_t m_pInGameMoneyServices = 0x818;
		constexpr std::ptrdiff_t m_iAccount = 0x40;
	}

	namespace pawn {
		constexpr std::ptrdiff_t m_vOldOrigin = 0x14A4;
		constexpr std::ptrdiff_t m_iHealth = 0x34C;
		constexpr std::ptrdiff_t m_iTeamNum = 0x3E7;
		constexpr std::ptrdiff_t m_bIsScoped = 0x1E98;
		constexpr std::ptrdiff_t m_ArmorValue = 0x1EC8;
		constexpr std::ptrdiff_t m_bIsDefusing = 0x1E9A;
		constexpr std::ptrdiff_t m_vecAbsVelocity = 0x3F8;
		constexpr std::ptrdiff_t m_pGameSceneNode = 0x330;
		constexpr std::ptrdiff_t m_entitySpottedState = 0x1E80;
		constexpr std::ptrdiff_t m_bSpottedByMask = 0xC;
		constexpr std::ptrdiff_t m_flFlashOverlayAlpha = 0x1504;
		constexpr std::ptrdiff_t m_pWeaponServices = 0x12F0;
		constexpr std::ptrdiff_t m_hActiveWeapon = 0x60;
		constexpr std::ptrdiff_t m_AttributeManager = 0x1290;
		constexpr std::ptrdiff_t m_Item = 0x50;
		constexpr std::ptrdiff_t m_iItemDefinitionIndex = 0x1BA;
		constexpr std::ptrdiff_t m_iClip1 = 0x1928;
		constexpr std::ptrdiff_t m_bInReload = 0x1A3C;
		constexpr std::ptrdiff_t m_pObserverServices = 0x1308;
	}

	namespace bomb {
		constexpr std::ptrdiff_t m_isPlanted = 0x8;
		constexpr std::ptrdiff_t m_bC4Activated = 0x12D0;
		constexpr std::ptrdiff_t m_nBombSite = 0x128C;
		constexpr std::ptrdiff_t m_vecAbsOrigin = 0xC8;
	}

	namespace bone {
		constexpr std::ptrdiff_t m_modelState = 0x140;
	}

	namespace observerServices {
		constexpr std::ptrdiff_t m_iObserverMode = 0x48;
		constexpr std::ptrdiff_t m_hObserverTarget = 0x4C;
	}

	namespace global {
		constexpr std::ptrdiff_t maxClients = 0x10;
		constexpr std::ptrdiff_t currentMapName = 0x180;
		constexpr std::ptrdiff_t currentTime = 0x2C;
	}

	namespace signatures
	{
		const std::string viewMatrix = "48 8D 0D ?? ?? ?? ?? 48 C1 E0 06";
		const std::string globalVars = "48 89 15 ?? ?? ?? ?? 48 89 42";
		const std::string entityList = "48 8B 0D ?? ?? ?? ?? 48 89 7C 24 ?? 8B FA C1 EB";
		const std::string localPlayerController = "48 8B 05 ?? ?? ?? ?? 41 89 BE";
		const std::string plantedC4 = "48 8B 1D ?? ?? ?? ?? 45 32 F6";
		const std::string viewAngles = "48 8B 0D ?? ?? ?? ?? 48 8D 15 ?? ?? ?? ?? 0F 57 C0";
		const std::string weaponC4 = "48 89 05 ?? ?? ?? ?? F7 C1 ?? ?? ?? ?? 74 ?? 81 E1 ?? ?? ?? ?? 89 0D ?? ?? ?? ?? 8B 05 ?? ?? ?? ?? 89 1D ?? ?? ?? ?? EB ?? 48 8B 15 ?? ?? ?? ?? 48 8B 5C 24 ?? FF C0 89 05 ?? ?? ?? ?? 48 8B C6 48 89 34 EA 80 BE";
		const std::string buildNumber = "89 05 ?? ?? ?? ?? 48 8d 0d ?? ?? ?? ?? ff 15 ?? ?? ?? ?? 48 8b 0d";
	}
}
