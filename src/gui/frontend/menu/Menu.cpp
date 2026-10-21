#include "Menu.hpp"
#include "core/Aimbot.hpp"
#include "core/engine/cache/Cache.hpp"
#include "gui/renderer/Renderer.hpp"
#include "gui/renderer/window/Window.hpp"
#include "assets/fonts/Icons.h"

// Required for the Discord Link
#include <Windows.h>
#include <shellapi.h>

bool Menu::Init() {
	return GetInstance().InitImpl();
}

void Menu::Render() {
	return GetInstance().RenderImpl();
}

void Menu::RenderStartupHelp() {
	return GetInstance().RenderStartupHelpImpl();
}

ImVec2 Menu::GetPos() {
	return GetInstance().pos;
}

ImVec2 Menu::GetSize() {
	return GetInstance().size;
}

bool Menu::InitImpl() {
	SetupStyles();
	LOGF(INFO, "Successfully initialized menu...");
	return true;
}

void Menu::RenderImpl() {
	if (!isSetup) return;

	static auto io = ImGui::GetIO();
	static auto screen = io.DisplaySize;
	static auto color_flags = ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoLabel | ImGuiColorEditFlags_None;

	static auto title = "VAULTED";

	ImGui::SetNextWindowSize(ImVec2(650, 450), ImGuiCond_FirstUseEver);
	ImGui::SetNextWindowPos(ImVec2(screen.x / 2 - 325, screen.y / 2 - 225), ImGuiCond_FirstUseEver);

	if (ImGui::Begin(title, nullptr, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoScrollbar)) {
		this->pos = ImGui::GetWindowPos();
		this->size = ImGui::GetWindowSize();

		static int active_tab = 3; // Default to Aim tab (ID 3)

		if (ImGui::BeginChild("##main_split")) {
			auto size = ImGui::GetContentRegionAvail();

			// --- SIDEBAR ---
			ImGui::BeginChild("##tab_buttons", ImVec2(140, size.y), true);
			{
				ImGui::SetCursorPosX(15);
				ImGui::SetCursorPosY(20);
				ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.90f, 0.20f, 0.20f, 1.0f));
				ImGui::Text("VAULTED");
				ImGui::PopStyleColor();
				
				ImGui::SetCursorPosY(35); ImGui::Separator(); ImGui::SetCursorPosY(50);

				ImGui::PushStyleVar(ImGuiStyleVar_ButtonTextAlign, ImVec2(0.0f, 0.5f));
				ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(12, 8));

				auto DrawTab = [&](const char* label, int id) {
					bool is_active = (active_tab == id);
					if (is_active) {
						ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.90f, 0.20f, 0.20f, 1.0f));
						ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.90f, 0.20f, 0.20f, 1.0f));
						ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 1.0f, 1.0f, 1.0f));
					} else {
						ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.0f, 0.0f, 0.0f, 0.0f));
						ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.15f, 0.15f, 0.18f, 1.0f));
						ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.60f, 0.60f, 0.60f, 1.0f));
					}
					if (ImGui::Button(label, ImVec2(-1, 32))) active_tab = id;
					ImGui::PopStyleColor(3);
				};

				// Custom Tabs
				DrawTab("Aimbot", 3);
				
				ImGui::Dummy(ImVec2(0, 5));
				ImGui::Separator();
				ImGui::Dummy(ImVec2(0, 5));

				// Original Tabs
				for (const auto& tab : tabs) {
					DrawTab(tab.label.c_str(), tab.id);
				}

				ImGui::PopStyleVar(2);

				// --- DISCORD BUTTON ---
				auto space = ImGui::GetContentRegionAvail().y;
				ImGui::SetCursorPosY(ImGui::GetCursorPosY() + space - 40);
				if (ImGui::Button("Join Discord", ImVec2(-1, 30))) {
					ShellExecuteA(NULL, "open", "https://discord.gg/yesbh5jzzA", NULL, NULL, SW_SHOWNORMAL);
				}
			}
			ImGui::EndChild();

			ImGui::SameLine();

			// --- CONTENT AREA ---
			ImGui::BeginChild("##tab_content", ImVec2(0, size.y), true);
			{
				ImGui::Dummy(ImVec2(0, 5));

				if (active_tab == 3) { // VAULTED AIMBOT
					ImGui::TextColored(ImVec4(0.90f, 0.20f, 0.20f, 1.0f), "AIMBOT");
					ImGui::Separator();
					ImGui::Spacing();

					ImGui::Checkbox("Enable Aimbot", &g_Aimbot.enabled);
					ImGui::BeginDisabled(!g_Aimbot.enabled);
					{
						ImGui::Spacing();
						ImGui::SliderFloat("FOV", &g_Aimbot.fov, 1.0f, 180.0f, "%.0f°");
						ImGui::SliderFloat("Smoothness", &g_Aimbot.smooth, 1.0f, 10.0f, "%.1f");
						ImGui::Checkbox("Team Check", &g_Aimbot.teamCheck);
						
						ImGui::Spacing();
						ImGui::Separator();
						ImGui::Spacing();

						// Keybind Button
						static bool waiting_for_key = false;
						char buf[128];
						
						if (waiting_for_key) {
							strcpy_s(buf, sizeof(buf), "Press any key...");
							for (int i = 1; i < 255; i++) {
								if (GetAsyncKeyState(i) & 0x8000) {
									if (i != VK_LBUTTON && i != VK_RBUTTON) {
										g_Aimbot.hotkey = i;
										waiting_for_key = false;
										break;
									}
								}
							}
							if (GetAsyncKeyState(VK_ESCAPE) & 0x8000) {
								g_Aimbot.hotkey = 0; // Always On
								waiting_for_key = false;
							}
						} else {
							if (g_Aimbot.hotkey == 0) {
								strcpy_s(buf, sizeof(buf), "Aim Key: Always On");
							} else {
								sprintf_s(buf, sizeof(buf), "Aim Key: %s", GetKeyName(g_Aimbot.hotkey));
							}
							if (ImGui::Button(buf, ImVec2(200, 30))) {
								waiting_for_key = true;
							}
						}
						ImGui::SameLine();
						ImGui::TextDisabled("(ESC to clear)");
					}
					ImGui::EndDisabled();
				}
				else if (active_tab == Tab::PLAYER) {
					ImGui::TextColored(ImVec4(0.90f, 0.20f, 0.20f, 1.0f), "PLAYER VISUALS");
					ImGui::Separator();
					ImGui::Spacing();

					ImGui::BeginGroup();
					{
						ImGui::Checkbox("Box", &cfg::esp::box);
						ImGui::BeginDisabled(!cfg::esp::box);
						ImGui::SameLine(); ImGui::ColorEdit4("Team box color", cfg::esp::colors::box_team.data(), color_flags);
						ImGui::SameLine(); ImGui::ColorEdit4("Enemy box color", cfg::esp::colors::box_enemy.data(), color_flags);
						ImGui::EndDisabled();

						ImGui::Checkbox("Skeleton", &cfg::esp::skeleton);
						ImGui::BeginDisabled(!cfg::esp::skeleton);
						ImGui::SameLine(); ImGui::ColorEdit4("Team skeleton color", cfg::esp::colors::skeleton_team.data(), color_flags);
						ImGui::SameLine(); ImGui::ColorEdit4("Enemy skeleton color", cfg::esp::colors::skeleton_enemy.data(), color_flags);
						ImGui::EndDisabled();

						ImGui::Checkbox("Head Tracker", &cfg::esp::head_tracker);
						ImGui::BeginDisabled(!cfg::esp::head_tracker);
						ImGui::SameLine(); ImGui::ColorEdit4("Team head tracker color", cfg::esp::colors::tracker_team.data(), color_flags);
						ImGui::SameLine(); ImGui::ColorEdit4("Enemy head tracker color", cfg::esp::colors::tracker_enemy.data(), color_flags);
						ImGui::EndDisabled();

						ImGui::Checkbox("Tracers", &cfg::esp::tracers);
						ImGui::BeginDisabled(!cfg::esp::tracers);
						ImGui::SameLine(); ImGui::ColorEdit4("Team tracer color", cfg::esp::colors::tracer_team.data(), color_flags);
						ImGui::SameLine(); ImGui::ColorEdit4("Enemy tracer color", cfg::esp::colors::tracer_enemy.data(), color_flags);
						ImGui::EndDisabled();
					}
					ImGui::EndGroup();

					ImGui::SameLine();

					ImGui::BeginGroup();
					{
						ImGui::Checkbox("Health", &cfg::esp::health);
						if (cfg::esp::health) ImGui::Checkbox("Health Number", &cfg::esp::health_number);
						ImGui::Checkbox("Armor", &cfg::esp::armor);
						ImGui::Checkbox("Spotted", &cfg::esp::spotted);
						ImGui::Checkbox("Show Team", &cfg::esp::team);
					}
					ImGui::EndGroup();

					ImGui::Spacing();
					ImGui::Text("Flags");
					ImGui::Separator();

					ImGui::BeginGroup();
					{
						ImGui::Checkbox("Flashed", &cfg::esp::flags::flashed);
						ImGui::Checkbox("Reloading", &cfg::esp::flags::reloading);
						ImGui::Checkbox("Defusing", &cfg::esp::flags::defusing);
						ImGui::Checkbox("Scoped", &cfg::esp::flags::scoped);
						ImGui::Checkbox("Has C4", &cfg::esp::flags::has_c4);
					}
					ImGui::EndGroup();

					ImGui::SameLine();

					ImGui::BeginGroup();
					{
						ImGui::Checkbox("Name", &cfg::esp::flags::name);
						ImGui::Checkbox("Money", &cfg::esp::flags::money);
						ImGui::Checkbox("Weapon", &cfg::esp::flags::weapon);
						ImGui::Checkbox("Ammo", &cfg::esp::flags::ammo);
						ImGui::Checkbox("Ping", &cfg::esp::flags::ping);
					}
					ImGui::EndGroup();
				}
				else if (active_tab == Tab::WORLD) {
					ImGui::TextColored(ImVec4(0.90f, 0.20f, 0.20f, 1.0f), "WORLD");
					ImGui::Separator();
					ImGui::Spacing();
					
					ImGui::Checkbox("Bomb ESP", &cfg::esp::bomb);
					ImGui::SameLine(); ImGui::ColorEdit4("Bomb color", cfg::esp::colors::bomb.data(), color_flags);
					ImGui::Checkbox("Bomb Timer", &cfg::world::bomb::timer);

					ImGui::Spacing();
					ImGui::Text("Spectators");
					ImGui::Separator();
					ImGui::Checkbox("Enable", &cfg::world::spectators::enabled);
					if (cfg::world::spectators::enabled) {
						ImGui::Checkbox("Detailed", &cfg::world::spectators::detailed);
						ImGui::Checkbox("Only Self", &cfg::world::spectators::self_only);
					}

					ImGui::Spacing();
					ImGui::Text("Misc");
					ImGui::Separator();
					ImGui::Checkbox("Radar", &cfg::world::radar::enabled);
					if (cfg::world::radar::enabled) {
						ImGui::SameLine(); ImGui::SliderFloat("Range", &cfg::world::radar::range, 100.f, 8000.f, "%.0f");
					}
				}
				else if (active_tab == Tab::SETTINGS) {
					ImGui::TextColored(ImVec4(0.90f, 0.20f, 0.20f, 1.0f), "SETTINGS");
					ImGui::Separator();
					ImGui::Spacing();

					if (ImGui::Checkbox("Streamproof", &cfg::settings::streamproof)) {
						Window::SetAffinity(Window::hwnd, cfg::settings::streamproof ? WindowAffinity::Invisible : WindowAffinity::Disabled);
					}
					if (ImGui::Checkbox("VSync", &cfg::settings::vsync)) Window::vsync = cfg::settings::vsync;
					ImGui::Checkbox("Free CPU", &cfg::settings::free_cpu);
				}
			}
			ImGui::EndChild();
		}
		ImGui::EndChild();
	}
	ImGui::End();
}

