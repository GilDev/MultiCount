#include "addWindow.h"
#include <pebble.h>

#include "MultiCount.h"
#include "errorWindow.h"

static Window *sWindow;
static GFont sFont;
static TextLayer *sNameTitle;
static TextLayer *sName;
static TextLayer *sValueTitle;
static TextLayer *sValue;
static InverterLayer *sInverterLayer;

static char name[MAX_NAME_SIZE + 1];
static uint8_t position;

static uint16_t value;
static char valueStr[MAX_COUNTER_DIGITS];

static void renderValue();
static void clickConfigProviderValue(void *ctx);
static void clickConfigProviderName(void *ctx);

/* ---------- NAME ---------- */

static GRect grectNameLayerForInverterLayer(void)
{
	GRect nameLayerFrame = layer_get_frame(text_layer_get_layer(sName));
	return GRect(nameLayerFrame.origin.x + position * CHAR_WIDTH, nameLayerFrame.origin.y, CHAR_WIDTH, nameLayerFrame.size.h);
}

static void finishedName()
{
	window_set_click_config_provider(sWindow, clickConfigProviderValue);
	renderValue();
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

/* ---------- VALUE ---------- */

static GRect grectValueLayerForInverterLayer(void)
{
	GRect valueLayerFrame = layer_get_frame(text_layer_get_layer(sValue));
	int16_t textLayerContentWidth = text_layer_get_content_size(sValue).w;
	return GRect(valueLayerFrame.origin.x + (valueLayerFrame.size.w - textLayerContentWidth) / 2, valueLayerFrame.origin.y, textLayerContentWidth, valueLayerFrame.size.h);
}

static void renderValue(void)
{
	snprintf(valueStr, MAX_COUNTER_VALUE, "%d", value);
	layer_mark_dirty(text_layer_get_layer(sValue));
	layer_set_frame(inverter_layer_get_layer(sInverterLayer), grectValueLayerForInverterLayer());
}

static void upButtonValue(ClickRecognizerRef recognizer, void *ctx)
{
	if (value < MAX_COUNTER_VALUE) {
		value++;
		renderValue();
	}
}

static void selectButtonValue(ClickRecognizerRef recognizer, void *ctx)
{
	int i;
	for (i = MAX_NAME_SIZE - 1; i > 0 && name[i] == ' '; i--)
		if (name[i - 1] != ' ') {
			name[i] = '\0';
			break;
		}

	if (i == 0) {
		pushErrorWindow("Oops!\n\nYou should specify a name");
	} else {
		createCounter(name, value);
		window_stack_pop(true);
	}
}

static void downButtonValue(ClickRecognizerRef recognizer, void *ctx)
{
	if (value > 0) {
		value--;
		renderValue();		
	}
}

static void backButtonValue(ClickRecognizerRef recognizer, void *ctx)
{
	window_set_click_config_provider(sWindow, clickConfigProviderName);
	layer_set_frame(inverter_layer_get_layer(sInverterLayer), grectNameLayerForInverterLayer());
}

static void clickConfigProviderValue(void *ctx)
{
	window_single_repeating_click_subscribe(BUTTON_ID_UP, CLICKS_INTERVAL_VALUE, upButtonValue);
	window_single_click_subscribe(BUTTON_ID_SELECT, selectButtonValue);
	window_single_repeating_click_subscribe(BUTTON_ID_DOWN, CLICKS_INTERVAL_VALUE, downButtonValue);
	window_single_click_subscribe(BUTTON_ID_BACK, backButtonValue);
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

	value = 0;
	position = 0;

	window_set_click_config_provider(sWindow, clickConfigProviderName);

	sNameTitle = text_layer_create(GRect(0, 0, 144, 50));
	text_layer_set_font(sNameTitle, fonts_get_system_font(FONT_KEY_GOTHIC_28_BOLD));
	text_layer_set_text_alignment(sNameTitle, GTextAlignmentCenter);
	text_layer_set_text(sNameTitle, "Name:");
	layer_add_child(window_get_root_layer(window), text_layer_get_layer(sNameTitle));

	sName = text_layer_create(GRect(10, 40, 124, 25));
	text_layer_set_font(sName, sFont);
	text_layer_set_text(sName, name);
	layer_add_child(window_get_root_layer(window), text_layer_get_layer(sName));

	sValueTitle = text_layer_create(GRect(0, 70, 144, 50));
	text_layer_set_font(sValueTitle, fonts_get_system_font(FONT_KEY_GOTHIC_28_BOLD));
	text_layer_set_text_alignment(sValueTitle, GTextAlignmentCenter);
	text_layer_set_text(sValueTitle, "Initial Value:");
	layer_add_child(window_get_root_layer(window), text_layer_get_layer(sValueTitle));

	sValue = text_layer_create(GRect(10, 110, 124, 25));
	text_layer_set_font(sValue, sFont);
	text_layer_set_text(sValue, valueStr);
	text_layer_set_text_alignment(sValue, GTextAlignmentCenter);
	snprintf(valueStr, MAX_COUNTER_DIGITS, "%d", value);
	layer_add_child(window_get_root_layer(window), text_layer_get_layer(sValue));

	sInverterLayer = inverter_layer_create(grectNameLayerForInverterLayer());
	layer_add_child(window_get_root_layer(window), inverter_layer_get_layer(sInverterLayer));
}

static void windowUnload(struct Window *window)
{
	text_layer_destroy(sNameTitle);
	text_layer_destroy(sName);
	text_layer_destroy(sValueTitle);
	text_layer_destroy(sValue);
	inverter_layer_destroy(sInverterLayer);
}

void pushAddWindow(void)
{
	if (counterNumber >= COUNTER_NUMBER) {
		pushErrorWindow("Oops!\n\nYou cannot add more counters");
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
