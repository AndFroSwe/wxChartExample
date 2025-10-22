#include <wx/wx.h>

#include "ChartView.h"
#include "wx/sizer.h"

class ChartApp : public wxApp {
  bool OnInit() override;
};

wxIMPLEMENT_APP(ChartApp);

bool ChartApp::OnInit() {
  SetAppearance(Appearance::System);

  auto *mainsz = new wxBoxSizer(wxVERTICAL);
  auto *view = new ChartView(nullptr, wxID_ANY, "ChartApp");

  std::vector<double> xs(1000);
  std::vector<double> ys(xs.size());
  for (size_t i = 0; i < xs.size(); i++) {
    xs[i] = 0.01 * i;
    ys[i] = sin(xs[i]);
  }
  auto res = view->SetPlotData(xs, ys);
  if (!res) {
    wxMessageBox(wxString::Format("error setting plotdata: %s", res.error()));
    return false;
  }

  view->SetSizerAndFit(mainsz);
  view->Show(true);

  return wxApp::OnInit();
}
