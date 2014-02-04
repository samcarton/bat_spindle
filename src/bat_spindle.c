#include <pebble.h>

static Window *window;
static TextLayer *text_layer;

static TextLayer *hour_layer;
static TextLayer *minute_layer;

static AppTimer *timer;
const uint32_t timerTick = 50;

const unsigned int textXPadding = 5;
const unsigned int textYPadding = 5;

Layer *spinner_display_layer;

const GPathInfo SPINNER_PATH_POINTS =
{
  3,
  (GPoint [])
    {
      {0,0},
      {-150,10},
      {-10,150}
    }
};

static GPath *spinner_path;

static unsigned int spinAngle = 0;

const unsigned int spinAmount = 5;


static void handle_minute_tick(struct tm *tick_time, TimeUnits units_changed);


static void timer_callback(void *data)
{
  layer_mark_dirty(spinner_display_layer);

  spinAngle += spinAmount;

  if(spinAngle < 180)
  {
    timer = app_timer_register(timerTick, timer_callback, NULL);
  }
  else
  {
    spinAngle = 0;
  }

}

static void spinner_display_layer_callback(Layer *layer, GContext* ctx)
{
  gpath_rotate_to(spinner_path, (TRIG_MAX_ANGLE / 360) * (spinAngle + 90));

  graphics_context_set_fill_color(ctx, GColorBlack);

  gpath_draw_filled(ctx, spinner_path);

  gpath_rotate_to(spinner_path, (TRIG_MAX_ANGLE / 360) * (spinAngle + 270));

  gpath_draw_filled(ctx, spinner_path);

}

static void window_load(Window *window) {
  Layer *window_layer = window_get_root_layer(window);
  GRect bounds = layer_get_bounds(window_layer);

  //init spinner layer
  spinner_display_layer = layer_create(bounds);
  layer_set_update_proc(spinner_display_layer, spinner_display_layer_callback);
  layer_add_child(window_layer, spinner_display_layer);

  //init spinner path
  spinner_path = gpath_create(&SPINNER_PATH_POINTS);
  gpath_move_to(spinner_path, grect_center_point(&bounds));

  //init hour text layer
  hour_layer = text_layer_create((GRect) { .origin = { textXPadding, textYPadding }, .size = { 50, 50 } });
  text_layer_set_text_color(hour_layer, GColorWhite);
  text_layer_set_background_color(hour_layer, GColorClear);
  text_layer_set_font(hour_layer, fonts_get_system_font(FONT_KEY_BITHAM_42_BOLD));
  text_layer_set_text_alignment(hour_layer, GTextAlignmentCenter);
  text_layer_set_text(hour_layer, "alaskdjflsadkjf");
  layer_add_child(window_layer, text_layer_get_layer(hour_layer));

  //init minute text layer
  minute_layer = text_layer_create((GRect) { .origin = { 144 - 50 - textXPadding, 168 - 50 - textYPadding }, .size = { 50, 50 } });
  text_layer_set_text_color(minute_layer, GColorWhite);
  text_layer_set_background_color(minute_layer, GColorClear);
  text_layer_set_font(minute_layer, fonts_get_system_font(FONT_KEY_BITHAM_42_BOLD));
  text_layer_set_text_alignment(minute_layer, GTextAlignmentCenter);
  text_layer_set_text(minute_layer, "alaskdjflsadkjf");
  layer_add_child(window_layer, text_layer_get_layer(minute_layer));

  time_t now = time(NULL);
  struct tm *current_time = localtime(&now);
  handle_minute_tick(current_time, SECOND_UNIT);

}

static void window_unload(Window *window) {
  text_layer_destroy(text_layer);
}

static void handle_minute_tick(struct tm *tick_time, TimeUnits units_changed)
{
  // kick off spin animation
  //layer_mark_dirty(spinner_display_layer);
  timer = app_timer_register(timerTick, timer_callback, NULL);

  // SET MINUTE UPDATE TO OCCUR HALF WAY THRU TRANSITION SO NUMBER CHANGE IS HIDDEN
  //update clock display
  static char hour_text[] = "00";
  strftime(hour_text, sizeof(hour_text), "%H", tick_time);
  text_layer_set_text(hour_layer, hour_text);

  static char minute_text[] = "00";
  strftime(minute_text, sizeof(minute_text), "%M", tick_time);
  text_layer_set_text(minute_layer, minute_text);
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

  tick_timer_service_subscribe(MINUTE_UNIT, handle_minute_tick);

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
