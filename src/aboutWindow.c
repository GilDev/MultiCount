#include "aboutWindow.h"
#include <pebble.h>

#include "MultiCount.h"

static Window *sWindow;
static ScrollLayer *sScrollLayer;
static TextLayer *sTextLayer;

static char aboutMessage[] =
"Welcome to MultiCount V" APP_VERSION "!\n\
\n\
This app allows you to setup simple counters to count anything you want, from the number of coffee you drink a week to the number of times you say \"Hello\" a day.\n \
\n\
Press SELECT to increment\n\
Hold SELECT to decrement\n\
\n\
By GilDev,\n\
@GilDev on Twitter, feel free to send me any feedback!\n\
Under MIT license";

/* ---------- WINDOW ---------- */

void windowLoad(struct Window *window)
{
	Layer *windowLayer = window_get_root_layer(window);
	GRect windowFrame = layer_get_frame(windowLayer);

	sTextLayer = text_layer_create(GRect(windowFrame.origin.x + 5, 0, windowFrame.size.w - 10, 2000));
	text_layer_set_font(sTextLayer, fonts_get_system_font(FONT_KEY_GOTHIC_24_BOLD));
	text_layer_set_text(sTextLayer, aboutMessage);
	GSize textSize = text_layer_get_content_size(sTextLayer);
	text_layer_set_size(sTextLayer, textSize);

	sScrollLayer = scroll_layer_create(windowFrame);
	scroll_layer_add_child(sScrollLayer, text_layer_get_layer(sTextLayer));
	textSize.h += 10;
	scroll_layer_set_content_size(sScrollLayer, textSize);
	scroll_layer_set_click_config_onto_window(sScrollLayer, window);
	layer_add_child(windowLayer, scroll_layer_get_layer(sScrollLayer));
}

void windowUnload(struct Window *window)
{
	text_layer_destroy(sTextLayer);
	scroll_layer_destroy(sScrollLayer);
}

void pushAboutWindow(void)
{
	window_stack_push(sWindow, true);
}

void initAboutWindow(void)
{
	sWindow = window_create();
	window_set_window_handlers(sWindow, (WindowHandlers) {
		.load = windowLoad,
		.unload = windowUnload
	});
}

void deinitAboutWindow(void)
{
	window_destroy(sWindow);
}
