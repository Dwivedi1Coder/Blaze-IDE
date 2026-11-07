#include "ui/MainFrame.h"

#include "ui/EditorPage.h"

#include <wx/dirdlg.h>
#include <wx/filedlg.h>
#include <wx/generic/dirctrl.h>
#include <wx/msgdlg.h>
#include <wx/notebook.h>
#include <wx/panel.h>
#include <wx/sizer.h>
#include <wx/splitter.h>
#include <wx/textctrl.h>

namespace blaze {

MainFrame::MainFrame()
    : wxFrame(nullptr, wxID_ANY, "Blaze-IDE", wxDefaultPosition, wxSize(1280, 820)) {
    SetMinSize(wxSize(900, 600));
    BuildMenus();
    BuildWorkspace();
    CreateStatusBar(2);
    SetStatusText("Ready", 0);
    SetStatusText("C++20 | Native desktop", 1);
    Centre();
}

void MainFrame::BuildMenus() {
    auto* fileMenu = new wxMenu();
    fileMenu->Append(IdNewFile, "&New File\tCtrl+N");
    fileMenu->Append(IdOpenFile, "&Open File...\tCtrl+O");
    fileMenu->Append(IdOpenFolder, "Open &Folder...");
    fileMenu->AppendSeparator();
    fileMenu->Append(IdSave, "&Save\tCtrl+S");
    fileMenu->Append(IdSaveAs, "Save &As...");
    fileMenu->Append(IdSaveAll, "Save A&ll");
    fileMenu->AppendSeparator();
    fileMenu->Append(wxID_EXIT, "E&xit\tAlt+F4");

    auto* editMenu = new wxMenu();
    editMenu->Append(wxID_UNDO, "&Undo\tCtrl+Z");
    editMenu->Append(wxID_REDO, "&Redo\tCtrl+Y");
    editMenu->AppendSeparator();
    editMenu->Append(wxID_CUT, "Cu&t\tCtrl+X");
    editMenu->Append(wxID_COPY, "&Copy\tCtrl+C");
    editMenu->Append(wxID_PASTE, "&Paste\tCtrl+V");
    editMenu->Append(wxID_SELECTALL, "Select &All\tCtrl+A");

    auto* helpMenu = new wxMenu();
    helpMenu->Append(wxID_ABOUT, "&About Blaze-IDE");

    auto* menuBar = new wxMenuBar();
    menuBar->Append(fileMenu, "&File");
    menuBar->Append(editMenu, "&Edit");
    menuBar->Append(helpMenu, "&Help");
    SetMenuBar(menuBar);

    Bind(wxEVT_MENU, &MainFrame::OnNewFile, this, IdNewFile);
    Bind(wxEVT_MENU, &MainFrame::OnOpenFile, this, IdOpenFile);
    Bind(wxEVT_MENU, &MainFrame::OnOpenFolder, this, IdOpenFolder);
    Bind(wxEVT_MENU, &MainFrame::OnSave, this, IdSave);
    Bind(wxEVT_MENU, &MainFrame::OnSaveAs, this, IdSaveAs);
    Bind(wxEVT_MENU, &MainFrame::OnSaveAll, this, IdSaveAll);
    Bind(wxEVT_MENU, &MainFrame::OnExit, this, wxID_EXIT);
    Bind(wxEVT_MENU, [this](wxCommandEvent&) {
        wxMessageBox("Blaze-IDE native shell\nVersion 0.1.0",
                     "About Blaze-IDE", wxOK | wxICON_INFORMATION, this);
    }, wxID_ABOUT);
}

void MainFrame::BuildWorkspace() {
    auto* outer = new wxSplitterWindow(this, wxID_ANY);
    m_explorer = new wxGenericDirCtrl(
        outer, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize,
        wxDIRCTRL_DIR_ONLY
    );

    auto* right = new wxSplitterWindow(outer, wxID_ANY);
    m_editorTabs = new wxNotebook(right, wxID_ANY);

    auto* bottom = new wxPanel(right);
    auto* bottomSizer = new wxBoxSizer(wxVERTICAL);
    m_output = new wxTextCtrl(
        bottom, wxID_ANY, "Blaze-IDE startup complete.\n",
        wxDefaultPosition, wxDefaultSize,
        wxTE_MULTILINE | wxTE_READONLY | wxTE_RICH2
    );
    bottomSizer->Add(m_output, 1, wxEXPAND);
    bottom->SetSizer(bottomSizer);

    right->SetMinimumPaneSize(100);
    right->SplitHorizontally(m_editorTabs, bottom, -190);
    right->SetSashGravity(0.78);

    outer->SetMinimumPaneSize(170);
    outer->SplitVertically(m_explorer, right, 250);
    outer->SetSashGravity(0.20);

    auto* rootSizer = new wxBoxSizer(wxVERTICAL);
    rootSizer->Add(outer, 1, wxEXPAND);
    SetSizer(rootSizer);

    CreateEditor();
}

EditorPage* MainFrame::CreateEditor(const wxString& filePath) {
    auto* page = new EditorPage(m_editorTabs, filePath);
    m_editorTabs->AddPage(page, page->GetDisplayName(), true);
    return page;
}

EditorPage* MainFrame::GetActiveEditor() const {
    if (m_editorTabs == nullptr || m_editorTabs->GetSelection() == wxNOT_FOUND) {
        return nullptr;
    }
    return dynamic_cast<EditorPage*>(m_editorTabs->GetPage(m_editorTabs->GetSelection()));
}

void MainFrame::OnNewFile(wxCommandEvent&) {
    CreateEditor();
    SetStatusText("New file", 0);
}

void MainFrame::OnOpenFile(wxCommandEvent&) {
    wxFileDialog dialog(this, "Open source file", wxEmptyString, wxEmptyString,
                        "C/C++ files (*.c;*.cc;*.cpp;*.h;*.hpp)|*.c;*.cc;*.cpp;*.h;*.hpp|All files (*.*)|*.*",
                        wxFD_OPEN | wxFD_FILE_MUST_EXIST);
    if (dialog.ShowModal() != wxID_OK) {
        return;
    }

    const wxString path = dialog.GetPath();
    for (size_t i = 0; i < m_editorTabs->GetPageCount(); ++i) {
        auto* page = dynamic_cast<EditorPage*>(m_editorTabs->GetPage(i));
        if (page != nullptr && page->GetFilePath() == path) {
            m_editorTabs->SetSelection(i);
            return;
        }
    }

    auto* page = CreateEditor();
    if (!page->LoadFromFile(path)) {
        m_editorTabs->DeletePage(m_editorTabs->GetSelection());
        wxMessageBox("The selected file could not be read.", "Open File",
                     wxOK | wxICON_ERROR, this);
        return;
    }
    m_editorTabs->SetPageText(m_editorTabs->GetSelection(), page->GetDisplayName());
    LogMessage("Opened: " + path);
}

void MainFrame::OnOpenFolder(wxCommandEvent&) {
    wxDirDialog dialog(this, "Open project folder", wxEmptyString,
                       wxDD_DEFAULT_STYLE | wxDD_DIR_MUST_EXIST);
    if (dialog.ShowModal() != wxID_OK) {
        return;
    }
    m_explorer->SetPath(dialog.GetPath());
    SetTitle("Blaze-IDE — " + dialog.GetPath());
    LogMessage("Project folder: " + dialog.GetPath());
}

void MainFrame::OnSave(wxCommandEvent&) {
    SaveEditor(GetActiveEditor(), false);
}

void MainFrame::OnSaveAs(wxCommandEvent&) {
    SaveEditor(GetActiveEditor(), true);
}

bool MainFrame::SaveEditor(EditorPage* editor, bool promptForPath) {
    if (editor == nullptr) {
        return false;
    }

    wxString path = editor->GetFilePath();
    if (promptForPath || path.empty()) {
        wxFileDialog dialog(this, "Save file", wxEmptyString, editor->GetDisplayName(),
                            "C/C++ files (*.c;*.cc;*.cpp;*.h;*.hpp)|*.c;*.cc;*.cpp;*.h;*.hpp|All files (*.*)|*.*",
                            wxFD_SAVE | wxFD_OVERWRITE_PROMPT);
        if (dialog.ShowModal() != wxID_OK) {
            return false;
        }
        path = dialog.GetPath();
    }

    if (!editor->SaveToFile(path)) {
        wxMessageBox("The file could not be saved.", "Save File",
                     wxOK | wxICON_ERROR, this);
        return false;
    }

    const int index = m_editorTabs->GetPageIndex(editor);
    if (index != wxNOT_FOUND) {
        m_editorTabs->SetPageText(static_cast<size_t>(index), editor->GetDisplayName());
    }
    LogMessage("Saved: " + path);
    return true;
}

void MainFrame::OnSaveAll(wxCommandEvent&) {
    for (size_t i = 0; i < m_editorTabs->GetPageCount(); ++i) {
        auto* page = dynamic_cast<EditorPage*>(m_editorTabs->GetPage(i));
        if (page != nullptr && page->IsDirty() && !SaveEditor(page, false)) {
            break;
        }
    }
}

void MainFrame::OnExit(wxCommandEvent&) {
    Close(true);
}

void MainFrame::LogMessage(const wxString& message) {
    if (m_output != nullptr) {
        m_output->AppendText(message + "\n");
    }
}

} // namespace blaze
