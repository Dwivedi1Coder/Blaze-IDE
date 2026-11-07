#pragma once

#include <wx/frame.h>

class wxGenericDirCtrl;
class wxNotebook;
class wxTextCtrl;

namespace blaze {

class EditorPage;

class MainFrame final : public wxFrame {
public:
    MainFrame();

private:
    enum : int {
        IdNewFile = wxID_HIGHEST + 1,
        IdOpenFile,
        IdOpenFolder,
        IdSave,
        IdSaveAs,
        IdSaveAll
    };

    void BuildMenus();
    void BuildWorkspace();

    void OnNewFile(wxCommandEvent& event);
    void OnOpenFile(wxCommandEvent& event);
    void OnOpenFolder(wxCommandEvent& event);
    void OnSave(wxCommandEvent& event);
    void OnSaveAs(wxCommandEvent& event);
    void OnSaveAll(wxCommandEvent& event);
    void OnExit(wxCommandEvent& event);

    EditorPage* CreateEditor(const wxString& filePath = wxString());
    EditorPage* GetActiveEditor() const;
    bool SaveEditor(EditorPage* editor, bool promptForPath);
    void LogMessage(const wxString& message);

    wxGenericDirCtrl* m_explorer{nullptr};
    wxNotebook* m_editorTabs{nullptr};
    wxTextCtrl* m_output{nullptr};
};

} // namespace blaze
