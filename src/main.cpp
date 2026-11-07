#include <wx/wx.h>

#include "ui/MainFrame.h"

class BlazeApp final : public wxApp {
public:
    bool OnInit() override {
        if (!wxApp::OnInit()) {
            return false;
        }

        SetAppName("Blaze-IDE");
        SetVendorName("Blaze-IDE");

        auto* frame = new blaze::MainFrame();
        frame->Show();
        return true;
    }
};

wxIMPLEMENT_APP(BlazeApp);
