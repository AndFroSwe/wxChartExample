#pragma once

#include <wx/wx.h>

#include "expected.hpp"
#include "wx/dcbuffer.h"

namespace chartview {
struct margins {
  float left;
  float top;
  float right;
  float bottom;
};

struct point {
  double x;
  double y;
};
} // namespace chartview

class ChartView : public wxFrame {
public:
  ChartView() = delete;
  ChartView(wxWindow *parent, wxWindowID id, const wxString &title);

  tl::expected<void, std::string>
  SetMargins(const chartview::margins &newMargins);

  [[nodiscard]] chartview::margins GetMargins() const;

  tl::expected<void, std::string> SetPlotData(const std::vector<double> &xs,
                                              const std::vector<double> &ys);
  void Clear();

private:
  chartview::margins m_margins;

  std::vector<chartview::point> m_points;
  std::pair<double, double> m_x_minmax;
  std::pair<double, double> m_y_minmax;

  void OnPaint(wxPaintEvent &evt);
  void DrawPlot(wxAutoBufferedPaintDC &dc);
};
