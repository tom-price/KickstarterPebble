#include <pebble.h>
#include <math.h>

#define KEY_PLEDGED 0
#define KEY_BACKERS 1

static Window *s_main_window;
static TextLayer *s_time_layer;
static TextLayer *s_pledged_layer;
static TextLayer *s_backers_layer;

static GFont s_time_font;
static GFont s_pledged_font;
static GFont s_data_font;

static void update_time() {
	// Get a tm structure
	time_t temp = time(NULL); 
	struct tm *tick_time = localtime(&temp);

	// Create a long-lived buffer
	static char buffer[] = "00:00";

	// Write the current hours and minutes into the buffer
	if(clock_is_24h_style() == true) {
		//Use 2h hour format
		strftime(buffer, sizeof("00:00"), "%H:%M", tick_time);
	} else {
		//Use 12 hour format
		strftime(buffer, sizeof("00:00"), "%I:%M", tick_time);
	}

	// Display this time on the TextLayer
	text_layer_set_text(s_time_layer, buffer);
}

static void main_window_load(Window *window) {
	// Create time TextLayer
	s_time_layer = text_layer_create(GRect(5, 5, 139, 50));
	text_layer_set_background_color(s_time_layer, GColorClear);
	text_layer_set_text_color(s_time_layer, GColorWhite);
	text_layer_set_text_alignment(s_time_layer, GTextAlignmentCenter);
	text_layer_set_text(s_time_layer, "00:00");

	// Create Kickstarter Layer
	s_pledged_layer = text_layer_create(GRect(0, 65, 144, 42));
	text_layer_set_background_color(s_pledged_layer, GColorClear);
	text_layer_set_text_color(s_pledged_layer, GColorWhite);
	text_layer_set_text_alignment(s_pledged_layer, GTextAlignmentCenter);
	text_layer_set_text(s_pledged_layer, "Loading...");
	
	// Create Backers Layer
	s_backers_layer = text_layer_create(GRect(0, 110, 144, 50));
	text_layer_set_background_color(s_backers_layer, GColorClear);
	text_layer_set_text_color(s_backers_layer, GColorWhite);
	text_layer_set_text_alignment(s_backers_layer, GTextAlignmentCenter);
	text_layer_set_text(s_backers_layer, "Loading...\nBackers");

	//Create GFont
	s_time_font = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_RNS_SUBSET_48));
	s_pledged_font = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_RNS_30));
	s_data_font = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_RNS_20));
	
	//Apply to TextLayer
	text_layer_set_font(s_time_layer, s_time_font);
	text_layer_set_font(s_pledged_layer, s_pledged_font);
	text_layer_set_font(s_backers_layer, s_data_font);

	// Add it as a child layer to the Window's root layer
	layer_add_child(window_get_root_layer(window), text_layer_get_layer(s_time_layer));
	layer_add_child(window_get_root_layer(window), text_layer_get_layer(s_pledged_layer));
	layer_add_child(window_get_root_layer(window), text_layer_get_layer(s_backers_layer));
  
	// Make sure the time is displayed from the start
	update_time();
}

static void main_window_unload(Window *window) {
	//Unload GFont
	fonts_unload_custom_font(s_time_font);
	fonts_unload_custom_font(s_pledged_font);
	fonts_unload_custom_font(s_data_font);

	// Destroy TextLayer
	text_layer_destroy(s_time_layer);
	text_layer_destroy(s_pledged_layer);
	text_layer_destroy(s_backers_layer);
}

static void tick_handler(struct tm *tick_time, TimeUnits units_changed) {
  update_time();
  
  // Get Kickstarter Info update every 5 minutes
  if(tick_time->tm_min % 5 == 0) {
    // Begin dictionary
    DictionaryIterator *iter;
    app_message_outbox_begin(&iter);

    // Add a key-value pair
    dict_write_uint8(iter, 0, 0);

    // Send the message!
    app_message_outbox_send();
  }
}

static void inbox_received_callback(DictionaryIterator *iterator, void *context) {
  // Store incoming information
	static char pledged_buffer[16];
	static char backers_buffer[16];
	static char backers_layer_buffer[32];
  
  // Read first item
  Tuple *t = dict_read_first(iterator);

  // For all items
	while(t != NULL) {
		// Which key was received?
		switch(t->key) {
		case KEY_PLEDGED:
			if (t->value->int32 > 1000000) {
				snprintf(pledged_buffer, sizeof(pledged_buffer), "$%d.%d M", (int)(t->value->int32 / 1000000),(int)(t->value->int32 % 1000000 / 100000));
			} else if (t->value->int32 > 100000) {
				snprintf(pledged_buffer, sizeof(pledged_buffer), "$%d K", (int)(t->value->int32 / 1000));
			} else { snprintf(pledged_buffer, sizeof(pledged_buffer), "$%d", (int)t->value->int32); }
			break;
		case KEY_BACKERS:
			snprintf(backers_buffer, sizeof(backers_buffer), "%d,%d", (int)(t->value->int32/1000),(int)(t->value->int32%1000));
			break;
		default:
			APP_LOG(APP_LOG_LEVEL_ERROR, "Key %d not recognized!", (int)t->key);
			break;
		}

		// Look for next item
		t = dict_read_next(iterator);
	}
  
  // Assemble full string and display
	text_layer_set_text(s_pledged_layer, pledged_buffer);
	snprintf(backers_layer_buffer, sizeof(backers_layer_buffer), "%s\nBackers", backers_buffer);
	text_layer_set_text(s_backers_layer, backers_layer_buffer);
}

static void inbox_dropped_callback(AppMessageResult reason, void *context) {
  APP_LOG(APP_LOG_LEVEL_ERROR, "Message dropped!");
}

static void outbox_failed_callback(DictionaryIterator *iterator, AppMessageResult reason, void *context) {
  APP_LOG(APP_LOG_LEVEL_ERROR, "Outbox send failed!");
}

static void outbox_sent_callback(DictionaryIterator *iterator, void *context) {
  APP_LOG(APP_LOG_LEVEL_INFO, "Outbox send success!");
}

static void init() {
	// Create main Window element and assign to pointer
	s_main_window = window_create();
	window_set_background_color(s_main_window, GColorBlack);

	// Set handlers to manage the elements inside the Window
	window_set_window_handlers(s_main_window, (WindowHandlers) {
		.load = main_window_load,
		.unload = main_window_unload
	});

	// Show the Window on the watch, with animated=true
	window_stack_push(s_main_window, true);
	
	// Register with TickTimerService
	tick_timer_service_subscribe(MINUTE_UNIT, tick_handler);
	
	// Register callbacks
	app_message_register_inbox_received(inbox_received_callback);
	app_message_register_inbox_dropped(inbox_dropped_callback);
	app_message_register_outbox_failed(outbox_failed_callback);
	app_message_register_outbox_sent(outbox_sent_callback);

	// Open AppMessage
	app_message_open(app_message_inbox_size_maximum(), app_message_outbox_size_maximum());
}

static void deinit() {
	// Destroy Window
	window_destroy(s_main_window);
}

int main(void) {
	init();
	app_event_loop();
	deinit();
}