void Menu::SetupStyles() {
	ImGuiStyle& style = ImGui::GetStyle();
	
	// Clean Dark Slate Theme
	style.Colors[ImGuiCol_Text]                  = ImVec4(0.90f, 0.90f, 0.90f, 1.00f);
	style.Colors[ImGuiCol_TextDisabled]          = ImVec4(0.40f, 0.40f, 0.40f, 1.00f);
	style.Colors[ImGuiCol_WindowBg]              = ImVec4(0.06f, 0.06f, 0.07f, 0.95f);
	style.Colors[ImGuiCol_ChildBg]               = ImVec4(0.08f, 0.08f, 0.10f, 0.95f);
	style.Colors[ImGuiCol_Border]                = ImVec4(0.15f, 0.15f, 0.18f, 1.00f);
	style.Colors[ImGuiCol_CheckMark]             = ImVec4(0.90f, 0.20f, 0.20f, 1.00f);
	style.Colors[ImGuiCol_SliderGrab]            = ImVec4(0.90f, 0.20f, 0.20f, 1.00f);
	style.Colors[ImGuiCol_Button]                = ImVec4(0.15f, 0.15f, 0.18f, 1.00f);
	style.Colors[ImGuiCol_ButtonHovered]         = ImVec4(0.20f, 0.20f, 0.25f, 1.00f);
	style.Colors[ImGuiCol_ButtonActive]          = ImVec4(0.90f, 0.20f, 0.20f, 1.00f);
	style.Colors[ImGuiCol_Header]                = ImVec4(0.15f, 0.15f, 0.18f, 1.00f);
	style.Colors[ImGuiCol_HeaderHovered]         = ImVec4(0.20f, 0.20f, 0.25f, 1.00f);
	style.Colors[ImGuiCol_HeaderActive]          = ImVec4(0.90f, 0.20f, 0.20f, 1.00f);
	style.Colors[ImGuiCol_FrameBg]               = ImVec4(0.12f, 0.12f, 0.15f, 1.00f);
	style.Colors[ImGuiCol_FrameBgHovered]        = ImVec4(0.18f, 0.18f, 0.22f, 1.00f);

	style.FrameBorderSize = 0.0f;
	style.WindowRounding = 6.0f;
	style.ChildRounding = 4.0f;
	style.FrameRounding = 4.0f;
	style.WindowPadding = ImVec2(0, 0);
	style.ItemSpacing = ImVec2(8, 8);

	auto& io = ImGui::GetIO();
	io.Fonts->Clear();
	io.Fonts->AddFontFromFileTTF("C:\\Windows\\Fonts\\arial.ttf", 16.0f);
}

void Menu::RenderStartupHelpImpl() {
	static bool has_opened_menu = false;
	if (has_opened_menu) return;
	auto& io = ImGui::GetIO();
	auto screen = io.DisplaySize;
	auto d = ImGui::GetBackgroundDrawList();
	if (Renderer::IsOpen()) has_opened_menu = true;
	auto help = "To OPEN the menu, Use Insert or Right Shift keys\n\t\t\t\tTo CLOSE, press the End key";
	auto size = ImGui::CalcTextSize(help);
	d->AddText(ImVec2(screen.x / 2 - size.x / 2, 80), IM_COL32(255, 255, 255, 255), help);
}
