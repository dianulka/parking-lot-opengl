#pragma once

namespace parking::ui {

/// Rozmiar przycisku otwierającego panel „Ustawienia” w lewym górnym rogu (px, framebuffer).
constexpr float kCornerPanelPx = 64.0f;
/// Odstęp od krawędzi okna — musi być zgodny z rysowaniem w Renderer::drawHudCornerOverlay.
constexpr float kCornerMarginPx = 8.0f;

}  // namespace parking::ui
