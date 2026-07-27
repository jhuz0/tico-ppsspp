#pragma once

#include "Common/CommonTypes.h"
#include "tico/PpssppTicoConfig.h"
#include "tico/TicoConfig.h"
#include "tico/TicoCoreConfig.h"

#include <array>
#include <cstddef>
#include <string>
#include <vector>

struct ImGuiContext;
struct ImDrawList;
struct ImVec2;

namespace Draw {
class DrawContext;
class Texture;
}

namespace Tico {

enum class OverlayAction {
	None,
	SaveState,
	LoadState,
	Reset,
	LoadCheats,
	ToggleCheat,
	ReloadCoreConfig,
};

struct OverlayCommand {
	OverlayAction action = OverlayAction::None;
	int slot = 0;
};

enum class CheatMenuEntryKind {
	Cheat,
	Section,
	Tip,
};

// A core-config key the quick menu can edit. Values are written straight back to
// ppsspp.jsonc, so anything listed here stops needing a text editor.
enum class SettingKind {
	Choice,        // fixed list of values
	Number,        // integer with min/max/step
	Text,          // free text via the system keyboard
	ServerChoice,  // populated at runtime from the ad hoc server list
	Action,        // runs something instead of holding a value
};

struct CoreSetting {
	const char *key;
	const char *labelKey;
	SettingKind kind;
	const char *const *choices = nullptr;
	int choiceCount = 0;
	int minValue = 0;
	int maxValue = 0;
	int step = 1;
	// Read once when the session starts, so changing it mid-game does nothing
	// until the game is relaunched. Flagged in the UI rather than hidden.
	bool needsRestart = false;
};

enum class ChatAlertPosition {
	TopLeft,
	TopRight,
	BottomLeft,
	BottomRight,
};

// A chat line that briefly slides in while playing, so messages are not missed
// without opening the menu. Drawn even when the overlay itself is hidden.
struct ChatNotification {
	std::string text;
	float timer = 0.0f;
	float duration = 5.0f;
	float slideIn = 0.3f;
	float slideOut = 0.3f;
};

struct CheatMenuEntry {
	std::string name;
	bool enabled = false;
	bool toggleable = true;
	int sourceIndex = -1;
	CheatMenuEntryKind kind = CheatMenuEntryKind::Cheat;
};

class Overlay {
public:
	bool Init(Draw::DrawContext *draw, const char *gamePath, LogCallback log = {});
	void Shutdown();

	bool HandleInput(u64 buttons, u64 pressed, int leftStickX, int leftStickY, int rightStickX, int rightStickY);
	void Render(Draw::DrawContext *draw);
	void SetSaveStateInfo(int currentSlot, const std::array<bool, Ppsspp::SaveStateSlotCount> &slotInUse);
	void SetCheatsEnabled(bool enabled);
	void SetCheatInfo(bool enabled, bool available, const std::vector<CheatMenuEntry> &entries);
	// Chat is only offered when the core config enables it and ad hoc is up.
	void SetChatEnabled(bool enabled);
	// Seconds a message stays on screen, and which corner it slides in from.
	void SetChatAlertStyle(float durationSeconds, ChatAlertPosition position);
	void SetChatLog(std::vector<std::string> lines);
	void PushChatNotification(const std::string &line);
	void ReloadDisplaySettings();

	bool IsReady() const { return ready_; }
	bool IsVisible() const { return visible_; }
	bool ShouldExitGame() const { return exitRequested_; }
	void ClearExitRequest() { exitRequested_ = false; }
	OverlayCommand ConsumeCommand();
	void SetVisible(bool visible);

private:
	enum class Menu {
		Quick,
		SaveStates,
		Cheats,
		Settings,
		Chat,
		Display,
		Controls,
		Online,
	};

