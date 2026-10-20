#include "Menu.hpp"
#include "core/Aimbot.hpp"
#include "core/Skinchanger.hpp"
#include "core/engine/cache/Cache.hpp"
#include "gui/renderer/Renderer.hpp"
#include "gui/renderer/window/Window.hpp"
#include "assets/fonts/Icons.h"

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
	if (!isSetup)
		return;

	static auto io = ImGui::GetIO();
	static auto screen = io.DisplaySize;
	static auto color_flags = ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoLabel | ImGuiColorEditFlags_None;

	static auto title = "VAULTED";

	ImGui::SetNextWindowSize(ImVec2(600, 400), ImGuiCond_FirstUseEver);
	ImGui::SetNextWindowPos(ImVec2(screen.x / 2 - 300, screen.y / 2 - 200), ImGuiCond_FirstUseEver);

	ImGui::GetWindowPos();
	if (ImGui::Begin(title, nullptr, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize)) {
		this->pos = ImGui::GetWindowPos();
		this->size = ImGui::GetWindowSize();

		static int active_tab = 0;

		if (ImGui::BeginChild("##main_split"))
		{
			auto size = ImGui::GetContentRegionAvail();

			ImGui::BeginChild("##tab_buttons", ImVec2(130, size.y), true);
			{
				// --- VAULTED BRANDING ---
				ImGui::SetCursorPosX(ImGui::GetCursorPosX() + 10);
				ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 15);
				ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.60f, 0.20f, 1.00f, 1.0f));
				ImGui::Text("VAULTED");
				ImGui::PopStyleColor();
				
				ImGui::Spacing();
				ImGui::Separator();
				ImGui::Spacing();

				ImGui::PushStyleVar(ImGuiStyleVar_ButtonTextAlign, ImVec2(0.1f, 0.5f));
				
				// --- AIM TAB BUTTON ---
				bool is_aim_active = (active_tab == 3);
				if (is_aim_active) {
					ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.60f, 0.20f, 1.00f, 0.30f));
					ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.60f, 0.20f, 1.00f, 0.30f));
					ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.60f, 0.20f, 1.00f, 0.30f));
				}
				if (ImGui::Button("Aim", ImVec2(-1, 28))) active_tab = 3;
				if (is_aim_active) ImGui::PopStyleColor(3);

				// --- SKINS TAB BUTTON ---
				bool is_skins_active = (active_tab == 4);
				if (is_skins_active) {
					ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.60f, 0.20f, 1.00f, 0.30f));
					ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.60f, 0.20f, 1.00f, 0.30f));
					ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.60f, 0.20f, 1.00f, 0.30f));
				}
				if (ImGui::Button("Skins", ImVec2(-1, 28))) active_tab = 4;
				if (is_skins_active) ImGui::PopStyleColor(3);

				// --- EXISTING TABS ---
				for (const auto& tab : tabs)
				{
					bool is_active = (active_tab == tab.id);

					if (is_active)
					{
						ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.60f, 0.20f, 1.00f, 0.30f));
						ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.60f, 0.20f, 1.00f, 0.30f));
						ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.60f, 0.20f, 1.00f, 0.30f));
					}

					if (ImGui::Button((tab.icon + " " + tab.label).c_str(), ImVec2(-1, 28)))
						active_tab = tab.id;

					if (is_active) 
						ImGui::PopStyleColor(3);
				}
				ImGui::PopStyleVar(1);

				auto space = ImGui::GetContentRegionAvail().y;
				ImGui::SetCursorPosY(ImGui::GetCursorPosY() + space - 16.f * 3);

				ImGui::Dummy(ImVec2(11, 0)); ImGui::SameLine();
				ImGui::TextLinkOpenURL(Icons::DISCORD, "https://discord.gg/pRew8ZDkyp");
				ImGui::SameLine();
				ImGui::Dummy(ImVec2(11, 0)); ImGui::SameLine();
				ImGui::TextLinkOpenURL(Icons::GITHUB, "https://github.com/IMXNOOBX/cs2-external-esp");
				ImGui::Separator();

				ImGui::Checkbox("Enable", &cfg::enabled);
			}
			ImGui::EndChild();

			ImGui::SameLine();

			ImGui::BeginChild("##tab_content", ImVec2(0, size.y), true);
			{
				// --- AIM TAB CONTENT ---
				if (active_tab == 3)
				{
					ImGui::Text("Aimbot");
					ImGui::Separator();
					ImGui::Spacing();

					ImGui::Checkbox("Enable Aimbot", &g_Aimbot.enabled);
					ImGui::BeginDisabled(!g_Aimbot.enabled);
					{
						ImGui::SliderFloat("FOV", &g_Aimbot.fov, 1.0f, 180.0f, "%.1f");
						ImGui::SliderFloat("Smoothness", &g_Aimbot.smooth, 1.0f, 10.0f, "%.1f");
						ImGui::Checkbox("Team Check", &g_Aimbot.teamCheck);
						
						ImGui::Spacing();
						
						// --- KEYBIND BUTTON ---
						static bool waiting_for_key = false;
						char buf[128];
						
						if (waiting_for_key) {
							strcpy_s(buf, sizeof(buf), "Press any key to bind...");
							
							// Check all keys
							for (int i = 1; i < 255; i++) {
								if (GetAsyncKeyState(i) & 0x8000) {
									// Ignore mouse clicks so they don't bind accidentally when clicking UI
									if (i != VK_LBUTTON && i != VK_RBUTTON) {
										g_Aimbot.hotkey = i;
										waiting_for_key = false;
										break;
									}
								}
							}
							
							// Also allow ESC to clear the bind
							if (GetAsyncKeyState(VK_ESCAPE) & 0x8000) {
								g_Aimbot.hotkey = 0;
								waiting_for_key = false;
							}
						} else {
							sprintf_s(buf, sizeof(buf), "Aim Key: %s", GetKeyName(g_Aimbot.hotkey));
							if (ImGui::Button(buf, ImVec2(-1, 24))) {
								waiting_for_key = true;
							}
						}
						ImGui::TextDisabled("Click the button above to change.");
						ImGui::TextDisabled("Press ESC while binding to set to 'Always'.");
					}
					ImGui::EndDisabled();
				}
				// --- SKINS TAB CONTENT ---
				else if (active_tab == 4)
				{
					ImGui::Text("Skinchanger");
					ImGui::Separator();
					ImGui::Spacing();

					ImGui::Checkbox("Enable Skinchanger", &g_Skinchanger.enabled);
					ImGui::BeginDisabled(!g_Skinchanger.enabled);
					{
						if (ImGui::BeginCombo("Weapon", GetWeaponName(g_Skinchanger.target_weapon_id))) {
							for (const auto& w : g_WeaponList) {
								bool is_selected = (g_Skinchanger.target_weapon_id == w.id);
								if (ImGui::Selectable(w.name, is_selected)) {
									g_Skinchanger.target_weapon_id = w.id;
								}
								if (is_selected) ImGui::SetItemDefaultFocus();
							}
							ImGui::EndCombo();
						}

						if (ImGui::BeginCombo("Skin", GetSkinName(g_Skinchanger.skin_id))) {
							for (const auto& pair : g_SkinDatabase) {
								bool is_selected = (g_Skinchanger.skin_id == pair.first);
								if (ImGui::Selectable(pair.second.c_str(), is_selected)) {
									g_Skinchanger.skin_id = pair.first;
								}
								if (is_selected) ImGui::SetItemDefaultFocus();
							}
							ImGui::EndCombo();
						}

						ImGui::Spacing();
						ImGui::SliderFloat("Wear", &g_Skinchanger.wear, 0.0f, 1.0f, "%.4f");
						ImGui::SliderInt("Pattern Seed", &g_Skinchanger.seed, 0, 1000);
						
						ImGui::Spacing();
						ImGui::TextDisabled("Hold the weapon to apply the skin.");
					}
					ImGui::EndDisabled();
				}
				else if (active_tab == Tab::PLAYER)
				{
					ImGui::Text("Visuals");
					ImGui::Separator();

					ImGui::BeginGroup();
					{
						ImGui::Checkbox("Box", &cfg::esp::box);
						ImGui::BeginDisabled(!cfg::esp::box);
						{
							ImGui::SameLine();
							ImGui::ColorEdit4("Team box color", cfg::esp::colors::box_team.data(), color_flags);
							ImGui::SameLine();
							ImGui::ColorEdit4("Enemy box color", cfg::esp::colors::box_enemy.data(), color_flags);
						}
						ImGui::EndDisabled();

						ImGui::Checkbox("Skeleton", &cfg::esp::skeleton);
						ImGui::BeginDisabled(!cfg::esp::skeleton);
						{
							ImGui::SameLine();
							ImGui::ColorEdit4("Team skeleton color", cfg::esp::colors::skeleton_team.data(), color_flags);
							ImGui::SameLine();
							ImGui::ColorEdit4("Enemy skeleton color", cfg::esp::colors::skeleton_enemy.data(), color_flags);
						}
						ImGui::EndDisabled();

						ImGui::Checkbox("Head Tracker", &cfg::esp::head_tracker);
						ImGui::BeginDisabled(!cfg::esp::head_tracker);
						{
							ImGui::SameLine();
							ImGui::ColorEdit4("Team head tracker color", cfg::esp::colors::tracker_team.data(), color_flags);
							ImGui::SameLine();
							ImGui::ColorEdit4("Enemy head tracker color", cfg::esp::colors::tracker_enemy.data(), color_flags);
						}
						ImGui::EndDisabled();

						ImGui::Checkbox("Tracers", &cfg::esp::tracers);
						ImGui::BeginDisabled(!cfg::esp::tracers);
						{
							ImGui::SameLine();
							ImGui::ColorEdit4("Team tracer color", cfg::esp::colors::tracer_team.data(), color_flags);
							ImGui::SameLine();
							ImGui::ColorEdit4("Enemy tracer color", cfg::esp::colors::tracer_enemy.data(), color_flags);
						}
						ImGui::EndDisabled();
					}
					ImGui::EndGroup();

					ImGui::SameLine();

					ImGui::BeginGroup();
					{
						ImGui::Checkbox("Health", &cfg::esp::health);
						if (cfg::esp::health)
							ImGui::Checkbox("Health Number", &cfg::esp::health_number);
						ImGui::Checkbox("Armor", &cfg::esp::armor);

						ImGui::Checkbox("Spotted", &cfg::esp::spotted);
						ImGui::SetItemTooltip("Esp will only be visible if the player has been spotted by you");

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
				else if (active_tab == Tab::WORLD)
				{
					ImGui::Text("Bomb");
					ImGui::Separator();
					{
						ImGui::Checkbox("Bomb ESP", &cfg::esp::bomb);
						ImGui::SameLine();
						ImGui::ColorEdit4("Bomb color", cfg::esp::colors::bomb.data(), color_flags);
					}
					ImGui::Checkbox("Bomb Location", &cfg::world::bomb::location);
					ImGui::Checkbox("Bomb Timer", &cfg::world::bomb::timer);

					ImGui::Spacing();

					ImGui::Text("Spectator list");
					ImGui::Separator();

					ImGui::Checkbox("Enable", &cfg::world::spectators::enabled);
					if (cfg::world::spectators::enabled) {
						ImGui::Checkbox("Detailed", &cfg::world::spectators::detailed);
						ImGui::Checkbox("Only Self", &cfg::world::spectators::self_only);
					}

					ImGui::Spacing();

					ImGui::Text("Misc");
					ImGui::Separator();

					ImGui::Checkbox("Crosshair", &cfg::world::crosshair::enabled);
					ImGui::Checkbox("Velocity Graph", &cfg::world::velocity::enabled);

					ImGui::Spacing();

					ImGui::Text("Radar");
					ImGui::Separator();

					ImGui::Checkbox("Radar", &cfg::world::radar::enabled);
					ImGui::BeginDisabled(!cfg::world::radar::enabled);
					{
						ImGui::SameLine();
						ImGui::SliderFloat("Range", &cfg::world::radar::range, 100.f, 8000.f, "%.0f u");
						ImGui::Checkbox("Disable Rotation", &cfg::world::radar::no_rotate);
					}
					ImGui::EndDisabled();
				}
				else if (active_tab == Tab::SETTINGS)
				{
					ImGui::Text("Misc");
					ImGui::Separator();

					if (ImGui::Checkbox("Streamproof", &cfg::settings::streamproof))
					{
						Window::SetAffinity(
							Window::hwnd,
							cfg::settings::streamproof ? WindowAffinity::Invisible : WindowAffinity::Disabled
						);
					}

					ImGui::Checkbox("Watermark", &cfg::settings::watermark);

					if (ImGui::Checkbox("VSync", &cfg::settings::vsync))
						Window::vsync = cfg::settings::vsync;

					ImGui::Checkbox("Free CPU", &cfg::settings::free_cpu);
				}
			}
			ImGui::EndChild();
			ImGui::EndChild();
		}
	}
	ImGui::End();
}

