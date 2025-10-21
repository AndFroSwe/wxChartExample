#include <wx/wx.h>

#include "ChartView.h"
#include "wx/sizer.h"

class ChartApp : public wxApp {
  bool OnInit() override;
};

wxIMPLEMENT_APP(ChartApp);

bool ChartApp::OnInit() {
  SetAppearance(Appearance::System);

  auto view = new wxFrame(nullptr, wxID_ANY, "ChartApp");
  auto mainsz = new wxBoxSizer(wxVERTICAL);

  mainsz->Add(new ChartView(view, wxID_ANY), 1, wxEXPAND);

  view->SetSizerAndFit(mainsz);
  view->Show(true);

  return wxApp::OnInit();
}
