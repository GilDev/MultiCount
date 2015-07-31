#include "MultiCount.h"
#include <pebble.h>

#include "localization.h"
#include "addWindow.h"
#include "aboutWindow.h"
#include "errorWindow.h"

#define activateReordering() \
	states.reordering = true; \
	counterIdToReorder = counters[menu_layer_get_selected_index(sMenu).row]; \
	window_set_click_config_provider(sWindow, clickConfigProviderReordering); \
	menu_layer_reload_data(sMenu);

#define desactivateReordering() \
	states.reordering = false; \
	configureMenuClicks(); \
	menu_layer_reload_data(sMenu);


#define optionsActionBar() \
	action_bar_layer_set_icon(sActionBar, BUTTON_ID_UP, sReorderIcon); \
	action_bar_layer_set_icon(sActionBar, BUTTON_ID_SELECT, sCrossIcon); \
	action_bar_layer_set_icon(sActionBar, BUTTON_ID_DOWN, sTrashIcon); \
	action_bar_layer_set_click_config_provider(sActionBar, clickConfigProviderOptions); \
	sSelectedActionBar = 2;

#define incrementActionBar() \
	action_bar_layer_set_icon(sActionBar, BUTTON_ID_UP, sPlusIcon); \
	action_bar_layer_set_icon(sActionBar, BUTTON_ID_SELECT, sMoreIcon); \
	action_bar_layer_set_icon(sActionBar, BUTTON_ID_DOWN, sMinusIcon); \
	action_bar_layer_set_click_config_provider(sActionBar, clickConfigProviderIncrement); \
	sSelectedActionBar = 1;

#define showActionBar() \
	incrementActionBar(); \
	action_bar_layer_add_to_window(sActionBar, sWindow);

#define hideActionBar() \
	action_bar_layer_remove_from_window(sActionBar); \
	configureMenuClicks(); \
	sSelectedActionBar = 0;

static Window *sWindow;
static MenuLayer *sMenu;
static ActionBarLayer *sActionBar;
static uint8_t sSelectedActionBar = 0; // 0 = None, 1 = Increment, 2 = Options

static GBitmap *sAddIcon;
static GBitmap *sInfoIcon;
static GBitmap *sResetAllIcon;
static GBitmap *sPlusIcon;
static GBitmap *sMoreIcon;
static GBitmap *sMinusIcon;
static GBitmap *sReorderIcon;
static GBitmap *sCrossIcon;
static GBitmap *sTrashIcon;

static struct Counter *counterIdToReorder; // Easily store which counter you are currently reordering

struct {
	uint8_t moreClick: 1;
	uint8_t reordering: 1;
} states = {false, false};

/* ---------- COUNTERS ---------- */

void createCounter(const char *name, uint16_t value)
{
	struct Counter *newCounter = malloc(sizeof *newCounter);

	strcpy(newCounter->name, name);
	newCounter->value = value;

	counters[counterNumber++] = newCounter;
}

/* ---------- BACK BUTTON HANDLER ---------- */

static void configureMenuClicks();
static void clickConfigProviderIncrement(void *ctx);
static void clickConfigProviderOptions(void *ctx);

static void backClick(ClickRecognizerRef recognizer, void *ctx)
{
	if (states.reordering) {
		desactivateReordering();
	} else {
		switch (sSelectedActionBar) {
			case 0:
				window_stack_pop_all(true);
				break;
			case 1:
				hideActionBar();
				break;
			case 2:
				incrementActionBar();
		}
	}
}

/* ---------- MENU ---------- */

static uint16_t menuGetNumRows(struct MenuLayer *menuLayer, uint16_t section_index, void *callbackContext)
{
	if (states.reordering)
		return counterNumber;
	else
		return counterNumber + NUMBER_MENU_OPTIONS;
}

static void menuDrawRow(GContext *ctx, const Layer *cellLayer, MenuIndex *cellIndex, void *callbackContext)
{
	if (cellIndex->row == counterNumber) {
		menu_cell_basic_draw(ctx, cellLayer, lc(LC_ADD), NULL, sAddIcon);
	} else if (cellIndex->row == counterNumber + 1) {
		menu_cell_basic_draw(ctx, cellLayer, lc(LC_ABOUT), NULL, sInfoIcon);
	} else if (cellIndex->row == counterNumber + 2) {
		menu_cell_basic_draw(ctx, cellLayer, lc(LC_RESET_ALL), NULL, sResetAllIcon);
	} else {
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
	} else if (cellIndex->row == counterNumber + 1) {
		pushAboutWindow();
	} else if (cellIndex->row == counterNumber + 2) {
		for (int i = 0; i < counterNumber; i++)
			counters[i]->value = 0;
		menu_layer_set_selected_index(sMenu, (MenuIndex) {0, 0}, MenuRowAlignCenter, true);
	} else if (counters[cellIndex->row]->value < MAX_COUNTER_VALUE) {
		++counters[cellIndex->row]->value;
		menu_layer_reload_data(sMenu);
	}
}

