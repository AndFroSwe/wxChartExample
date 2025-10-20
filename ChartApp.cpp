#include "wx/settings.h"
#include <wx/wx.h>

class ChartApp : public wxApp {
  bool OnInit() override;
};

wxIMPLEMENT_APP(ChartApp);

bool ChartApp::OnInit() {
  SetAppearance(Appearance::System);

  auto view = new wxFrame(nullptr, wxID_ANY, "ChartApp");
  view->Show(true);

  return wxApp::OnInit();
}
