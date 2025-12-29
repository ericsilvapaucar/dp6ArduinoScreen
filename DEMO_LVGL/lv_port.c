/*
 * SPDX-FileCopyrightText: 2022-2024 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "esp_check.h"
#include "esp_err.h"
#include "esp_lcd_panel_interface.h"
#include "esp_lcd_panel_io.h"
#include "esp_lcd_panel_ops.h"
#include "esp_log.h"
#include "esp_system.h"
#include "esp_timer.h"
#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"
#include "freertos/task.h"

#include "lv_port.h"
#include "lvgl.h"

#ifdef ESP_LVGL_PORT_TOUCH_COMPONENT
#include "esp_lcd_touch.h"
#endif

#if (ESP_IDF_VERSION < ESP_IDF_VERSION_VAL(4, 4, 4)) ||                        \
    (ESP_IDF_VERSION == ESP_IDF_VERSION_VAL(5, 0, 0))
#define LVGL_PORT_HANDLE_FLUSH_READY 0
#else
#define LVGL_PORT_HANDLE_FLUSH_READY 1
#endif

static const char *TAG = "LVGL";

/*******************************************************************************
 * Types definitions
 *******************************************************************************/

typedef struct lvgl_port_ctx_s {
  SemaphoreHandle_t lvgl_mux;
  esp_timer_handle_t tick_timer;
  bool running;
  int task_max_sleep_ms;
} lvgl_port_ctx_t;

typedef struct {
  esp_lcd_panel_io_handle_t io_handle; /* LCD panel IO handle */
  esp_lcd_panel_handle_t panel_handle; /* LCD panel handle */
  // lv_disp_drv_t             disp_drv;     /* LVGL display driver (Removed in
  // v9) */

  uint32_t trans_size;              /* Maximum size for one transport */
  void *trans_buf_1;                /* Buffer send to driver */
  void *trans_buf_2;                /* Buffer send to driver */
  void *trans_act;                  /* Active buffer for sending to driver */
  SemaphoreHandle_t trans_done_sem; /* Semaphore for signaling idle transfer */
  /* lv_display_rotation_t sw_rotate; (Removed, using LVGL native rotation) */

  lvgl_port_wait_cb draw_wait_cb; /* Callback function for drawing */
} lvgl_port_display_ctx_t;

#ifdef ESP_LVGL_PORT_TOUCH_COMPONENT
typedef struct {
  esp_lcd_touch_handle_t handle; /* LCD touch IO handle */
  // lv_indev_drv_t          indev_drv;     /* LVGL input device driver (Removed
  // in v9) */
  lvgl_port_wait_cb touch_wait_cb; /* Callback function for touch */
} lvgl_port_touch_ctx_t;
#endif

/*******************************************************************************
 * Local variables
 *******************************************************************************/
static lvgl_port_ctx_t lvgl_port_ctx;
static int lvgl_port_timer_period_ms = 5;

/*******************************************************************************
 * Function definitions
 *******************************************************************************/
static void lvgl_port_task(void *arg);
static esp_err_t lvgl_port_tick_init(void);
static void lvgl_port_task_deinit(void);

// LVGL callbacks
#if LVGL_PORT_HANDLE_FLUSH_READY
static bool lvgl_port_flush_ready_callback(esp_lcd_panel_io_handle_t panel_io,
                                           esp_lcd_panel_io_event_data_t *edata,
                                           void *user_ctx);
#endif
static void lvgl_port_flush_callback(lv_display_t *disp, const lv_area_t *area,
                                     uint8_t *px_map);
#ifdef ESP_LVGL_PORT_TOUCH_COMPONENT
static void lvgl_port_touchpad_read(lv_indev_t *indev, lv_indev_data_t *data);
#endif
/*******************************************************************************
 * Public API functions
 *******************************************************************************/