static void menuSelectLongClick(struct MenuLayer *menuLayer, MenuIndex *cellIndex, void *callbackContext)
{
	if (sSelectedActionBar == 0 && cellIndex->row < counterNumber) {
		showActionBar();
		static const uint32_t duration = VIBRATION_DURATION;
		vibes_enqueue_custom_pattern((VibePattern) {
			.durations = &duration,
			.num_segments = 1
		});
	}
}

/* Without the following fuctions, a call to
 * menu_layer_set_click_config_onto_window(sMenu, sWindow)
 * after removing an action bar would not remap the Back button
 */

static ClickConfigProvider sBaseMenuClickConfigProvider;

static void menuClickConfigProvider(void *ctx)
{
	sBaseMenuClickConfigProvider(ctx);
	window_single_click_subscribe(BUTTON_ID_BACK, backClick);
}

static void configureMenuClicks(void)
{
	menu_layer_set_click_config_onto_window(sMenu, sWindow);
	sBaseMenuClickConfigProvider = window_get_click_config_provider(sWindow);
	window_set_click_config_provider_with_context(sWindow, menuClickConfigProvider, sMenu);
}

/* ---------- REORDERING ---------- */

static void reorderingUpClick(ClickRecognizerRef recognizer, void *ctx)
{
	uint16_t counterId = menu_layer_get_selected_index(sMenu).row;
	if (counterId > 0) {
		counters[counterId] = counters[counterId - 1];
		counters[counterId - 1] = counterIdToReorder;
		menu_layer_set_selected_next(sMenu, true, MenuRowAlignCenter, true);
	}
}

static void reorderingDownClick(ClickRecognizerRef recognizer, void *ctx)
{
	uint16_t counterId = menu_layer_get_selected_index(sMenu).row;
	if (counterId < counterNumber - 1) {
		counters[counterId] = counters[counterId + 1];
		counters[counterId + 1] = counterIdToReorder;
		menu_layer_set_selected_next(sMenu, false, MenuRowAlignCenter, true);
	}
}

static void clickConfigProviderReordering(void *ctx)
{
	window_single_click_subscribe(BUTTON_ID_UP, reorderingUpClick);
	window_single_click_subscribe(BUTTON_ID_SELECT, backClick); // When reordering, SELECT and BACK make the same thing
	window_single_click_subscribe(BUTTON_ID_DOWN, reorderingDownClick);
	window_single_click_subscribe(BUTTON_ID_BACK, backClick);
}

/* ---------- ACTION BARS ---------- */

/* Increment Action Bar */

static void incrementActionBarUpClick(ClickRecognizerRef recognizer, void *ctx)
{
	uint16_t rowId = menu_layer_get_selected_index(sMenu).row;
	if (counters[rowId]->value < MAX_COUNTER_VALUE) {
		++counters[rowId]->value;
		menu_layer_reload_data(sMenu);
	}
}

static void incrementActionBarSelectClick(ClickRecognizerRef recognizer, void *ctx)
{
	states.moreClick = true;
	optionsActionBar();
	sSelectedActionBar++;
}

static void incrementActionBarDownClick(ClickRecognizerRef recognizer, void *ctx)
{
	uint16_t rowId = menu_layer_get_selected_index(sMenu).row;
	if (counters[rowId]->value > 0) {
		--counters[rowId]->value;
		menu_layer_reload_data(sMenu);
	}
}

static void clickConfigProviderIncrement(void *ctx)
{
	window_single_repeating_click_subscribe(BUTTON_ID_UP, CLICKS_INTERVAL_INCREMENTING, incrementActionBarUpClick);
	window_single_click_subscribe(BUTTON_ID_SELECT, incrementActionBarSelectClick); // If non-raw, selection color stays when pressing BACK while sOptionsActionBar is shown
	window_single_repeating_click_subscribe(BUTTON_ID_DOWN, CLICKS_INTERVAL_INCREMENTING, incrementActionBarDownClick);
	window_single_click_subscribe(BUTTON_ID_BACK, backClick);
}

/* Options Action Bar */

static void optionsActionBarUpClick(ClickRecognizerRef recognizer, void *ctx)
{
	hideActionBar();
	activateReordering();
}

static void optionsActionBarSelectClick(ClickRecognizerRef recognizer, void *ctx)
{
	hideActionBar();
}

