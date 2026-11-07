#pragma once

#include <wx/panel.h>
#include <wx/string.h>

class wxTextCtrl;

namespace blaze {

class EditorPage final : public wxPanel {
public:
    EditorPage(wxWindow* parent, const wxString& filePath = wxString());

    bool LoadFromFile(const wxString& filePath);
    bool SaveToFile();
    bool SaveToFile(const wxString& filePath);

    [[nodiscard]] const wxString& GetFilePath() const noexcept { return m_filePath; }
    [[nodiscard]] wxString GetDisplayName() const;
    [[nodiscard]] bool IsDirty() const noexcept { return m_dirty; }

private:
    void OnTextChanged(wxCommandEvent& event);

    wxTextCtrl* m_editor{nullptr};
    wxString m_filePath;
    bool m_dirty{false};
};

} // namespace blaze
