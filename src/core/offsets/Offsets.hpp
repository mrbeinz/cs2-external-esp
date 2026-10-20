#pragma once
#include <Windows.h>
#include <string>
#include <cstddef>

namespace offsets
{
	// Hardcoded from the latest dump to ensure it works immediately
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
}
