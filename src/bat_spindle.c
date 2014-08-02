#include <pebble.h>
#include <string.h>

static Window *window;


static TextLayer *hour_layer;
static TextLayer *minute_layer;

static AppTimer *timer;
const uint32_t timerTick = 50; //ms

const unsigned char textXPadding = 5;
const unsigned char textYPadding = 15;
const unsigned char textXSize = 60; 
const unsigned char textYSize = 50;

const unsigned char timeTransitionAngle = 90;
short int timeTransitionedThisTick = 0;

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

static unsigned char spinAngle = 0;

const unsigned char spinAmount = 5;

static char hour_text_next[] = "00";
static char hour_text[] = "00";
static char minute_text_next[] = "00";
static char minute_text[] = "00";


// Change from angle to time-based spin
static uint32_t currentSpinProgress = 0;
const uint32_t lengthOfSpin = 2000; //ms

static void handle_minute_tick(struct tm *tick_time, TimeUnits units_changed);
static void updateTextLayers();
static float easingFunction(float p);
static float backEaseInOut(float p);
static float backEaseOut(float p);
static float bounceEaseOut(float p);

static void timer_callback(void *data)
{
  layer_mark_dirty(spinner_display_layer);
  
  currentSpinProgress += timerTick;
  float eased = easingFunction(currentSpinProgress/(lengthOfSpin*1.0f));
  spinAngle = 180*eased;
  
  if(currentSpinProgress < lengthOfSpin)
  {
    timer = app_timer_register(timerTick, timer_callback, NULL);
  }
  else
  {
    spinAngle = 0;
    currentSpinProgress = 0;
  }

  if(timeTransitionedThisTick == 0)
  {
    if(eased > 0.5f) 
    {
      updateTextLayers();
      timeTransitionedThisTick = 1;
    }
  }

}

static float easingFunction(float p)
{  
  return bounceEaseOut(p);
}

// Easing functions adapted from https://github.com/warrenm/AHEasing/ (WTFPL license)
static float backEaseOut(float p)
{
  float f = (1 - p);
  return 1 - (f * f - f * sin_lookup(f * TRIG_MAX_ANGLE)/(TRIG_MAX_RATIO*1.0f));
}

static float backEaseInOut(float p)
{  
  if(p < 0.5)
  {
    float f = 2 * p;
    return 0.5 * (f * f * f - f * sin_lookup(f * TRIG_MAX_ANGLE)/(TRIG_MAX_RATIO*1.0f));
  }
  else
  {
    float f = (1 - (2*p - 1));
    return 0.5 * (1 - (f * f * f - f * sin_lookup(f * TRIG_MAX_ANGLE)/(TRIG_MAX_RATIO*1.0f))) + 0.5;
  }
}

static float bounceEaseOut(float p)
{
  if(p < 4/11.0)
  {
    return (121 * p * p)/16.0;
  }
  else if(p < 8/11.0)
  {
    return (363/40.0 * p * p) - (99/10.0 * p) + 17/5.0;
  }
  else if(p < 9/10.0)
  {
    return (4356/361.0 * p * p) - (35442/1805.0 * p) + 16061/1805.0;
  }
  else
  {
    return (54/5.0 * p * p) - (513/25.0 * p) + 268/25.0;
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
  hour_layer = text_layer_create((GRect) { .origin = { textXPadding, textYPadding }, .size = { textXSize, textYSize } });
  text_layer_set_text_color(hour_layer, GColorWhite);
  text_layer_set_background_color(hour_layer, GColorClear);
  text_layer_set_font(hour_layer, fonts_get_system_font(FONT_KEY_BITHAM_42_BOLD));
  text_layer_set_text_alignment(hour_layer, GTextAlignmentCenter);
  text_layer_set_text(hour_layer, "");
  layer_add_child(window_layer, text_layer_get_layer(hour_layer));

  //init minute text layer
  minute_layer = text_layer_create((GRect) { .origin = { 144 - textXSize - textXPadding, 168 - textYSize - textYPadding }, .size = { textXSize, textYSize } });
  text_layer_set_text_color(minute_layer, GColorWhite);
  text_layer_set_background_color(minute_layer, GColorClear);
  text_layer_set_font(minute_layer, fonts_get_system_font(FONT_KEY_BITHAM_42_BOLD));
  text_layer_set_text_alignment(minute_layer, GTextAlignmentCenter);
  text_layer_set_text(minute_layer, "");
  layer_add_child(window_layer, text_layer_get_layer(minute_layer));

}

static void window_unload(Window *window) {
  text_layer_destroy(hour_layer);
  text_layer_destroy(minute_layer);
  gpath_destroy(spinner_path);
}

static void handle_minute_tick(struct tm *tick_time, TimeUnits units_changed)
{
  // kick off spin animation
  //layer_mark_dirty(spinner_display_layer);
  timer = app_timer_register(timerTick, timer_callback, NULL);

  // SET MINUTE UPDATE TO OCCUR HALF WAY THRU TRANSITION SO NUMBER CHANGE IS HIDDEN
  timeTransitionedThisTick = 0;

  //Save Time strings for updating later  
  strftime(hour_text_next, sizeof(hour_text_next), "%H", tick_time);
  strftime(minute_text_next, sizeof(minute_text_next), "%M", tick_time);
  
}

static void updateTextLayers()
{
  strcpy(hour_text,hour_text_next);
  strcpy(minute_text,minute_text_next);
  text_layer_set_text(hour_layer, hour_text);
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
  // UPDATE THIS
  window_destroy(window);
}

int main(void) {
  init();

  APP_LOG(APP_LOG_LEVEL_DEBUG, "Done initializing, pushed window: %p", window);

  app_event_loop();
  deinit();
}
