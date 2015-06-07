#include "errorWindow.h"
#include <pebble.h>

static Window *sWindow;
static TextLayer *sMessage;

static void windowLoad(struct Window *window)
{
	GRect windowFrame = layer_get_frame(window_get_root_layer(window));

	sMessage = text_layer_create(GRect(windowFrame.origin.x + 10, windowFrame.origin.y + 10, windowFrame.size.w - 20, windowFrame.size.h - 20));
	text_layer_set_font(sMessage, fonts_get_system_font(FONT_KEY_GOTHIC_24_BOLD));
	text_layer_set_text(sMessage, (char *) window_get_user_data(window));
	layer_add_child(window_get_root_layer(window), text_layer_get_layer(sMessage));
}

static void windowUnload(struct Window *window)
{
	text_layer_destroy(sMessage);
}

void pushErrorWindow(char *message)
{
	window_set_user_data(sWindow, message);
	window_stack_push(sWindow, true);
}

void initErrorWindow(void)
{
	sWindow = window_create();
	window_set_window_handlers(sWindow, (WindowHandlers) {
		.load = windowLoad,
		.unload = windowUnload
	});
}

void deinitErrorWindow(void)
{
	window_destroy(sWindow);
}
