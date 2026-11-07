#include "ui/EditorPage.h"

#include <fstream>
#include <iterator>
#include <string>

#include <wx/filename.h>
#include <wx/font.h>
#include <wx/textctrl.h>

namespace blaze {

EditorPage::EditorPage(wxWindow* parent, const wxString& filePath)
    : wxPanel(parent), m_filePath(filePath) {
    auto* sizer = new wxBoxSizer(wxVERTICAL);

    m_editor = new wxTextCtrl(
        this,
        wxID_ANY,
        wxEmptyString,
        wxDefaultPosition,
        wxDefaultSize,
        wxTE_MULTILINE | wxTE_RICH2 | wxTE_DONTWRAP | wxHSCROLL
    );

    wxFont editorFont(11, wxFONTFAMILY_TELETYPE, wxFONTSTYLE_NORMAL,
                      wxFONTWEIGHT_NORMAL);
    m_editor->SetFont(editorFont);

    sizer->Add(m_editor, 1, wxEXPAND);
    SetSizer(sizer);

    m_editor->Bind(wxEVT_TEXT, &EditorPage::OnTextChanged, this);

    if (!m_filePath.empty()) {
        LoadFromFile(m_filePath);
    }
}

bool EditorPage::LoadFromFile(const wxString& filePath) {
    const std::filesystem::path nativePath(filePath.ToStdWstring());
    std::ifstream input(nativePath, std::ios::binary);
    if (!input) {
        return false;
    }

    const std::string contents((std::istreambuf_iterator<char>(input)),
                               std::istreambuf_iterator<char>());
    m_editor->ChangeValue(wxString::FromUTF8(contents.data(), contents.size()));
    m_filePath = filePath;
    m_dirty = false;
    return true;
}

bool EditorPage::SaveToFile() {
    if (m_filePath.empty()) {
        return false;
    }
    return SaveToFile(m_filePath);
}

bool EditorPage::SaveToFile(const wxString& filePath) {
    const std::filesystem::path nativePath(filePath.ToStdWstring());
    std::ofstream output(nativePath, std::ios::binary | std::ios::trunc);
    if (!output) {
        return false;
    }

    const wxScopedCharBuffer utf8 = m_editor->GetValue().ToUTF8();
    if (utf8.data() == nullptr) {
        return false;
    }

    output.write(utf8.data(), static_cast<std::streamsize>(utf8.length()));
    if (!output.good()) {
        return false;
    }

    m_filePath = filePath;
    m_dirty = false;
    return true;
}

wxString EditorPage::GetDisplayName() const {
    if (m_filePath.empty()) {
        return "Untitled";
    }
    return wxFileName(m_filePath).GetFullName();
}

void EditorPage::OnTextChanged(wxCommandEvent& event) {
    m_dirty = true;
    event.Skip();
}

} // namespace blaze
