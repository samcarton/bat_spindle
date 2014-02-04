#include <pebble.h>

static Window *window;
static TextLayer *text_layer;

Layer *spinner_display_layer;

const GPathInfo SPINNER_PATH_POINTS =
{
  3,
  (GPoint [])
    {
      {0,0},
      {-150,30},
      {-30,150}
    }
};

static GPath *spinner_path;

static unsigned int spinAngle = 0;

static void spinner_display_layer_callback(Layer *layer, GContext* ctx)
{
  gpath_rotate_to(spinner_path, (TRIG_MAX_ANGLE / 360) * spinAngle);

  graphics_context_set_fill_color(ctx, GColorBlack);

  gpath_draw_filled(ctx, spinner_path);

  gpath_rotate_to(spinner_path, (TRIG_MAX_ANGLE / 360) * (spinAngle + 180));

  gpath_draw_filled(ctx, spinner_path);

  spinAngle += 10;

}

static void window_load(Window *window) {
  Layer *window_layer = window_get_root_layer(window);
  GRect bounds = layer_get_bounds(window_layer);

  text_layer = text_layer_create((GRect) { .origin = { 0, 72 }, .size = { bounds.size.w, 20 } });
  text_layer_set_text(text_layer, "Hello Narah!");
  text_layer_set_text_alignment(text_layer, GTextAlignmentCenter);
  layer_add_child(window_layer, text_layer_get_layer(text_layer));

  //init spinner layer
  spinner_display_layer = layer_create(bounds);
  layer_set_update_proc(spinner_display_layer, spinner_display_layer_callback);
  layer_add_child(window_layer, spinner_display_layer);

  //init spinner path
  spinner_path = gpath_create(&SPINNER_PATH_POINTS);
  gpath_move_to(spinner_path, grect_center_point(&bounds));
}

static void window_unload(Window *window) {
  text_layer_destroy(text_layer);
}

static void handle_minute_tick(struct tm *tick_time, TimeUnits units_changed)
{
  layer_mark_dirty(spinner_display_layer);
}

static void init(void) {
  window = window_create();
  window_set_background_color(window, GColorWhite);
  window_set_window_handlers(window, (WindowHandlers) {
    .load = window_load,
    .unload = window_unload,
  });
  const bool animated = true;
  window_stack_push(window, animated);

  tick_timer_service_subscribe(SECOND_UNIT, handle_minute_tick);

}

static void deinit(void) {
  window_destroy(window);
}

int main(void) {
  init();

  APP_LOG(APP_LOG_LEVEL_DEBUG, "Done initializing, pushed window: %p", window);

  app_event_loop();
  deinit();
}
