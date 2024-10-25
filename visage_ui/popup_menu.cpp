/* Copyright Vital Audio, LLC
 *
 * visage is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * visage is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with visage.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "popup_menu.h"

#include "visage_graphics/theme.h"

namespace visage {
  THEME_COLOR(PopupMenuBackground, 0xff262a2e);
  THEME_COLOR(PopupMenuBorder, 0xff606265);
  THEME_COLOR(PopupMenuText, 0xffeeeeee);
  THEME_COLOR(PopupMenuSelection, 0xffaa88ff);
  THEME_COLOR(PopupMenuSelectionText, 0xffffffff);

  THEME_VALUE(PopupOptionHeight, 22.0f, ScaledHeight, true);
  THEME_VALUE(PopupMinWidth, 175.0f, ScaledHeight, true);
  THEME_VALUE(PopupTextPadding, 9.0f, ScaledHeight, true);
  THEME_VALUE(PopupFontSize, 15.0f, ScaledHeight, true);
  THEME_VALUE(PopupSelectionPadding, 4.0f, ScaledHeight, true);

  int PopupList::renderHeight() {
    int popup_height = paletteValue(kPopupOptionHeight);
    int selection_padding = paletteValue(kPopupSelectionPadding);
    return options_.size() * popup_height + 2 * selection_padding;
  }

  int PopupList::renderWidth() {
    int width = paletteValue(kPopupMinWidth);
    int x_padding = paletteValue(kPopupSelectionPadding) + paletteValue(kPopupTextPadding);
    for (const PopupOptions& option : options_) {
      int string_width = font_.stringWidth(option.name.c_str(), option.name.size()) + 2 * x_padding;
      width = std::max<int>(width, string_width);
    }

    return width;
  }

  int PopupList::yForIndex(int index) {
    return (paletteValue(kPopupSelectionPadding) + index * paletteValue(kPopupOptionHeight));
  }

  void PopupList::selectHoveredIndex() {
    if (hover_index_ >= 0 && hover_index_ < options_.size()) {
      if (!options_[hover_index_].sub_options.empty()) {
        for (Listener* listener : listeners_)
          listener->subMenuSelected(options_[hover_index_], yForIndex(hover_index_), this);
        menu_open_index_ = hover_index_;
      }
      else {
        for (Listener* listener : listeners_)
          listener->optionSelected(options_[hover_index_], this);
      }
    }
  }

  void PopupList::setHoverFromPosition(Point position) {
    int y = paletteValue(kPopupSelectionPadding);

    int option_height = paletteValue(kPopupOptionHeight);
    for (int i = 0; i < options_.size(); ++i) {
      if (!options_[i].is_break && position.y >= y && position.y < y + option_height) {
        hover_index_ = i;
        return;
      }

      y += option_height;
    }

    hover_index_ = -1;
  }

  void PopupList::selectFromPosition(Point position) {
    setHoverFromPosition(position + Point(0, yPosition()));
    selectHoveredIndex();
  }

  void PopupList::draw(Canvas& canvas) {
    static constexpr float kTriangleWidthRatio = 0.25f;
    int dpi_scale = canvas.dpiScale();

    QuadColor background = canvas.color(kPopupMenuBackground).withMultipliedAlpha(opacity_);
    QuadColor border = canvas.color(kPopupMenuBorder).withMultipliedAlpha(opacity_);
    canvas.setColor(background);
    canvas.roundedRectangle(0, 0, width(), height(), 8 * dpi_scale);

    canvas.setColor(border);
    canvas.roundedRectangleBorder(0, 0, width(), height(), 8 * dpi_scale, 1);

    canvas.setPaletteColor(kPopupMenuText);
    int selection_padding = paletteValue(kPopupSelectionPadding);
    int x_padding = selection_padding + paletteValue(kPopupTextPadding);
    int option_height = paletteValue(kPopupOptionHeight);
    int y = selection_padding - yPosition();

    QuadColor text = canvas.color(kPopupMenuText).withMultipliedAlpha(opacity_);
    QuadColor selected_text = canvas.color(kPopupMenuSelectionText).withMultipliedAlpha(opacity_);
    for (int i = 0; i < options_.size(); ++i) {
      if (y + option_height > 0 && y < height()) {
        if (options_[i].is_break)
          canvas.rectangle(x_padding, y + option_height / 2, width() - 2 * x_padding, 1);
        else {
          if (i == hover_index_) {
            QuadColor selected = canvas.color(kPopupMenuSelection).withMultipliedAlpha(opacity_);
            canvas.setColor(selected);
            canvas.roundedRectangle(selection_padding, y, width() - 2 * selection_padding,
                                    option_height, 4 * dpi_scale);
            canvas.setColor(selected_text);
          }
          else
            canvas.setColor(text);

          Font font(paletteValue(kPopupFontSize), font_.fontData());
          canvas.text(options_[i].name, font, Font::kLeft, x_padding, y, width(), option_height);

          if (!options_[i].sub_options.empty()) {
            int triangle_width = font.size() * kTriangleWidthRatio;
            int triangle_x = width() - x_padding - triangle_width;
            int triangle_y = y + option_height / 2 - triangle_width;
            canvas.triangleRight(triangle_x, triangle_y, triangle_width);
          }
        }
      }
      y += option_height;
    }
  }

  void PopupList::resized() {
    ScrollableFrame::resized();
    setScrollableHeight(renderHeight(), height());
  }

  void PopupList::mouseDown(const MouseEvent& e) {
    Point position = e.relativeTo(this).position;
    if (!isVisible() || !localBounds().contains(position))
      return;

    setHoverFromPosition(e.relativeTo(this).position + Point(0, yPosition()));

    if (hover_index_ < options_.size() && hover_index_ >= 0 && !options_[hover_index_].sub_options.empty())
      selectHoveredIndex();

    redraw();
  }

  void PopupList::mouseExit(const MouseEvent& e) {
    if (!isVisible())
      return;

    hover_index_ = menu_open_index_;
    for (Listener* listener : listeners_)
      listener->mouseMovedOnMenu(e.relativeTo(this).position, this);

    redraw();
  }

  void PopupList::mouseMove(const MouseEvent& e) {
    if (!isVisible())
      return;

    for (Listener* listener : listeners_)
      listener->mouseMovedOnMenu(e.relativeTo(this).position, this);

    redraw();
  }

  void PopupList::mouseDrag(const MouseEvent& e) {
    if (!isVisible())
      return;

    for (Listener* listener : listeners_)
      listener->mouseDraggedOnMenu(e.relativeTo(this).position, this);

    redraw();
  }

  void PopupList::mouseUp(const MouseEvent& e) {
    if (!isVisible())
      return;

    Point position = e.relativeTo(this).position;
    if (localBounds().contains(position) == 0) {
      for (Listener* listener : listeners_)
        listener->mouseUpOutside(position, this);
      return;
    }

    if (enable_mouse_up_)
      selectFromPosition(position);
    enable_mouse_up_ = true;

    redraw();
  }

  void PopupMenu::draw(Canvas& canvas) {
    float opacity = opacity_animation_.update();
    for (auto& list : lists_)
      list.setOpacity(opacity);

    if (opacity_animation_.isAnimating())
      redraw();
    else if (!opacity_animation_.isTargeting())
      setVisible(false);
  }

  void PopupMenu::showMenu(const PopupOptions& options, Bounds bounds,
                           std::function<void(int)> callback, std::function<void()> cancel) {
    if (bounds == last_bounds_ && isRunning()) {
      stopTimer();
      return;
    }
    last_bounds_ = bounds;

    callback_ = std::move(callback);
    cancel_ = std::move(cancel);
    for (int i = 1; i < kMaxSubMenus; ++i)
      lists_[i].setVisible(false);

    setListFonts(Font(paletteValue(kPopupFontSize), font_.fontData()));
    lists_[0].setOptions(options.sub_options);
    int h = std::min(height(), lists_[0].renderHeight());
    int w = lists_[0].renderWidth();

    int y = bounds.y();
    int x = bounds.x();
    int bottom = y + h;
    int right = x + w;
    if (bottom > height())
      y = std::max(0, bounds.y() - h);
    if (right > width())
      x = std::max(0, bounds.x() - w);

    for (auto& list : lists_) {
      list.resetOpenMenu();
      list.setNoHover();
    }

    lists_[0].setBounds(x, y, w, h);
    lists_[0].setVisible(true);
    lists_[0].redraw();
    setVisible(true);
    opacity_animation_.target(true, true);

    stopTimer();
    startTimer(kWaitForSelection);
    for (auto& list : lists_)
      list.enableMouseUp(false);

    requestKeyboardFocus();
    redraw();
  }

  void PopupMenu::focusChanged(bool is_focused, bool was_clicked) {
    if (!is_focused && isVisible()) {
      startTimer(1);
      opacity_animation_.target(false);
    }

    redraw();
  }

  void PopupMenu::timerCallback() {
    redraw();
    stopTimer();

    for (auto& list : lists_)
      list.enableMouseUp(true);

    if (hover_list_ && hover_index_ >= 0 && hover_index_ < hover_list_->numOptions()) {
      const PopupOptions& option = hover_list_->option(hover_index_);
      if (!option.sub_options.empty()) {
        subMenuSelected(option, hover_list_->hoverY(), hover_list_);
        return;
      }
    }

    int last_open_menu = kMaxSubMenus - 1;
    for (; last_open_menu > 0 && hover_list_ != &lists_[last_open_menu]; --last_open_menu) {
      lists_[last_open_menu].setVisible(false);
      lists_[last_open_menu].resetOpenMenu();
    }

    lists_[last_open_menu].resetOpenMenu();
    if (hover_index_ < 0)
      lists_[last_open_menu].setNoHover();
  }

  void PopupMenu::optionSelected(const PopupOptions& option, PopupList* list) {
    for (auto& sub_list : lists_)
      sub_list.setVisible(false);

    if (!isVisible()) {
      if (cancel_ != nullptr)
        cancel_();
      return;
    }

    setVisible(false);

    for (auto& sub_list : lists_) {
      sub_list.resetOpenMenu();
      sub_list.setNoHover();
    }
    if (callback_ != nullptr)
      callback_(option.id);

    redraw();
  }

  void PopupMenu::subMenuSelected(const PopupOptions& option, int selection_y, PopupList* list) {
    int source_index = 0;
    for (int i = 0; i < kMaxSubMenus; ++i) {
      if (list == &lists_[i])
        source_index = i;
    }

    lists_[source_index].setOpenMenu(lists_[source_index].hoverIndex());
    if (source_index < kMaxSubMenus - 1) {
      lists_[source_index + 1].setOptions(option.sub_options);
      int h = lists_[source_index + 1].renderHeight();
      int w = lists_[source_index + 1].renderWidth();
      int y = list->y() + selection_y;
      int bottom = y + h;
      int x = lists_[source_index].right();
      int right = x + w;
      if (bottom > height())
        y = height() - h;
      if (right > width())
        x = lists_[source_index].x() - w;

      lists_[source_index + 1].setBounds(x, y, w, h);
      lists_[source_index + 1].setNoHover();
      lists_[source_index + 1].setVisible(true);
    }
  }

  void PopupMenu::moveHover(Point position, const PopupList* list) {
    PopupList* last_hover_list = hover_list_;
    int last_hover_index = hover_index_;
    position += list->topLeft();
    hover_list_ = nullptr;
    hover_index_ = -1;
    for (auto& sub_list : lists_) {
      if (sub_list.isVisible() && sub_list.bounds().contains(position)) {
        sub_list.setHoverFromPosition(position - sub_list.topLeft() + Point(0, list->yPosition()));

        hover_list_ = &sub_list;
        hover_index_ = hover_list_->hoverIndex();
      }
    }
    if (hover_list_ != last_hover_list || hover_index_ != last_hover_index) {
      stopTimer();
      startTimer(kPauseMs);
    }
  }

  void PopupMenu::mouseUpOutside(Point position, PopupList* list) {
    position += list->topLeft();
    for (auto& sub_list : lists_) {
      if (sub_list.isVisible() && sub_list.bounds().contains(position)) {
        sub_list.selectFromPosition(position - sub_list.topLeft());
        return;
      }
    }

    if (isRunning())
      return;

    setVisible(false);
    if (cancel_ != nullptr)
      cancel_();
  }

  void ValueDisplay::showDisplay(const String& text, Bounds bounds, Font::Justification justification) {
    setVisible(true);

    text_ = text;

    Font font(paletteValue(kPopupFontSize), font_->fontData());
    int x_padding = paletteValue(kPopupSelectionPadding) + paletteValue(kPopupTextPadding);
    int width = font.stringWidth(text.c_str(), text.length()) + 2 * x_padding;
    int height = paletteValue(kPopupOptionHeight);
    int x = bounds.xCenter() - width / 2;
    int y = bounds.yCenter() - height / 2;
    if (justification & Font::kLeft)
      x = bounds.x() - width;
    else if (justification & Font::kRight)
      x = bounds.right();
    if (justification & Font::kBottom)
      y = bounds.bottom();
    else if (justification & Font::kTop)
      y = bounds.y() - height;

    setBounds(x, y, width, height);
    redraw();
  }

  void ValueDisplay::draw(Canvas& canvas) {
    Font font(canvas.value(kPopupFontSize), font_->fontData());
    float pixel_scale = canvas.dpiScale();
    canvas.setPaletteColor(kPopupMenuBackground);
    canvas.roundedRectangle(0, 0, width(), height(), 8 * pixel_scale);

    canvas.setPaletteColor(kPopupMenuBorder);
    canvas.roundedRectangleBorder(0, 0, width(), height(), 8 * pixel_scale, 1);

    canvas.setPaletteColor(kPopupMenuText);
    canvas.setColor(canvas.color(kPopupMenuText));
    canvas.text(text_, font, Font::kCenter, 0, 0, width(), height());
  }
}