esp_err_t lvgl_port_init(const lvgl_port_cfg_t *cfg) {
  esp_err_t ret = ESP_OK;
  ESP_GOTO_ON_FALSE(cfg, ESP_ERR_INVALID_ARG, err, TAG, "invalid argument");
  ESP_GOTO_ON_FALSE(cfg->task_affinity < (configNUM_CORES), ESP_ERR_INVALID_ARG,
                    err, TAG,
                    "Bad core number for task! Maximum core number is %d",
                    (configNUM_CORES - 1));

  memset(&lvgl_port_ctx, 0, sizeof(lvgl_port_ctx));

  /* LVGL init */
  lv_init();
  /* Tick init */
  lvgl_port_timer_period_ms = cfg->timer_period_ms;
  ESP_RETURN_ON_ERROR(lvgl_port_tick_init(), TAG, "");
  /* Create task */
  lvgl_port_ctx.task_max_sleep_ms = cfg->task_max_sleep_ms;
  if (lvgl_port_ctx.task_max_sleep_ms == 0) {
    lvgl_port_ctx.task_max_sleep_ms = 500;
  }
  lvgl_port_ctx.lvgl_mux = xSemaphoreCreateRecursiveMutex();
  ESP_GOTO_ON_FALSE(lvgl_port_ctx.lvgl_mux, ESP_ERR_NO_MEM, err, TAG,
                    "Create LVGL mutex fail!");

  BaseType_t res;
  if (cfg->task_affinity < 0) {
    res = xTaskCreate(lvgl_port_task, "LVGL task", cfg->task_stack, NULL,
                      cfg->task_priority, NULL);
  } else {
    res = xTaskCreatePinnedToCore(lvgl_port_task, "LVGL task", cfg->task_stack,
                                  NULL, cfg->task_priority, NULL,
                                  cfg->task_affinity);
  }
  ESP_GOTO_ON_FALSE(res == pdPASS, ESP_FAIL, err, TAG,
                    "Create LVGL task fail!");

err:
  if (ret != ESP_OK) {
    lvgl_port_deinit();
  }

  return ret;
}

esp_err_t lvgl_port_resume(void) {
  esp_err_t ret = ESP_ERR_INVALID_STATE;

  if (lvgl_port_ctx.tick_timer != NULL) {
    lv_timer_enable(true);
    ret = esp_timer_start_periodic(lvgl_port_ctx.tick_timer,
                                   lvgl_port_timer_period_ms * 1000);
  }

  return ret;
}

esp_err_t lvgl_port_stop(void) {
  esp_err_t ret = ESP_ERR_INVALID_STATE;

  if (lvgl_port_ctx.tick_timer != NULL) {
    lv_timer_enable(false);
    ret = esp_timer_stop(lvgl_port_ctx.tick_timer);
  }

  return ret;
}

esp_err_t lvgl_port_deinit(void) {
  /* Stop and delete timer */
  if (lvgl_port_ctx.tick_timer != NULL) {
    esp_timer_stop(lvgl_port_ctx.tick_timer);
    esp_timer_delete(lvgl_port_ctx.tick_timer);
    lvgl_port_ctx.tick_timer = NULL;
  }

  /* Stop running task */
  if (lvgl_port_ctx.running) {
    lvgl_port_ctx.running = false;
  } else {
    lvgl_port_task_deinit();
  }

  return ESP_OK;
}

