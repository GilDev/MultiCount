#include "addWindow.h"
#include <pebble.h>

#include "MultiCount.h"
#include "localization.h"
#include "errorWindow.h"

static Window *sWindow;
static GFont sFont;
static TextLayer *sNameTitle;
static TextLayer *sName;
static InverterLayer *sInverterLayer;

static char name[MAX_NAME_SIZE + 1];
static uint8_t position;

static void clickConfigProviderName(void *ctx);

/* ---------- NAME ---------- */

static GRect grectNameLayerForInverterLayer(void)
{
	GRect nameLayerFrame = layer_get_frame(text_layer_get_layer(sName));
	return GRect(nameLayerFrame.origin.x + position * CHAR_WIDTH, nameLayerFrame.origin.y, CHAR_WIDTH, nameLayerFrame.size.h);
}

static void finishedName()
{
	int i;
	for (i = MAX_NAME_SIZE - 1; i > 0 && name[i] == ' '; i--)
		if (name[i - 1] != ' ') {
			name[i] = '\0';
			break;
		}

	if (i == 0) {
		pushErrorWindow(lc(LC_ADD_ERROR_NO_NAME));
	} else {
		createCounter(name, 0);
		window_stack_pop(true);
	}
}

static void upButtonName(ClickRecognizerRef recognizer, void *ctx)
{
	switch (name[position]) {
		case 'a':
		case 'A':
			name[position] = ' ';
			break;
		case ' ':
		case '\0':
			name[position] = (position) ? 'z' : 'Z';
			break;
		default:
			name[position]--;
	}

	layer_mark_dirty(text_layer_get_layer(sName));
}

static void selectButtonName(ClickRecognizerRef recognizer, void *ctx)
{
	if (position < MAX_NAME_SIZE - 1) {
		position++;
		layer_set_frame(inverter_layer_get_layer(sInverterLayer), grectNameLayerForInverterLayer());
	} else {
		finishedName();
	}
}

static void downButtonName(ClickRecognizerRef recognizer, void *ctx)
{
	switch (name[position]) {
		case 'z':
		case 'Z':
			name[position] = ' ';
			break;
		case ' ':
		case '\0':
			name[position] = (position) ? 'a' : 'A';
			break;
		default:
			name[position]++;
	}

	layer_mark_dirty(text_layer_get_layer(sName));
}

static void backButtonName(ClickRecognizerRef recognizer, void *ctx)
{
	if (position > 0) {
		position--;
		layer_set_frame(inverter_layer_get_layer(sInverterLayer), grectNameLayerForInverterLayer());
	} else {
		window_stack_remove(sWindow, true);
	}
}

static void clickConfigProviderName(void *ctx)
{
	window_single_repeating_click_subscribe(BUTTON_ID_UP, CLICKS_INTERVAL_NAME, upButtonName);
	window_single_click_subscribe(BUTTON_ID_SELECT, selectButtonName);
	window_long_click_subscribe(BUTTON_ID_SELECT, 0, finishedName, NULL);
	window_single_repeating_click_subscribe(BUTTON_ID_DOWN, CLICKS_INTERVAL_NAME, downButtonName);
	window_single_click_subscribe(BUTTON_ID_BACK, backButtonName);
}

/* ---------- WINDOW ---------- */

static void windowLoad(struct Window *window)
{
	for (int i = 0; i < MAX_NAME_SIZE; i++)
		name[i] = ' ';
	name[MAX_NAME_SIZE] = '\0';

	position = 0;

	window_set_click_config_provider(sWindow, clickConfigProviderName);

	sNameTitle = text_layer_create(GRect(0, 30, 144, 50));
	text_layer_set_font(sNameTitle, fonts_get_system_font(FONT_KEY_GOTHIC_28_BOLD));
	text_layer_set_text_alignment(sNameTitle, GTextAlignmentCenter);
	text_layer_set_text(sNameTitle, lc(LC_ADD_NAME));
	layer_add_child(window_get_root_layer(window), text_layer_get_layer(sNameTitle));

	sName = text_layer_create(GRect(10, 80, 124, 25));
	text_layer_set_font(sName, sFont);
	text_layer_set_text(sName, name);
	layer_add_child(window_get_root_layer(window), text_layer_get_layer(sName));

	sInverterLayer = inverter_layer_create(grectNameLayerForInverterLayer());
	layer_add_child(window_get_root_layer(window), inverter_layer_get_layer(sInverterLayer));
}

static void windowUnload(struct Window *window)
{
	text_layer_destroy(sNameTitle);
	text_layer_destroy(sName);
	inverter_layer_destroy(sInverterLayer);
}

void pushAddWindow(void)
{
	if (counterNumber >= MAX_COUNTER_NUMBER) {
		pushErrorWindow(lc(LC_ADD_ERROR_NO_PLACE_LEFT));
	} else {
		window_stack_push(sWindow, true);
	}
}

void initAddWindow(void)
{
	sWindow = window_create();
	window_set_window_handlers(sWindow, (WindowHandlers) {
		.load = windowLoad,
		.unload = windowUnload
	});

	sFont = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_VERA_MONO_BOLD_20));
}

void deinitAddWindow(void)
{
	fonts_unload_custom_font(sFont);
	window_destroy(sWindow);
}