	int ItemCount() const;
	int QuickMenuStorageIndex(int visibleIndex) const;
	void UpdateBattery(float deltaTime);
	void DrawUI(float width, float height, float deltaTime);
	void DrawBackground(::ImDrawList *drawList, ::ImVec2 displaySize, float ease);
	void DrawTitle(::ImDrawList *drawList, ::ImVec2 displaySize, float scale, float ease);
	void DrawMenu(::ImDrawList *drawList, ::ImVec2 displaySize, float scale, float ease);
	void DrawSocialArea(::ImDrawList *drawList, ::ImVec2 displaySize, float scale, float ease);
	void DrawHelpers(::ImDrawList *drawList, ::ImVec2 displaySize, float scale, float ease);
	void DrawStatus(::ImDrawList *drawList, ::ImVec2 displaySize, float scale, float ease, float deltaTime);
	void DrawRAAlerts(Draw::DrawContext *draw, ::ImDrawList *drawList, ::ImVec2 displaySize, float scale, float deltaTime);
	void DrawChat(::ImDrawList *drawList, ::ImVec2 displaySize, float scale, float ease);
	void DrawSettingsList(::ImDrawList *drawList, ::ImVec2 displaySize, float scale, float ease);
	const CoreSetting *CurrentSettingTable(int *count) const;
	int SettingStorageIndex(int visibleIndex) const;
	int VisibleSettingCount() const;
	std::string SettingValue(const CoreSetting &setting) const;
	void CycleCoreSetting(int direction);
	void EditCoreSettingText(const CoreSetting &setting);
	void RefreshServerChoices();
	void AddCustomServer();
	void RemoveSelectedCustomServer();
	bool SelectedServerIsCustom() const;
	Menu ParentMenu(Menu menu) const;
	void DrawChatAlerts(::ImDrawList *drawList, ::ImVec2 displaySize, float scale, float deltaTime);
	void OpenChatComposer();
	void CycleSetting(int direction);
	void ApplyDisplaySettings(bool save);
	void LoadSocial(Draw::DrawContext *draw);
	bool LoadAvatarTextureFromFile(Draw::DrawContext *draw, const char *path);
	bool LoadAvatarTextureFromMemory(Draw::DrawContext *draw, const unsigned char *data, size_t size, const char *tag);
	void ReleaseAvatarTexture();
	Draw::Texture *LoadRAIconTexture(Draw::DrawContext *draw);
	Draw::Texture *LoadLoaderTexture(Draw::DrawContext *draw);
	void ReleaseRAIconTexture();
	void ReleaseLoaderTexture();
	std::string TitleText() const;
	void ExecuteSelection();

	LogCallback log_;
	bool ready_ = false;
	bool visible_ = false;
	bool comboDown_ = false;
	bool exitRequested_ = false;
	int selection_ = 0;
	int settingsSelection_ = 0;
	Menu menu_ = Menu::Quick;
	OverlayAction saveStateMode_ = OverlayAction::SaveState;
	DisplaySettings displaySettings_;
	OverlayCommand pendingCommand_;
	int currentStateSlot_ = 0;
	std::array<bool, Ppsspp::SaveStateSlotCount> slotInUse_{};
	bool cheatsEnabled_ = false;
	bool cheatsAvailable_ = false;
	bool cheatsLoading_ = false;
	bool cheatsLoadCommandSent_ = false;
	int cheatsLoadingDelayFrames_ = 0;
	std::vector<CheatMenuEntry> cheats_;
	bool chatEnabled_ = false;
	std::vector<std::string> serverHosts_;
	std::vector<std::string> serverLabels_;
	std::vector<bool> serverIsCustom_;
	int settingsScroll_ = 0;
	float chatAlertDuration_ = 5.0f;
	ChatAlertPosition chatAlertPosition_ = ChatAlertPosition::BottomLeft;
	std::vector<std::string> chatLog_;
	std::vector<ChatNotification> chatNotifications_;
	int chatScroll_ = 0;
	u64 nextChatNavMs_ = 0;
	u64 lastAnalogNavMs_ = 0;
	u64 nextCheatVerticalNavMs_ = 0;
	u64 nextCheatHorizontalNavMs_ = 0;
	int cheatVerticalNavDir_ = 0;
	int cheatHorizontalNavDir_ = 0;
	float animTimer_ = 0.0f;
	float loaderTimer_ = 0.0f;
	std::string title_;
	std::string nickname_ = "Player 1";
	Draw::Texture *avatarTexture_ = nullptr;
	Draw::Texture *raIconTexture_ = nullptr;
	Draw::Texture *loaderTexture_ = nullptr;
	int avatarWidth_ = 0;
	int avatarHeight_ = 0;
	ImGuiContext *context_ = nullptr;
	bool psmReady_ = false;
	u32 batteryLevel_ = 100;
	bool charging_ = false;
	float batteryTimer_ = 0.0f;
};

}  // namespace Tico