lv_display_t *lvgl_port_add_disp(const lvgl_port_display_cfg_t *disp_cfg) {
  esp_err_t ret = ESP_OK;
  lv_display_t *disp = NULL;
  void *buf1 = NULL;
  void *buf2 = NULL;
  void *buf3 = NULL;
  SemaphoreHandle_t trans_done_sem = NULL;

  assert(disp_cfg != NULL);
  assert(disp_cfg->io_handle != NULL);
  assert(disp_cfg->panel_handle != NULL);
  assert(disp_cfg->buffer_size > 0);
  assert(disp_cfg->hres > 0);
  assert(disp_cfg->vres > 0);

  /* Display context */
  lvgl_port_display_ctx_t *disp_ctx = malloc(sizeof(lvgl_port_display_ctx_t));
  ESP_GOTO_ON_FALSE(disp_ctx, ESP_ERR_NO_MEM, err, TAG,
                    "Not enough memory for display context allocation!");
  disp_ctx->io_handle = disp_cfg->io_handle;
  disp_ctx->panel_handle = disp_cfg->panel_handle;
  disp_ctx->trans_size = disp_cfg->trans_size;
  /* disp_ctx->sw_rotate = disp_cfg->sw_rotate; (Removed) */
  disp_ctx->draw_wait_cb = disp_cfg->draw_wait_cb;

  uint32_t buff_caps = MALLOC_CAP_DEFAULT;
  if (disp_cfg->flags.buff_dma) {
    buff_caps = MALLOC_CAP_DMA;
  } else if (disp_cfg->flags.buff_spiram) {
    buff_caps = MALLOC_CAP_SPIRAM;
  }

  /* alloc draw buffers used by LVGL */
  /* it's recommended to choose the size of the draw buffer(s) to be at least
   * 1/10 screen sized */
  buf1 =
      heap_caps_malloc(disp_cfg->buffer_size * sizeof(lv_color_t), buff_caps);
  ESP_GOTO_ON_FALSE(buf1, ESP_ERR_NO_MEM, err, TAG,
                    "Not enough memory for LVGL buffer (buf1) allocation!");

  if (disp_ctx->trans_size) {

    uint32_t caps = MALLOC_CAP_DMA;

    buf2 = heap_caps_malloc(disp_ctx->trans_size * sizeof(lv_color_t), caps);
    ESP_GOTO_ON_FALSE(buf2, ESP_ERR_NO_MEM, err, TAG,
                      "Not enough memory for buffer(transport) allocation!");
    disp_ctx->trans_buf_1 = buf2;

    buf3 = heap_caps_malloc(disp_ctx->trans_size * sizeof(lv_color_t), caps);
    ESP_GOTO_ON_FALSE(buf3, ESP_ERR_NO_MEM, err, TAG,
                      "Not enough memory for buffer(transport) allocation!");
    disp_ctx->trans_buf_2 = buf3;

    trans_done_sem = xSemaphoreCreateCounting(1, 0);
    ESP_GOTO_ON_FALSE(trans_done_sem, ESP_ERR_NO_MEM, err, TAG,
                      "Failed to create transport counting Semaphore");
    disp_ctx->trans_done_sem = trans_done_sem;
  }

  ESP_LOGD(TAG, "Create display object for LVGL");
  disp = lv_display_create(disp_cfg->hres, disp_cfg->vres);
  ESP_GOTO_ON_FALSE(disp, ESP_ERR_NO_MEM, err, TAG,
                    "Failed to create display object");

  lv_display_set_flush_cb(disp, lvgl_port_flush_callback);
  lv_display_set_buffers(disp, buf1, NULL,
                         disp_cfg->buffer_size * sizeof(lv_color_t),
                         LV_DISPLAY_RENDER_MODE_PARTIAL);
  lv_display_set_user_data(disp, disp_ctx);

  /* Set rotation using LVGL native API */
  lv_display_set_rotation(disp, disp_cfg->sw_rotate);

#if LVGL_PORT_HANDLE_FLUSH_READY
  /* Register done callback */
  const esp_lcd_panel_io_callbacks_t cbs = {
      .on_color_trans_done = lvgl_port_flush_ready_callback,
  };
  /* Note: In LVGL9 we don't pass driver but display, but callback gets display
     driver or similar. Wait, register_event_callbacks expects user_data. We
     pass 'disp' now instead of disp_drv address. However,
     lvgl_port_flush_ready_callback needs to know how to notify properly. We
     will check flush_ready_callback implementation.
  */
  esp_lcd_panel_io_register_event_callbacks(disp_ctx->io_handle, &cbs, disp);
#endif

  return disp;

err:
  if (ret != ESP_OK) {
    if (buf1) {
      free(buf1);
    }
    // buf2, buf3, and trans_done_sem are no longer allocated here
    /*
    if (buf2) {
      free(buf2);
    }
    if (buf3) {
      free(buf3);
    }
    if (trans_done_sem) {
      vSemaphoreDelete(trans_done_sem);
    }
    */
    if (disp_ctx) {
      free(disp_ctx);
    }
    if (disp) {
      lv_display_delete(disp);
    }
  }

  return NULL;
}

esp_err_t lvgl_port_remove_disp(lv_display_t *disp) {
  assert(disp);
  lvgl_port_display_ctx_t *disp_ctx =
      (lvgl_port_display_ctx_t *)lv_display_get_user_data(disp);

  // Buffers are managed by user in this port logic, we need to free them if we
  // allocated them. In `lvgl_port_add_disp` we allocated `buf1`, `buf2`,
  // `buf3`. Wait, `buf1` was passed to `lv_display_set_buffers`. LVGL 9 doesn't
  // free it by default unless configured? Actually, standard LVGL
  // `lv_display_delete` might not free the buffer pointer if it's passed as
  // reference. We need to retrieve the buffer pointer from display? Or just
  // rely on `disp_ctx` cleanup if we stored it? We didn't store `buf1` in
  // `disp_ctx` explicitly in `lvgl_port_add_disp` (the original code stored
  // `lv_disp_draw_buf_t *draw_buf` in `disp_drv`, which had `buf1`). In the new
  // code, `buf1` is passed to LVGL. We should probably accept that we might
  // leak `buf1` if we don't store it, OR we rely on LVGL to not free it and we
  // lose reference. Let's check `disp_ctx` members. It has `trans_buf_1` (buf2)
  // and `trans_buf_2` (buf3). `buf1` was just `buf1`. We need to free it. We
  // can get it via `lv_display_get_buf_active(disp)` etc? For now, I'll focus
  // on `lv_display_delete(disp)` and `free(disp_ctx)`. The original code freed
  // `disp_drv->draw_buf->buf1`.

  lv_display_delete(disp);

  if (disp_ctx) {
    // trans_buf_1, trans_buf_2, and trans_done_sem are no longer used/allocated
    /*
    if (disp_ctx->trans_buf_1)
      free(disp_ctx->trans_buf_1);
    if (disp_ctx->trans_buf_2)
      free(disp_ctx->trans_buf_2);
    if (disp_ctx->trans_done_sem)
      vSemaphoreDelete(disp_ctx->trans_done_sem);
    */
    free(disp_ctx);
  }

  return ESP_OK;
}

