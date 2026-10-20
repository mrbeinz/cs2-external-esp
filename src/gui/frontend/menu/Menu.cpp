#include "Menu.hpp"
#include "core/Aimbot.hpp"
#include "core/Skinchanger.hpp"
#include "core/engine/cache/Cache.hpp"
#include "gui/renderer/Renderer.hpp"
#include "gui/renderer/window/Window.hpp"

// Required for opening the Discord link
#include <Windows.h>
#include <shellapi.h>

bool Menu::Init() { return GetInstance().InitImpl(); }
void Menu::Render() { return GetInstance().RenderImpl(); }
void Menu::RenderStartupHelp() { return GetInstance().RenderStartupHelpImpl(); }
ImVec2 Menu::GetPos() { return GetInstance().pos; }
ImVec2 Menu::GetSize() { return GetInstance().size; }

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

	// Clean, hard-edged window
	ImGui::SetNextWindowSize(ImVec2(650, 450), ImGuiCond_FirstUseEver);
	ImGui::SetNextWindowPos(ImVec2(screen.x / 2 - 325, screen.y / 2 - 225), ImGuiCond_FirstUseEver);

	if (ImGui::Begin(title, nullptr, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoScrollbar)) {
		this->pos = ImGui::GetWindowPos();
		this->size = ImGui::GetWindowSize();

		static int active_tab = 0;

		if (ImGui::BeginChild("##main_split")) {
			auto size = ImGui::GetContentRegionAvail();

			// --- SIDEBAR ---
			ImGui::BeginChild("##tab_buttons", ImVec2(140, size.y), true);
			{
				// Branding
				ImGui::SetCursorPosX(15);
				ImGui::SetCursorPosY(20);
				ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.90f, 0.20f, 0.20f, 1.0f)); // Crimson Accent
				ImGui::Text("VAULTED");
				ImGui::PopStyleColor();
				
				ImGui::SetCursorPosY(35);
				ImGui::Separator();
				ImGui::SetCursorPosY(50);

				ImGui::PushStyleVar(ImGuiStyleVar_ButtonTextAlign, ImVec2(0.0f, 0.5f));
				ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(12, 8)); // Thicker buttons

				auto DrawTab = [&](const char* label, int id) {
					bool is_active = (active_tab == id);
					if (is_active) {
						ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.90f, 0.20f, 0.20f, 1.0f));
						ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.90f, 0.20f, 0.20f, 1.0f));
						ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 1.0f, 1.0f, 1.0f));
					} else {
						ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.0f, 0.0f, 0.0f, 0.0f)); // Invisible bg
						ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.15f, 0.15f, 0.18f, 1.0f));
						ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.60f, 0.60f, 0.60f, 1.0f));
					}
					if (ImGui::Button(label, ImVec2(-1, 32))) active_tab = id;
					ImGui::PopStyleColor(3);
				};

				// Custom Tabs
				DrawTab("Aimbot", 3);
				DrawTab("Skinchanger", 4);
				
				ImGui::Dummy(ImVec2(0, 5));
				ImGui::Separator();
				ImGui::Dummy(ImVec2(0, 5));

				// Original Tabs
				for (const auto& tab : tabs) {
					DrawTab(tab.label.c_str(), tab.id);
				}

				ImGui::PopStyleVar(2);

				// --- BOTTOM SIDEBAR ---
				auto space = ImGui::GetContentRegionAvail().y;
				ImGui::SetCursorPosY(ImGui::GetCursorPosY() + space - 40);
				
				// Discord Button
				if (ImGui::Button("Join Discord", ImVec2(-1, 30))) {
					ShellExecuteA(NULL, "open", "https://discord.gg/yesbh5jzzA", NULL, NULL, SW_SHOWNORMAL);
				}
			}
			ImGui::EndChild();

			ImGui::SameLine();

			// --- CONTENT AREA ---
			ImGui::BeginChild("##tab_content", ImVec2(0, size.y), true);
			{
				ImGui::Dummy(ImVec2(0, 5)); // Top padding

				if (active_tab == 3) { // AIMBOT
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
				else if (active_tab == 4) { // SKINS
					ImGui::TextColored(ImVec4(0.90f, 0.20f, 0.20f, 1.0f), "SKINCHANGER");
					ImGui::Separator();
					ImGui::Spacing();

					ImGui::Checkbox("Enable Skinchanger", &g_Skinchanger.enabled);
					ImGui::BeginDisabled(!g_Skinchanger.enabled);
					{
						ImGui::Spacing();
						if (ImGui::BeginCombo("Weapon", GetWeaponName(g_Skinchanger.target_weapon_id))) {
							for (const auto& w : g_WeaponList) {
								bool is_selected = (g_Skinchanger.target_weapon_id == w.id);
								if (ImGui::Selectable(w.name, is_selected)) g_Skinchanger.target_weapon_id = w.id;
							}
							ImGui::EndCombo();
						}
						if (ImGui::BeginCombo("Skin", GetSkinName(g_Skinchanger.skin_id))) {
							for (const auto& pair : g_SkinDatabase) {
								bool is_selected = (g_Skinchanger.skin_id == pair.first);
								if (ImGui::Selectable(pair.second.c_str(), is_selected)) g_Skinchanger.skin_id = pair.first;
							}
							ImGui::EndCombo();
						}
						ImGui::SliderFloat("Wear", &g_Skinchanger.wear, 0.0f, 1.0f, "%.4f");
						ImGui::SliderInt("Pattern Seed", &g_Skinchanger.seed, 0, 1000);
						ImGui::Spacing();
						ImGui::TextDisabled("Hold the weapon in-game to apply.");
					}
					ImGui::EndDisabled();
				}
				else if (active_tab == Tab::PLAYER) {
					ImGui::TextColored(ImVec4(0.90f, 0.20f, 0.20f, 1.0f), "PLAYER VISUALS");
					ImGui::Separator();
					ImGui::Spacing();
					
					ImGui::BeginGroup();
					ImGui::Checkbox("Box ESP", &cfg::esp::box);
					ImGui::Checkbox("Skeleton", &cfg::esp::skeleton);
					ImGui::Checkbox("Tracers", &cfg::esp::tracers);
					ImGui::EndGroup();
					
					ImGui::SameLine(150);

					ImGui::BeginGroup();
					ImGui::Checkbox("Health Bar", &cfg::esp::health);
					ImGui::Checkbox("Name", &cfg::esp::flags::name);
					ImGui::Checkbox("Weapon", &cfg::esp::flags::weapon);
					ImGui::EndGroup();
				}
				else if (active_tab == Tab::WORLD) {
					ImGui::TextColored(ImVec4(0.90f, 0.20f, 0.20f, 1.0f), "WORLD");
					ImGui::Separator();
					ImGui::Spacing();
					
					ImGui::Checkbox("Bomb ESP", &cfg::esp::bomb);
					ImGui::Checkbox("Bomb Timer", &cfg::world::bomb::timer);
					ImGui::Checkbox("Radar", &cfg::world::radar::enabled);
				}
				else if (active_tab == Tab::SETTINGS) {
					ImGui::TextColored(ImVec4(0.90f, 0.20f, 0.20f, 1.0f), "SETTINGS");
					ImGui::Separator();
					ImGui::Spacing();

					if (ImGui::Checkbox("Streamproof", &cfg::settings::streamproof)) {
						Window::SetAffinity(Window::hwnd, cfg::settings::streamproof ? WindowAffinity::Invisible : WindowAffinity::Disabled);
					}
					if (ImGui::Checkbox("VSync", &cfg::settings::vsync)) Window::vsync = cfg::settings::vsync;
					
					ImGui::Spacing();
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
	
	// Clean Dark Slate Theme (No Gradients)
	style.Colors[ImGuiCol_Text]                  = ImVec4(0.90f, 0.90f, 0.90f, 1.00f);
	style.Colors[ImGuiCol_TextDisabled]          = ImVec4(0.40f, 0.40f, 0.40f, 1.00f);
	style.Colors[ImGuiCol_WindowBg]              = ImVec4(0.06f, 0.06f, 0.07f, 1.00f); // Very dark slate
	style.Colors[ImGuiCol_ChildBg]               = ImVec4(0.08f, 0.08f, 0.10f, 1.00f); // Slightly lighter panels
	style.Colors[ImGuiCol_PopupBg]               = ImVec4(0.08f, 0.08f, 0.10f, 0.95f);
	style.Colors[ImGuiCol_Border]                = ImVec4(0.15f, 0.15f, 0.18f, 1.00f); // Subtle borders
	style.Colors[ImGuiCol_BorderShadow]          = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
	style.Colors[ImGuiCol_FrameBg]               = ImVec4(0.12f, 0.12f, 0.15f, 1.00f);
	style.Colors[ImGuiCol_FrameBgHovered]        = ImVec4(0.18f, 0.18f, 0.22f, 1.00f);
	style.Colors[ImGuiCol_FrameBgActive]         = ImVec4(0.25f, 0.25f, 0.30f, 1.00f);
	style.Colors[ImGuiCol_TitleBg]               = ImVec4(0.04f, 0.04f, 0.05f, 1.00f);
	style.Colors[ImGuiCol_TitleBgActive]         = ImVec4(0.04f, 0.04f, 0.05f, 1.00f);
	style.Colors[ImGuiCol_TitleBgCollapsed]      = ImVec4(0.04f, 0.04f, 0.05f, 1.00f);
	style.Colors[ImGuiCol_CheckMark]             = ImVec4(0.90f, 0.20f, 0.20f, 1.00f); // Crimson Red
	style.Colors[ImGuiCol_SliderGrab]            = ImVec4(0.90f, 0.20f, 0.20f, 1.00f);
	style.Colors[ImGuiCol_SliderGrabActive]      = ImVec4(1.00f, 0.30f, 0.30f, 1.00f);
	style.Colors[ImGuiCol_Button]                = ImVec4(0.15f, 0.15f, 0.18f, 1.00f);
	style.Colors[ImGuiCol_ButtonHovered]         = ImVec4(0.20f, 0.20f, 0.25f, 1.00f);
	style.Colors[ImGuiCol_ButtonActive]          = ImVec4(0.90f, 0.20f, 0.20f, 1.00f);
	style.Colors[ImGuiCol_Header]                = ImVec4(0.15f, 0.15f, 0.18f, 1.00f);
	style.Colors[ImGuiCol_HeaderHovered]         = ImVec4(0.20f, 0.20f, 0.25f, 1.00f);
	style.Colors[ImGuiCol_HeaderActive]          = ImVec4(0.90f, 0.20f, 0.20f, 1.00f);
	style.Colors[ImGuiCol_Separator]             = ImVec4(0.15f, 0.15f, 0.18f, 1.00f);

	// Geometry
	style.FrameBorderSize = 0.0f;
	style.WindowBorderSize = 1.0f;
	style.WindowRounding = 6.0f;
	style.ChildRounding = 4.0f;
	style.FrameRounding = 4.0f;
	style.PopupRounding = 4.0f;
	style.GrabRounding = 4.0f;
	style.ItemSpacing = ImVec2(8, 8);
	style.WindowPadding = ImVec2(0, 0); // Remove padding around the window itself

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
