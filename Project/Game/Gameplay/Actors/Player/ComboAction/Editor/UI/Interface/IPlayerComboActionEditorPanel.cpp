#include "IPlayerComboActionEditorPanel.h"

//============================================================================
//	IPlayerComboActionEditorPanel classMethods
//============================================================================

int PlayerComboPanelHelper::InputTextCallback_Resize(ImGuiInputTextCallbackData* data) {

	if (data->EventFlag != ImGuiInputTextFlags_CallbackResize) {
		return 0;
	}

	auto* str = static_cast<std::string*>(data->UserData);
	if (!str) {
		return 0;
	}

	str->resize(static_cast<size_t>(data->BufTextLen));
	data->Buf = str->data();
	return 0;
}

bool PlayerComboPanelHelper::InputTextStdString(const char* label, std::string* str, ImGuiInputTextFlags flags) {

	if (!str) {
		return false;
	}
	flags |= ImGuiInputTextFlags_CallbackResize;
	return ImGui::InputText(label, str->data(), str->capacity() + 1, flags, InputTextCallback_Resize, str);
}