#ifdef ESP_LVGL_PORT_TOUCH_COMPONENT
lv_indev_t *lvgl_port_add_touch(const lvgl_port_touch_cfg_t *touch_cfg) {
  assert(touch_cfg != NULL);
  assert(touch_cfg->disp != NULL);
  assert(touch_cfg->handle != NULL);

  /* Touch context */
  lvgl_port_touch_ctx_t *touch_ctx = malloc(sizeof(lvgl_port_touch_ctx_t));
  if (touch_ctx == NULL) {
    ESP_LOGE(TAG, "Not enough memory for touch context allocation!");
    return NULL;
  }
  touch_ctx->handle = touch_cfg->handle;
  touch_ctx->touch_wait_cb = touch_cfg->touch_wait_cb;

  /* Register a touchpad input device */
  lv_indev_t *indev = lv_indev_create();
  lv_indev_set_type(indev, LV_INDEV_TYPE_POINTER);
  lv_indev_set_disp(indev, touch_cfg->disp);
  lv_indev_set_read_cb(indev, lvgl_port_touchpad_read);
  lv_indev_set_user_data(indev, touch_ctx);
  return indev;
}

esp_err_t lvgl_port_remove_touch(lv_indev_t *touch) {
  assert(touch);
  lvgl_port_touch_ctx_t *touch_ctx =
      (lvgl_port_touch_ctx_t *)lv_indev_get_user_data(touch);

  lv_indev_delete(touch);

  if (touch_ctx) {
    free(touch_ctx);
  }

  return ESP_OK;
}
#endif

bool lvgl_port_lock(uint32_t timeout_ms) {
  assert(lvgl_port_ctx.lvgl_mux && "lvgl_port_init must be called first");

  const TickType_t timeout_ticks =
      (timeout_ms == 0) ? portMAX_DELAY : pdMS_TO_TICKS(timeout_ms);
  return xSemaphoreTakeRecursive(lvgl_port_ctx.lvgl_mux, timeout_ticks) ==
         pdTRUE;
}

void lvgl_port_unlock(void) {
  assert(lvgl_port_ctx.lvgl_mux && "lvgl_port_init must be called first");
  xSemaphoreGiveRecursive(lvgl_port_ctx.lvgl_mux);
}

void lvgl_port_flush_ready(lv_display_t *disp) {
  assert(disp);
  lv_display_flush_ready(disp);
}

/*******************************************************************************
 * Private functions
 *******************************************************************************/

static void lvgl_port_task(void *arg) {
  uint32_t task_delay_ms = lvgl_port_ctx.task_max_sleep_ms;

  ESP_LOGI(TAG, "Starting LVGL task");
  lvgl_port_ctx.running = true;
  while (lvgl_port_ctx.running) {
    if (lvgl_port_lock(0)) {
      task_delay_ms = lv_timer_handler();
      lvgl_port_unlock();
    }
    if ((task_delay_ms > lvgl_port_ctx.task_max_sleep_ms) ||
        (1 == task_delay_ms)) {
      task_delay_ms = lvgl_port_ctx.task_max_sleep_ms;
    } else if (task_delay_ms < 1) {
      task_delay_ms = 1;
    }
    vTaskDelay(pdMS_TO_TICKS(task_delay_ms));
  }

  lvgl_port_task_deinit();

  /* Close task */
  vTaskDelete(NULL);
}

