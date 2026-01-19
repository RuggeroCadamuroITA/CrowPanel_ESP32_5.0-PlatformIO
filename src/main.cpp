#include <lvgl.h>
#include <SPI.h>
#include <Wire.h>
#include <Arduino.h>

// --- CONFIGURAZIONE LOVYAN GFX (Driver Display) ---
#include <LovyanGFX.hpp>
#include <lgfx/v1/platforms/esp32s3/Panel_RGB.hpp>
#include <lgfx/v1/platforms/esp32s3/Bus_RGB.hpp>

// Configurazione Pin Backlight
#define TFT_BL 2

class LGFX : public lgfx::LGFX_Device
{
public:
  lgfx::Bus_RGB     _bus_instance;
  lgfx::Panel_RGB   _panel_instance;

  LGFX(void)
  {
    {
      auto cfg = _bus_instance.config();
      cfg.panel = &_panel_instance;
      
      cfg.pin_d0  = GPIO_NUM_8; 
      cfg.pin_d1  = GPIO_NUM_3;  
      cfg.pin_d2  = GPIO_NUM_46;  
      cfg.pin_d3  = GPIO_NUM_9;  
      cfg.pin_d4  = GPIO_NUM_1;  
      
      cfg.pin_d5  = GPIO_NUM_5;  
      cfg.pin_d6  = GPIO_NUM_6; 
      cfg.pin_d7  = GPIO_NUM_7;  
      cfg.pin_d8  = GPIO_NUM_15;  
      cfg.pin_d9  = GPIO_NUM_16; 
      cfg.pin_d10 = GPIO_NUM_4;  
      
      cfg.pin_d11 = GPIO_NUM_45; 
      cfg.pin_d12 = GPIO_NUM_48; 
      cfg.pin_d13 = GPIO_NUM_47; 
      cfg.pin_d14 = GPIO_NUM_21; 
      cfg.pin_d15 = GPIO_NUM_14; 

      cfg.pin_henable = GPIO_NUM_40;
      cfg.pin_vsync   = GPIO_NUM_41;
      cfg.pin_hsync   = GPIO_NUM_39;
      cfg.pin_pclk    = GPIO_NUM_0;
      cfg.freq_write  = 16000000; 

      cfg.hsync_polarity    = 0;
      cfg.hsync_front_porch = 8;
      cfg.hsync_pulse_width = 4;
      cfg.hsync_back_porch  = 43;
      
      cfg.vsync_polarity    = 0;
      cfg.vsync_front_porch = 8;
      cfg.vsync_pulse_width = 4;
      cfg.vsync_back_porch  = 12;

      cfg.pclk_active_neg   = 1;
      cfg.de_idle_high      = 0;
      cfg.pclk_idle_high    = 0;

      _bus_instance.config(cfg);
    }
    {
      auto cfg = _panel_instance.config();
      cfg.memory_width  = 800;
      cfg.memory_height = 480;
      cfg.panel_width  = 800;
      cfg.panel_height = 480;
      cfg.offset_x = 0;
      cfg.offset_y = 0;
      _panel_instance.config(cfg);
    }
    _panel_instance.setBus(&_bus_instance);
    setPanel(&_panel_instance);
  }
};

LGFX lcd; 

// --- CONFIGURAZIONE TOUCH ---
#include "touch.h"

// --- VARIABILI LVGL ---
static lv_disp_draw_buf_t draw_buf;
static lv_color_t *disp_draw_buf;
static lv_disp_drv_t disp_drv;

lv_color_t *cbuf; 
lv_obj_t *canvas;

// Variabili per l'interpolazione
static lv_point_t last_point = {0, 0};
static bool is_drawing = false;

// --- FUNZIONI LVGL ---

void my_disp_flush(lv_disp_drv_t *disp, const lv_area_t *area, lv_color_t *color_p)
{
  uint32_t w = (area->x2 - area->x1 + 1);
  uint32_t h = (area->y2 - area->y1 + 1);
  lcd.pushImageDMA(area->x1, area->y1, w, h, (lgfx::rgb565_t*)&color_p->full);
  lv_disp_flush_ready(disp);
}

void my_touchpad_read(lv_indev_drv_t *indev_driver, lv_indev_data_t *data)
{
  if (touch_has_signal())
  {
    if (touch_touched())
    {
      data->state = LV_INDEV_STATE_PR;
      data->point.x = touch_last_x;
      data->point.y = touch_last_y;
    }
    else if (touch_released())
    {
      data->state = LV_INDEV_STATE_REL;
    }
  }
  else
  {
    data->state = LV_INDEV_STATE_REL;
  }
}

// --- LOGICA APP DISEGNO ---

