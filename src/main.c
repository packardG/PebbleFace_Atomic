#include <pebble.h>

static Window *s_main_window;
static TextLayer *s_time_layer, *s_date_layer;
static GFont s_time_font, s_date_font;
static BitmapLayer *s_background_layer;
static GBitmap *s_background_bitmap;

static void update_time() {
  //Get tm struct
  time_t temp = time(NULL);
  struct tm *tick_time = localtime(&temp);
  
  //Write current hours/min into buffer
  static char s_buffer[8];
  strftime(s_buffer, sizeof(s_buffer), clock_is_24h_style() ?
                                          "%H:%M" : "%I:%M", tick_time);
  // Write date into buffer
  static char s_date_buffer[16];
  strftime(s_date_buffer, sizeof(s_date_buffer), "%a,%b %d", tick_time);
  
  //Display this time & date on text layer
  text_layer_set_text(s_time_layer, s_buffer);
  text_layer_set_text(s_date_layer, s_date_buffer);
}

static void tick_handler(struct tm *tick_time, TimeUnits units_changed){
  update_time();
}

static void main_window_load(Window *window){
  // Get information about the Window
  Layer *window_layer = window_get_root_layer(window);
  GRect bounds = layer_get_bounds(window_layer);

  // create GBitmap
  s_background_bitmap = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_ATOM);
  
  // Create BitmapLayer to display the GBitmap
  s_background_layer = bitmap_layer_create(bounds);
  
  // Set the bitmap onto the layer and add to the window
  bitmap_layer_set_bitmap(s_background_layer, s_background_bitmap);
  //bitmap_layer_set_compositing_mode(s_background_layer, GCompOpSet);
  layer_add_child(window_layer, bitmap_layer_get_layer(s_background_layer));
  
  // Create the TextLayer with specific bounds
  s_time_layer = text_layer_create(
      GRect(0, PBL_IF_ROUND_ELSE(58, 52), bounds.size.w, 50));
  s_date_layer = text_layer_create(GRect(0, 120, 144, 30));
  
  // Create GFont
  s_time_font = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_AKASHI_35));
  s_date_font = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_AKASHI_19));

  // Improve the layout to be more like a watchface
  // Time
  text_layer_set_font(s_time_layer, s_time_font);
  text_layer_set_background_color(s_time_layer, GColorClear);
  text_layer_set_text_color(s_time_layer, GColorBlack);
  text_layer_set_text_alignment(s_time_layer, GTextAlignmentCenter);
  // Date
  text_layer_set_font(s_date_layer, s_date_font);
  text_layer_set_background_color(s_date_layer, GColorClear);
  text_layer_set_text_color(s_date_layer, GColorBlack);
  text_layer_set_text_alignment(s_date_layer, GTextAlignmentCenter);

  // Add it as a child layer to the Window's root layer
  layer_add_child(window_layer, text_layer_get_layer(s_time_layer));
  layer_add_child(window_layer, text_layer_get_layer(s_date_layer));
  
  // Trying to vertically align text layer
//   GRect frame = layer_get_frame(text_layer_get_layer(s_time_layer));
//   GSize content_size = text_layer_get_content_size(s_time_layer);
//   layer_set_frame(text_layer_get_layer(s_time_layer), GRect(frame.origin.x, frame.origin.y + (frame.size.h - content_size.h) / 2, frame.size.w, content_size.h));
}

static void main_window_unload(Window *window){
  //Destroy text layer
  text_layer_destroy(s_time_layer);
  text_layer_destroy(s_date_layer);
  //Unload GFont
  fonts_unload_custom_font(s_time_font);
  fonts_unload_custom_font(s_date_font);
  //Destroy GBitmap
  gbitmap_destroy(s_background_bitmap);
  //Destroy BitmapLayer
  bitmap_layer_destroy(s_background_layer);
}

static void init() {
  // Create main Window element and assign to pointer
  s_main_window = window_create();

  window_set_background_color(s_main_window, COLOR_FALLBACK(GColorWhite, GColorBlack));
  
  // Set handlers to manage the elements inside the Window
  window_set_window_handlers(s_main_window, (WindowHandlers) {
    .load = main_window_load,
    .unload = main_window_unload
  });

  // Show the Window on the watch, with animated=true
  window_stack_push(s_main_window, true);
  
  //Make sure time is displayed from start
  update_time();
  
  //Register with TickTimerService
  tick_timer_service_subscribe(MINUTE_UNIT, tick_handler);
}

static void deinit() {
  // Destroy window layer
  window_destroy(s_main_window);
}

int main(void) {
  init();
  app_event_loop();
  deinit();
}