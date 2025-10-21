#include "ChartView.h"
#include "expected.hpp"
#include "wx/dcbuffer.h"
#include "wx/event.h"
#include "wx/geometry.h"
#include "wx/graphics.h"
#include <format>

ChartView::ChartView(wxWindow *parent, wxWindowID id) : wxWindow(parent, id) {
  SetBackgroundStyle(wxBG_STYLE_PAINT); // Needed for windows

  // Set default margins
  auto res = SetMargins({.left = 0.1, .top = 0.1, .right = 0.1, .bottom = 0.1});
  assert(res && "Default margins are not in span!");

  this->Bind(wxEVT_PAINT, &ChartView::OnPaint, this);
}

tl::expected<void, std::string>
ChartView::SetMargins(const chartview::margins &newMargins) {
  constexpr float minVal = 0.0;
  constexpr float maxVal = 0.5;

  if (newMargins.left < minVal || newMargins.left > maxVal) {
    return tl::make_unexpected(
        std::format("left margin {} outside of span [{}, {}]", newMargins.left,
                    minVal, maxVal));
  } else if (newMargins.top < minVal || newMargins.top > maxVal) {
    return tl::make_unexpected(
        std::format("top margin {} outside of span [{}, {}]", newMargins.top,
                    minVal, maxVal));
  } else if (newMargins.right < minVal || newMargins.right > maxVal) {
    return tl::make_unexpected(
        std::format("right margin {} outside of span [{}, {}]",
                    newMargins.right, minVal, maxVal));
  } else if (newMargins.bottom < minVal || newMargins.bottom > maxVal) {
    return tl::make_unexpected(
        std::format("bottom margin {} outside of span [{}, {}]",
                    newMargins.bottom, minVal, maxVal));
  }

  m_margins = newMargins;

  return {};
}

chartview::margins ChartView::GetMargins() const {
  return m_margins;
}

void ChartView::OnPaint(wxPaintEvent &evt) {
  wxAutoBufferedPaintDC dc(this);
  dc.Clear();
  std::unique_ptr<wxGraphicsContext> gc(wxGraphicsContext::Create(dc));

  auto currentSize = this->GetSize();

  wxRect2DDouble fullArea(0, 0, static_cast<double>(currentSize.GetWidth()),
                          static_cast<double>(currentSize.GetHeight()));
  wxRect2DDouble plotArea = fullArea;
  plotArea.Inset(fullArea.GetSize().GetWidth() * m_margins.left,
                 fullArea.GetSize().GetHeight() * m_margins.top,
                 fullArea.GetSize().GetWidth() * m_margins.right,
                 fullArea.GetSize().GetHeight() * m_margins.bottom);

  gc->SetBrush(*wxWHITE_BRUSH);
  gc->DrawRectangle(plotArea);
}
