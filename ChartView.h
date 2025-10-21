#pragma once

#include <wx/wx.h>

#include "expected.hpp"

namespace chartview {
struct margins {
  float left;
  float top;
  float right;
  float bottom;
};
} // namespace chartview

class ChartView : public wxWindow {
public:
  ChartView() = delete;
  ChartView(wxWindow *parent, wxWindowID id);

  tl::expected<void, std::string>
  SetMargins(const chartview::margins &newMargins);

  chartview::margins GetMargins() const;

private:
  chartview::margins m_margins;

  void OnPaint(wxPaintEvent &evt);
};
