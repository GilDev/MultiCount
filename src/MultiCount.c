#include "MultiCount.h"
#include <pebble.h>

#include "localization.h"
#include "addWindow.h"
#include "deleteWindow.h"
#include "aboutWindow.h"
#include "errorWindow.h"

static Window *sWindow;
static MenuLayer *sMenuLayer;

static GBitmap *sAddIcon;
static GBitmap *sDeleteIcon;
static GBitmap *sInfoIcon;

/* ---------- COUNTERS ---------- */

void createCounter(const char *name, uint16_t value)
{
	struct Counter *newCounter = malloc(sizeof *newCounter);

	strcpy(newCounter->name, name);
	newCounter->value = value;

	counters[counterNumber++] = newCounter;
}

/* ---------- MENU ---------- */

static uint16_t menuGetNumRows(struct MenuLayer *menuLayer, uint16_t section_index, void *callbackContext)
{
	return counterNumber + 3;
}

static void menuDrawRow(GContext *ctx, const Layer *cellLayer, MenuIndex *cellIndex, void *callbackContext)
{
	if (cellIndex->row == counterNumber)
		menu_cell_basic_draw(ctx, cellLayer, lc(LC_ADD), NULL, sAddIcon);
	else if (cellIndex->row == counterNumber + 1)
		menu_cell_basic_draw(ctx, cellLayer, lc(LC_DELETE), NULL, sDeleteIcon);
	else if (cellIndex->row == counterNumber + 2)
		menu_cell_basic_draw(ctx, cellLayer, lc(LC_ABOUT), NULL, sInfoIcon);
	else {
		char valueStr[MAX_NAME_SIZE + 1];
		snprintf(valueStr, MAX_NAME_SIZE + 1, "%d", counters[cellIndex->row]->value);
		menu_cell_basic_draw(ctx, cellLayer, counters[cellIndex->row]->name, valueStr, NULL);
	}
}

static void menuSelectClick(struct MenuLayer *menuLayer, MenuIndex *cellIndex, void *callbackContext)
{
	if (cellIndex->row == counterNumber) {
		pushAddWindow();
		menu_layer_reload_data(menuLayer);
	}
	else if (cellIndex->row == counterNumber + 1) {
		pushDeleteWindow();
		menu_layer_set_selected_index(menuLayer, (MenuIndex) {.section = cellIndex->section, .row = 0}, MenuRowAlignCenter, false);
	}
	else if (cellIndex->row == counterNumber + 2)
		pushAboutWindow();
	else
		if (counters[cellIndex->row]->value < 65535) {
			++counters[cellIndex->row]->value;
			menu_layer_reload_data(sMenuLayer);
		}
}

static void menuSelectLongClick(struct MenuLayer *menuLayer, MenuIndex *cellIndex, void *callbackContext)
{
	if (cellIndex->row < counterNumber && counters[cellIndex->row]->value > 0) {
		--counters[cellIndex->row]->value;
		menu_layer_reload_data(sMenuLayer);
	}
}

/* ---------- WINDOW ---------- */

static void windowLoad(Window *window)
{
	Layer *windowLayer = window_get_root_layer(window);

	sAddIcon = gbitmap_create_with_resource(RESOURCE_ID_ADD_ICON);
	sDeleteIcon = gbitmap_create_with_resource(RESOURCE_ID_DELETE_ICON);
	sInfoIcon = gbitmap_create_with_resource(RESOURCE_ID_INFO_ICON);

	sMenuLayer = menu_layer_create(layer_get_bounds(windowLayer));

	menu_layer_set_click_config_onto_window(sMenuLayer, window);

	menu_layer_set_callbacks(sMenuLayer, NULL, (MenuLayerCallbacks) {
		.get_num_rows = (MenuLayerGetNumberOfRowsInSectionsCallback) menuGetNumRows,
		.draw_row = (MenuLayerDrawRowCallback) menuDrawRow,
		.select_click = (MenuLayerSelectCallback) menuSelectClick,
		.select_long_click = (MenuLayerSelectCallback) menuSelectLongClick
	});

	layer_add_child(windowLayer, menu_layer_get_layer(sMenuLayer));
}

static void windowUnload(Window *window)
{
	gbitmap_destroy(sAddIcon);
	gbitmap_destroy(sDeleteIcon);
	gbitmap_destroy(sInfoIcon);
	menu_layer_destroy(sMenuLayer);
}

static void windowAppear(struct Window *window)
{
	menu_layer_reload_data(sMenuLayer); // Without that, the number of items would not be right after a deletion
}

/* ---------- MAIN ---------- */

static void init(void)
{
	initLocales();

	if (persist_exists(STORAGE_VERSION_KEY)) {
		counterNumber = persist_read_int(COUNTER_NUMBER_KEY);
		for (int i = 0; i < counterNumber; i++) {
			struct Counter *counter = malloc(sizeof *counter);
			persist_read_data(i + FIRST_COUNTER_KEY, counter, sizeof *counter);
			counters[i] = counter;
		}
	} else {
		persist_write_int(STORAGE_VERSION_KEY, STORAGE_VERSION);
	}

	sWindow = window_create();
	window_set_window_handlers(sWindow, (WindowHandlers) {
		.load = windowLoad,
		.unload = windowUnload,
		.appear = windowAppear
	});

	initAddWindow();
	initDeleteWindow();
	initAboutWindow();
	initErrorWindow();

	window_stack_push(sWindow, true);
}

static void deinit(void)
{
	persist_write_int(COUNTER_NUMBER_KEY, counterNumber);
	for (int i = 0; i < counterNumber; i++) {
		persist_write_data(i + FIRST_COUNTER_KEY, counters[i], sizeof **counters);
		free(counters[i]);
	}

	deinitLocales();

	deinitAddWindow();
	deinitDeleteWindow();
	deinitAboutWindow();
	deinitErrorWindow();

	window_destroy(sWindow);
}

int main(void)
{
	init();
	app_event_loop();
	deinit();
}