void Menu::SetupStyles() {
	ImGuiStyle& style = ImGui::GetStyle();
	style.Colors[ImGuiCol_Text] = ImVec4(1.00f, 1.00f, 1.00f, 1.00f);
	style.Colors[ImGuiCol_TextDisabled] = ImVec4(0.50f, 0.50f, 0.50f, 1.00f);

	style.Colors[ImGuiCol_WindowBg] = ImVec4(0.02f, 0.02f, 0.02f, 0.95f);
	style.Colors[ImGuiCol_ChildBg] = ImVec4(0.03f, 0.03f, 0.03f, 0.95f);
	style.Colors[ImGuiCol_PopupBg] = ImVec4(0.05f, 0.05f, 0.05f, 1.00f);
	
	style.Colors[ImGuiCol_Border] = ImVec4(0.60f, 0.20f, 1.00f, 0.80f);
	style.Colors[ImGuiCol_BorderShadow] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);

	style.Colors[ImGuiCol_FrameBg] = ImVec4(0.10f, 0.10f, 0.12f, 1.00f);
	style.Colors[ImGuiCol_FrameBgHovered] = ImVec4(0.15f, 0.15f, 0.18f, 1.00f);
	style.Colors[ImGuiCol_FrameBgActive] = ImVec4(0.20f, 0.20f, 0.25f, 1.00f);
	
	style.Colors[ImGuiCol_TitleBg] = ImVec4(0.02f, 0.02f, 0.02f, 1.00f);
	style.Colors[ImGuiCol_TitleBgActive] = ImVec4(0.05f, 0.05f, 0.05f, 1.00f);
	style.Colors[ImGuiCol_TitleBgCollapsed] = ImVec4(0.00f, 0.00f, 0.00f, 0.51f);

	style.Colors[ImGuiCol_CheckMark] = ImVec4(0.60f, 0.20f, 1.00f, 1.00f);
	style.Colors[ImGuiCol_SliderGrab] = ImVec4(0.60f, 0.20f, 1.00f, 1.00f);
	style.Colors[ImGuiCol_SliderGrabActive] = ImVec4(0.70f, 0.30f, 1.00f, 1.00f);

	style.Colors[ImGuiCol_Button] = ImVec4(0.12f, 0.12f, 0.14f, 1.00f);
	style.Colors[ImGuiCol_ButtonHovered] = ImVec4(0.25f, 0.25f, 0.30f, 1.00f);
	style.Colors[ImGuiCol_ButtonActive] = ImVec4(0.40f, 0.15f, 0.80f, 1.00f);
	
	style.Colors[ImGuiCol_Header] = ImVec4(0.15f, 0.15f, 0.18f, 1.00f);
	style.Colors[ImGuiCol_HeaderHovered] = ImVec4(0.20f, 0.20f, 0.25f, 1.00f);
	style.Colors[ImGuiCol_HeaderActive] = ImVec4(0.60f, 0.20f, 1.00f, 1.00f);
	
	style.Colors[ImGuiCol_Separator] = style.Colors[ImGuiCol_Border];
	style.Colors[ImGuiCol_SeparatorHovered] = ImVec4(0.50f, 0.20f, 0.90f, 1.00f);
	style.Colors[ImGuiCol_SeparatorActive] = ImVec4(0.60f, 0.20f, 1.00f, 1.00f);

	style.FrameBorderSize = 1.0f;

	style.WindowRounding = 8.f;
	style.ChildRounding = 6.f;
	style.FrameRounding = 4.f;
	style.PopupRounding = 4.f;
	style.GrabRounding = 4.f;

	auto& io = ImGui::GetIO();
	io.Fonts->Clear();
	io.Fonts->AddFontFromFileTTF("C:\\Windows\\Fonts\\arial.ttf", 16.0f);

	ImFontConfig merge_icon_cfg{};
	merge_icon_cfg.FontDataOwnedByAtlas = false;
	merge_icon_cfg.MergeMode = true;
	merge_icon_cfg.GlyphOffset = Vec2_t(0, 3.5f);

	static const ImWchar icon_ranges[] = { 0xE100, 0xE108, 0 };
	io.Fonts->AddFontFromMemoryTTF(icons_font, icons_font_len, 20.f, &merge_icon_cfg, icon_ranges);
}

void Menu::RenderStartupHelpImpl() {
	static bool has_opened_menu = false;
	if (has_opened_menu) return;

	auto& io = ImGui::GetIO();
	auto screen = io.DisplaySize;
	auto d = ImGui::GetBackgroundDrawList();

	if (Renderer::IsOpen())
		has_opened_menu = true;

	auto help = "To OPEN the menu, Use Insert or Right Shift keys"
		"\n\t\t\t\tTo CLOSE, press the End key";
	auto size = ImGui::CalcTextSize(help);

	d->AddText(
		ImVec2(screen.x / 2 - size.x / 2, 80),
		IM_COL32(255, 255, 255, 255),
		help
	);
}
