// Copyright (c) 2015 Marcus Fritzsch
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
// THE SOFTWARE.

#include <pebble.h>

static Window *window;
static GRect bounds;
static GPoint center;
static GBitmap *fontbitmap;
static GBitmap *numbers[10];

typedef struct {
   GColor tick;
   GColor background;
   GColor text;
} ThemeInfo;

static ThemeInfo theme;

static void update_effect_layer(Layer *l, GContext *ctx) {
   time_t t = time(NULL);
   struct tm *tm = localtime(&t);
   int32_t min = tm->tm_min;
   int32_t hr = tm->tm_hour;

   graphics_context_set_fill_color(ctx, theme.background);
   graphics_fill_rect(ctx, bounds, 0, 0);

   const int radius_out = 144 / 2 - 10;
   const int radius_in = 144 / 2 - 15;

   // Need to draw the ticks _long_ else they look rather fscking ugly.
   graphics_context_set_stroke_color(ctx, theme.tick);
   graphics_context_set_stroke_width(ctx, 1);
   for (int i = 1; i <= min; i++) {
      int32_t angle = TRIG_MAX_ANGLE * i / 60;
      int32_t sin = sin_lookup(angle);
      int32_t cos = cos_lookup(angle);

      GPoint p1 = {
          sin * radius_out / TRIG_MAX_RATIO + center.x,
         -cos * radius_out / TRIG_MAX_RATIO + center.y,
      };

      graphics_draw_line(ctx, center, p1);
   }

   graphics_fill_circle(ctx, center, radius_in);

   graphics_draw_bitmap_in_rect(ctx, numbers[hr / 10], GRect(144 / 2 - 24, 168 / 2 - 20, 24, 40));
   graphics_draw_bitmap_in_rect(ctx, numbers[hr % 10], GRect(144 / 2, 168 / 2 - 20, 24, 40));
}

static void window_load(Window *window) {
   Layer *window_layer = window_get_root_layer(window);
   bounds = layer_get_bounds(window_layer);
   center = grect_center_point(&bounds);
   layer_set_update_proc(window_layer, update_effect_layer);
}

static void window_unload(Window *window) {
}

static void handle_tick(struct tm *tick_time, TimeUnits units_changed) {
   layer_mark_dirty(window_get_root_layer(window));
}

static void init(void) {
   // Font bitmap created with GIMP
   // font: Nimbus Sans L Condense
   // size: 52
   // line height: -25.0
   // hinting: some
   // antialiasing: off

   /* // Indigo background theme */
   /* theme = (ThemeInfo) { */
   /*      .tick = GColorRajah, */
   /*      .background = GColorIndigo, */
   /*      .text = GColorWhite */
   /* }; */
   /* fontbitmap = gbitmap_create_with_resource(RESOURCE_ID_NUMBERS_INDIGO); */

   // black background theme
   theme = (ThemeInfo) {
           .tick = GColorLightGray,
           .background = GColorBlack,
           .text = GColorFolly
   };
   fontbitmap = gbitmap_create_with_resource(RESOURCE_ID_NUMBERS_BLACK);

   window = window_create();
   tick_timer_service_subscribe(MINUTE_UNIT, handle_tick);
   window_set_window_handlers(window,
                              (WindowHandlers){
                                 .load = window_load, .unload = window_unload,
                              });

   for (int i = 0; i < 10; i++) {
      numbers[i] = gbitmap_create_as_sub_bitmap(fontbitmap, GRect(0, i*40, 24, 40));
   }

   window_stack_push(window, true);
}

static void deinit(void) {
   for (int i = 0; i < 10; i++) {
      gbitmap_destroy(numbers[i]);
   }
   gbitmap_destroy(fontbitmap);
   tick_timer_service_unsubscribe();
   window_destroy(window);
}

int main(void) {
   init();
   app_event_loop();
   deinit();
}
