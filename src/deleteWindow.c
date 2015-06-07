#include "deleteWindow.h"
#include <pebble.h>

#include "MultiCount.h"
#include "localization.h"
#include "errorWindow.h"

static Window *sWindow;
static MenuLayer *sMenuLayer;

/* ---------- MENU ---------- */

static uint16_t menuGetNumRows(struct MenuLayer *menuLayer, uint16_t section_index, void *callbackContext)
{
	return counterNumber;
}

static void menuDrawRow(GContext *ctx, const Layer *cellLayer, MenuIndex *cellIndex, void *callbackContext)
{
	char valueStr[MAX_NAME_SIZE + 1];
	snprintf(valueStr, MAX_NAME_SIZE + 1, "%d", counters[cellIndex->row]->value);
	menu_cell_basic_draw(ctx, cellLayer, counters[cellIndex->row]->name, valueStr, NULL);
}

static void menuSelectClick(struct MenuLayer *menuLayer, MenuIndex *cellIndex, void *callbackContext)
{
	free(counters[cellIndex->row]);

	for (int i = cellIndex->row; i < counterNumber - 1; i++)
		counters[i] = counters[i + 1];
	counters[counterNumber--] = NULL;

	window_stack_pop(true);
}

/* ---------- WINDOW ---------- */

static void windowLoad(struct Window *window)
{
	Layer *windowLayer = window_get_root_layer(window);

	sMenuLayer = menu_layer_create(layer_get_bounds(windowLayer));

	menu_layer_set_click_config_onto_window(sMenuLayer, window);

	menu_layer_set_callbacks(sMenuLayer, NULL, (MenuLayerCallbacks) {
		.get_num_rows = (MenuLayerGetNumberOfRowsInSectionsCallback) menuGetNumRows,
		.draw_row = (MenuLayerDrawRowCallback) menuDrawRow,
		.select_click = (MenuLayerSelectCallback) menuSelectClick
	});

	layer_add_child(windowLayer, menu_layer_get_layer(sMenuLayer));
}

static void windowUnload(struct Window *window)
{
	menu_layer_destroy(sMenuLayer);
}

void pushDeleteWindow(void)
{
	if (counterNumber <= 0) {
		pushErrorWindow(lc(LC_DELETE_ERROR_NO_COUNTER));
	} else {
		window_stack_push(sWindow, true);
	}
}

void initDeleteWindow(void)
{
	sWindow = window_create();
	window_set_window_handlers(sWindow, (WindowHandlers) {
		.load = windowLoad,
		.unload = windowUnload
	});
}

void deinitDeleteWindow(void)
{
	window_destroy(sWindow);
}