static void optionsActionBarDownClick(ClickRecognizerRef recognizer, void *ctx)
{
	MenuIndex counterIdToDelete = menu_layer_get_selected_index(sMenu);
	uint16_t row = counterIdToDelete.row;

	free(counters[row]);

	for (int i = row; i < counterNumber - 1; i++)
		counters[i] = counters[i + 1];
	counters[counterNumber--] = NULL;

	hideActionBar();

	counterIdToDelete.row = (row) ? row - 1 : 0; // Because after executing this function, sMenu automatically register a DOWN click. In fact after deleting the first counter, the second one will be selected (bug)
	menu_layer_set_selected_index(sMenu, counterIdToDelete, MenuRowAlignCenter, false);
	menu_layer_reload_data(sMenu);
}

static void clickConfigProviderOptions(void *ctx)
{
	window_raw_click_subscribe(BUTTON_ID_UP, optionsActionBarUpClick, NULL, NULL);
	window_single_click_subscribe(BUTTON_ID_SELECT, optionsActionBarSelectClick); // Should be raw to avoid stuck selection color but when it is, it justs makes the sIncrementActionBar pops up after pressing SELECT (weird bug)
	window_raw_click_subscribe(BUTTON_ID_DOWN, optionsActionBarDownClick, NULL, NULL);
	window_single_click_subscribe(BUTTON_ID_BACK, backClick);
}

/* ---------- WINDOW ---------- */

static void windowLoad(Window *window)
{
	Layer *windowLayer = window_get_root_layer(window);

	sAddIcon = gbitmap_create_with_resource(RESOURCE_ID_ADD_ICON);
	sInfoIcon = gbitmap_create_with_resource(RESOURCE_ID_INFO_ICON);
	sResetAllIcon = gbitmap_create_with_resource(RESOURCE_ID_RESET_ALL_ICON);

	sMenu = menu_layer_create(layer_get_bounds(windowLayer));
	configureMenuClicks();
	menu_layer_set_callbacks(sMenu, NULL, (MenuLayerCallbacks) {
		.get_num_rows = (MenuLayerGetNumberOfRowsInSectionsCallback) menuGetNumRows,
		.draw_row = (MenuLayerDrawRowCallback) menuDrawRow,
		.select_click = (MenuLayerSelectCallback) menuSelectClick,
		.select_long_click = (MenuLayerSelectCallback) menuSelectLongClick
	});
	layer_add_child(windowLayer, menu_layer_get_layer(sMenu));


	sPlusIcon = gbitmap_create_with_resource(RESOURCE_ID_PLUS_ICON);
	sMoreIcon = gbitmap_create_with_resource(RESOURCE_ID_MORE_ICON);
	sMinusIcon = gbitmap_create_with_resource(RESOURCE_ID_MINUS_ICON);
	sReorderIcon = gbitmap_create_with_resource(RESOURCE_ID_REORDER_ICON);
	sCrossIcon = gbitmap_create_with_resource(RESOURCE_ID_CROSS_ICON);
	sTrashIcon = gbitmap_create_with_resource(RESOURCE_ID_TRASH_ICON);

	sActionBar = action_bar_layer_create();
	action_bar_layer_set_icon(sActionBar, BUTTON_ID_UP, sPlusIcon);
	action_bar_layer_set_icon(sActionBar, BUTTON_ID_SELECT, sMoreIcon);
	action_bar_layer_set_icon(sActionBar, BUTTON_ID_DOWN, sMinusIcon);
	action_bar_layer_set_click_config_provider(sActionBar, clickConfigProviderIncrement);
}

static void windowUnload(Window *window)
{
	gbitmap_destroy(sAddIcon);
	gbitmap_destroy(sInfoIcon);
	gbitmap_destroy(sResetAllIcon);
	gbitmap_destroy(sPlusIcon);
	gbitmap_destroy(sMoreIcon);
	gbitmap_destroy(sMinusIcon);
	gbitmap_destroy(sReorderIcon);
	gbitmap_destroy(sCrossIcon);
	gbitmap_destroy(sTrashIcon);
	action_bar_layer_destroy(sActionBar);
	menu_layer_destroy(sMenu);
}

static void windowAppear(struct Window *window)
{
	menu_layer_reload_data(sMenu); // Without that, the number of items would not be right after a deletion
}

/* ---------- MAIN ---------- */

static void foo(struct tm *tick_time, TimeUnits units_changed)
{
	APP_LOG(APP_LOG_LEVEL_DEBUG, "%d", counterNumber);
}

static void init(void)
{
	tick_timer_service_subscribe(SECOND_UNIT, foo);
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