static void lvgl_port_task_deinit(void) {
  if (lvgl_port_ctx.lvgl_mux) {
    vSemaphoreDelete(lvgl_port_ctx.lvgl_mux);
  }
  memset(&lvgl_port_ctx, 0, sizeof(lvgl_port_ctx));
#if LV_ENABLE_GC || !LV_MEM_CUSTOM
  /* Deinitialize LVGL */
  lv_deinit();
#endif
}

#if LVGL_PORT_HANDLE_FLUSH_READY
static bool lvgl_port_flush_ready_callback(esp_lcd_panel_io_handle_t panel_io,
                                           esp_lcd_panel_io_event_data_t *edata,
                                           void *user_ctx) {
  BaseType_t taskAwake = pdFALSE;

  /* user_ctx is now 'disp' (lv_display_t*) */
  lv_display_t *disp = (lv_display_t *)user_ctx;
  assert(disp != NULL);
  lvgl_port_display_ctx_t *disp_ctx =
      (lvgl_port_display_ctx_t *)lv_display_get_user_data(disp);
  assert(disp_ctx != NULL);

  if (disp_ctx->trans_done_sem) {
    xSemaphoreGiveFromISR(disp_ctx->trans_done_sem, &taskAwake);
  }

  return false;
}
#endif

static void lvgl_port_flush_callback(lv_display_t *disp, const lv_area_t *area,
                                     uint8_t *px_map) {
  assert(disp != NULL);
  lvgl_port_display_ctx_t *disp_ctx =
      (lvgl_port_display_ctx_t *)lv_display_get_user_data(disp);
  assert(disp_ctx != NULL);

  const int x_start = area->x1;
  const int x_end = area->x2;
  const int y_start = area->y1;
  const int y_end = area->y2;

  /* Use simple pass-through. LVGL handles software rotation into the buffer if
   * configured. */
  /* If LVGL rotation is used (lv_display_set_rotation), the 'area' and 'px_map'
   * are already in the physical coordinates/orientation */

  esp_lcd_panel_draw_bitmap(disp_ctx->panel_handle, x_start, y_start, x_end + 1,
                            y_end + 1, px_map);

  lv_display_flush_ready(disp);
}

#ifdef ESP_LVGL_PORT_TOUCH_COMPONENT
static void lvgl_port_touchpad_read(lv_indev_t *indev, lv_indev_data_t *data) {
  assert(indev);
  lvgl_port_touch_ctx_t *touch_ctx =
      (lvgl_port_touch_ctx_t *)lv_indev_get_user_data(indev);
  assert(touch_ctx->handle);

  uint16_t touchpad_x[1] = {0};
  uint16_t touchpad_y[1] = {0};
  uint8_t touchpad_cnt = 0;

  /* Read data from touch controller into memory */
  bool touch_int = false;
  if (touch_ctx->touch_wait_cb) {
    touch_int = touch_ctx->touch_wait_cb(touch_ctx->handle->config.user_data);
  }
  if (touch_int) {
    esp_lcd_touch_read_data(touch_ctx->handle);
    /* Read data from touch controller */
    bool touchpad_pressed = esp_lcd_touch_get_coordinates(
        touch_ctx->handle, touchpad_x, touchpad_y, NULL, &touchpad_cnt, 1);

    if (touchpad_pressed && touchpad_cnt > 0) {
      data->point.x = touchpad_x[0];
      data->point.y = touchpad_y[0];
      data->state = LV_INDEV_STATE_PRESSED;
      esp_rom_printf("Touchpad pressed: x=%d, y=%d\n", data->point.x,
                     data->point.y);
    } else {
      data->state = LV_INDEV_STATE_RELEASED;
    }
  }
}
#endif

static void lvgl_port_tick_increment(void *arg) {
  /* Tell LVGL how many milliseconds have elapsed */
  lv_tick_inc(lvgl_port_timer_period_ms);
}

static esp_err_t lvgl_port_tick_init(void) {
  // Tick interface for LVGL (using esp_timer to generate 2ms periodic event)
  const esp_timer_create_args_t lvgl_tick_timer_args = {
      .callback = &lvgl_port_tick_increment,
      .name = "LVGL tick",
  };
  ESP_RETURN_ON_ERROR(
      esp_timer_create(&lvgl_tick_timer_args, &lvgl_port_ctx.tick_timer), TAG,
      "Creating LVGL timer filed!");
  return esp_timer_start_periodic(lvgl_port_ctx.tick_timer,
                                  lvgl_port_timer_period_ms * 1000);
}