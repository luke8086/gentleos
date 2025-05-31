// --------------------------------------------------------------------------------------
// Copyright (c) 2025-2026 luke8086
// Distributed under the terms of GPL-2 License
// --------------------------------------------------------------------------------------
// File: widget.c - Widget routines
// --------------------------------------------------------------------------------------

#include <gui.h>

void
gui_widget_draw(widget_st *widget)
{
    if (widget->draw) {
        widget->draw(widget);
    } else if (widget->type == WIDGET_TYPE_BUTTON) {
        gui_button_draw(widget);
    }
}