static void canvas_draw_event_cb(lv_event_t * e)
{
    lv_event_code_t code = lv_event_get_code(e);
    lv_obj_t * cnv = lv_event_get_target(e);
    lv_indev_t * indev = lv_indev_get_act();

    if(indev == NULL) return;

    lv_point_t current_point;
    lv_indev_get_point(indev, &current_point);

    // Configurazione Penna (Linea)
    lv_draw_line_dsc_t line_dsc;
    lv_draw_line_dsc_init(&line_dsc);
    line_dsc.color = lv_color_white();
    line_dsc.width = 8;        // Spessore pennello
    line_dsc.round_end = 1;    // Estremità arrotondate
    line_dsc.round_start = 1;
    line_dsc.opa = LV_OPA_COVER;

    // Configurazione Penna (Punto singolo)
    lv_draw_rect_dsc_t rect_dsc;
    lv_draw_rect_dsc_init(&rect_dsc);
    rect_dsc.radius = LV_RADIUS_CIRCLE;
    rect_dsc.bg_color = lv_color_white();
    rect_dsc.bg_opa = LV_OPA_COVER;

    if(code == LV_EVENT_PRESSED) {
        last_point = current_point;
        is_drawing = true;
        
        // Disegna punto iniziale
        lv_canvas_draw_rect(cnv, current_point.x - 4, current_point.y - 4, 8, 8, &rect_dsc);
        lv_obj_invalidate(cnv);
    }
    else if(code == LV_EVENT_PRESSING && is_drawing) {
        
        // --- CORREZIONE QUI ---
        // Creiamo un array di punti che definisce la linea: [Inizio, Fine]
        lv_point_t line_points[2];
        line_points[0] = last_point;
        line_points[1] = current_point;

        // Disegna la linea usando l'array e specificando che sono 2 punti
        lv_canvas_draw_line(cnv, line_points, 2, &line_dsc);
        
        last_point = current_point;
        lv_obj_invalidate(cnv);
    }
    else if(code == LV_EVENT_RELEASED) {
        is_drawing = false;
    }
}

static void btn_clear_event_cb(lv_event_t * e)
{
    lv_canvas_fill_bg(canvas, lv_color_black(), LV_OPA_COVER);
    lv_obj_invalidate(canvas); 
}

void setup_paint_app()
{
    cbuf = (lv_color_t *)heap_caps_malloc(LV_CANVAS_BUF_SIZE_TRUE_COLOR(800, 480), MALLOC_CAP_SPIRAM);
    
    if (cbuf == NULL) {
        Serial.println("ERRORE PSRAM!");
        return;
    }

    canvas = lv_canvas_create(lv_scr_act());
    lv_obj_add_flag(canvas, LV_OBJ_FLAG_CLICKABLE); 
    lv_canvas_set_buffer(canvas, cbuf, 800, 480, LV_IMG_CF_TRUE_COLOR);
    lv_obj_center(canvas);
    
    lv_canvas_fill_bg(canvas, lv_color_black(), LV_OPA_COVER);

    lv_obj_add_event_cb(canvas, canvas_draw_event_cb, LV_EVENT_ALL, NULL);

    lv_obj_t * btn = lv_btn_create(lv_scr_act());
    lv_obj_align(btn, LV_ALIGN_BOTTOM_MID, 0, -20);
    lv_obj_add_event_cb(btn, btn_clear_event_cb, LV_EVENT_CLICKED, NULL);
    lv_obj_set_style_bg_color(btn, lv_palette_main(LV_PALETTE_BLUE), 0);

    lv_obj_t * label = lv_label_create(btn);
    lv_label_set_text(label, "PULISCI");
    lv_obj_center(label);

    lv_obj_t * title = lv_label_create(lv_scr_act());
    lv_label_set_text(title, "Test Disegno Continuo (Linee)");
    lv_obj_set_style_text_color(title, lv_color_white(), 0); 
    lv_obj_align(title, LV_ALIGN_TOP_MID, 0, 10);
}

void setup()
{
  Serial.begin(115200);
  
  lcd.begin();
  lcd.fillScreen(TFT_BLACK);
  lcd.setBrightness(128);
  pinMode(TFT_BL, OUTPUT);
  digitalWrite(TFT_BL, HIGH);

  delay(5);

  lv_init();
  touch_init(); 

  uint32_t screenWidth = 800;
  uint32_t screenHeight = 480;
  uint32_t buf_size = screenWidth * screenHeight / 10; 

  disp_draw_buf = (lv_color_t *)heap_caps_malloc(buf_size * sizeof(lv_color_t), MALLOC_CAP_INTERNAL | MALLOC_CAP_8BIT);
  if(!disp_draw_buf) {
     disp_draw_buf = (lv_color_t *)heap_caps_malloc(buf_size * sizeof(lv_color_t), MALLOC_CAP_SPIRAM);
  }

  lv_disp_draw_buf_init(&draw_buf, disp_draw_buf, NULL, buf_size);

  lv_disp_drv_init(&disp_drv);
  disp_drv.hor_res = screenWidth;
  disp_drv.ver_res = screenHeight;
  disp_drv.flush_cb = my_disp_flush;
  disp_drv.draw_buf = &draw_buf;
  lv_disp_drv_register(&disp_drv);

  static lv_indev_drv_t indev_drv;
  lv_indev_drv_init(&indev_drv);
  indev_drv.type = LV_INDEV_TYPE_POINTER;
  indev_drv.read_cb = my_touchpad_read;
  lv_indev_drv_register(&indev_drv);

  setup_paint_app();

  Serial.println("Setup completato.");
}

void loop()
{
  lv_timer_handler();
  delay(5);
}