#include "ChartView.h"
#include "expected.hpp"
#include "wx/affinematrix2d.h"
#include "wx/dcbuffer.h"
#include "wx/event.h"
#include "wx/geometry.h"
#include "wx/graphics.h"
#include <algorithm>
#include <array>
#include <format>

ChartView::ChartView(wxWindow *parent, wxWindowID id, const wxString &title)
    : wxFrame(parent, id, title), m_margins(), m_points(0), m_xMinmax(0, 0),
      m_yMinmax(0, 0), m_isResizing(false) {
  SetBackgroundStyle(wxBG_STYLE_PAINT); // Needed for windows

  // Set default margins
  auto res = SetMargins({.left = 0.1, .top = 0.1, .right = 0.1, .bottom = 0.1});
  assert(res && "Default margins are not in span!");

  CalculateTransforms(); // Make first calc on start

  // Bindings
  this->Bind(wxEVT_PAINT, &ChartView::OnPaint, this);
  this->Bind(wxEVT_SIZE, &ChartView::OnResize, this);

  m_timerResize.SetOwner(this);
  this->Bind(wxEVT_TIMER, &ChartView::OnResizeTimer, this,
             m_timerResize.GetId());
}

tl::expected<void, std::string>
ChartView::SetMargins(const chartview::margins &newMargins) {
  constexpr float minVal = 0.0;
  constexpr float maxVal = 0.5;

  if (newMargins.left < minVal || newMargins.left > maxVal) {
    return tl::make_unexpected(
        std::format("left margin {} outside of span [{}, {}]", newMargins.left,
                    minVal, maxVal));
  }
  if (newMargins.top < minVal || newMargins.top > maxVal) {
    return tl::make_unexpected(
        std::format("top margin {} outside of span [{}, {}]", newMargins.top,
                    minVal, maxVal));
  }
  if (newMargins.right < minVal || newMargins.right > maxVal) {
    return tl::make_unexpected(
        std::format("right margin {} outside of span [{}, {}]",
                    newMargins.right, minVal, maxVal));
  }
  if (newMargins.bottom < minVal || newMargins.bottom > maxVal) {
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

tl::expected<void, std::string>
ChartView::SetPlotData(const std::vector<double> &xs,
                       const std::vector<double> &ys) {
  if (xs.size() != ys.size()) {
    return tl::make_unexpected(std::format(
        "plot error: x/y size mismatch x={}, y={}", xs.size(), ys.size()));
  }

  if (xs.size() == 0 || ys.size() == 0) {
    return tl::make_unexpected("plot error: x/y size is 0. Use Clear instead");
  }

  std::vector<chartview::point> tmp(xs.size());
  for (size_t i = 0; i < xs.size(); ++i) {
    tmp[i] = {.x = xs.at(i), .y = ys.at(i)};
  }

  std::pair<double, double> _xmax;
  std::pair<double, double> _ymax;
  try {
    const auto [xmin, xmax] = std::ranges::minmax_element(xs);
    const auto [ymin, ymax] = std::ranges::minmax_element(ys);

    _xmax = {*xmin, *xmax};
    _ymax = {*ymin, *ymax};
  } catch (const std::exception &e) {
    return tl::make_unexpected(
        std::format("error getting minmax x and y: {}", e.what()));
  }

  m_points = std::move(tmp);
  m_xMinmax = std::move(_xmax);
  m_yMinmax = std::move(_ymax);

  return {};
}

void ChartView::Clear() {
  m_points.clear();
}

void ChartView::DrawPlot(wxAutoBufferedPaintDC &dc) {
  dc.Clear();
  std::unique_ptr<wxGraphicsContext> gc(wxGraphicsContext::Create(dc));
  assert(gc && "failed to create Graphicscontext");

  bool aaSupported = gc->SetAntialiasMode(wxAntialiasMode::wxANTIALIAS_DEFAULT);

  auto currentSize = this->GetSize();
  wxRect2DDouble fullArea(0, 0, static_cast<double>(currentSize.GetWidth()),
                          static_cast<double>(currentSize.GetHeight()));
  wxRect2DDouble plotArea = fullArea;
  // NOLINTBEGIN Ignore narrowing conversion warning
  plotArea.Inset(fullArea.GetSize().GetWidth() * m_margins.left,
                 fullArea.GetSize().GetHeight() * m_margins.top,
                 fullArea.GetSize().GetWidth() * m_margins.right,
                 fullArea.GetSize().GetHeight() * m_margins.bottom);
  // NOLINTEND

  gc->SetBrush(*wxWHITE_BRUSH);
  gc->SetPen(*wxBLACK_PEN);
  gc->DrawRectangle(plotArea);

  // Only draw graph when not resizing
  if (m_isResizing) {
    return;
  }

  // Draw axis
  auto [segs, newMin, newMax] = NiceLabels(m_yMinmax.first, m_yMinmax.second);

  if (segs > 1) {
    gc->SetPen(*wxGREY_PEN);
    for (size_t i = 0; i < segs; i++) {

      double y = plotArea.GetY() +
                 (plotArea.GetHeight() * (1.0 - (double)i / (segs - 1)));
      std::array<wxPoint2DDouble, 2> points{
          wxPoint2DDouble(plotArea.GetX(), y),
          wxPoint2DDouble(plotArea.GetRight(), y)};
      gc->StrokeLines(points.size(), points.data());
    }
  } else {
    newMin = m_yMinmax.first;
    newMax = m_yMinmax.second;
  }

  // Transform points to plot area
  wxAffineMatrix2D transformationMatrix;
  transformationMatrix.Translate(plotArea.GetX(),
                                 plotArea.GetY() + plotArea.GetHeight());
  transformationMatrix.Scale(plotArea.GetWidth() /
                                 (m_xMinmax.second - m_xMinmax.first),
                             plotArea.GetHeight() / (newMax - newMin));
  transformationMatrix.Scale(1, -1);
  transformationMatrix.Translate(-m_xMinmax.first, -m_yMinmax.first);

  wxPen plotPen;
  plotPen.SetColour(*wxBLUE);

  gc->SetPen(plotPen);
  gc->SetBrush(wxNullBrush);

  auto path = gc->CreatePath();
  for (auto &point : m_points) {
    double x = point.x;
    double y = point.y;
    transformationMatrix.TransformPoint(&x, &y);
    path.AddLineToPoint(x, y);
  }

  gc->DrawPath(path);
}

std::tuple<int, double, double> ChartView::NiceLabels(double origLow,
                                                      double origHigh) {
  constexpr std::array<double, 7> rangeMults{0.2, 0.25, 0.5, 1.0,
                                             2.0, 2.5,  5.0};
  constexpr int maxSegments = 6;

  double magnitude = std::floor(std::log10(origHigh - origLow));

  for (auto r : rangeMults) {
    double stepSize = r * std::pow(10.0, magnitude);
    double low = std::floor(origLow / stepSize) * stepSize;
    double high = std::ceil(origHigh / stepSize) * stepSize;

    auto segments = static_cast<int>(round((high - low) / stepSize));

    if (segments <= maxSegments) {
      return std::make_tuple(segments, low, high);
    }
  }

  // return some defaults in case rangeMults and maxSegments are mismatched
  return std::make_tuple(10, origLow, origHigh);
}

void ChartView::OnResizeTimer(wxTimerEvent & /*evt*/) {
  m_isResizing = false;
  Refresh();
}

void ChartView::OnResize(wxSizeEvent &evt) {
  m_isResizing = true;
  m_timerResize.StartOnce(100);

  evt.Skip();
}

void ChartView::OnPaint(wxPaintEvent &evt) {
  wxAutoBufferedPaintDC dc(this);
  DrawPlot(dc);

  evt.Skip();
}
