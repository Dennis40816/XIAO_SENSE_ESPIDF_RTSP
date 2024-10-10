#ifdef __cplusplus
extern "C"
{
#endif
/** Include Region(System) **/
/* freertos */
#include "freertos/FreeRTOS.h"
/* esp related */
#include "esp_camera.h"
#include "esp_log.h"
#include "esp_rtsp.h"
/* driver */
#include "driver/gpio.h"
#include "driver/usb_serial_jtag.h"
/* camera related */
#include "img_converters.h"

/** Include Region(User) **/
#include "psram_checker.h"

/** XIAO Camera Defines **/
/* freq */
#define XIAO_XCLK_FREQ (20000000)

/* xiao pinout */
#define XIAO_GPIO_DUMMY      (-1)
#define XIAO_GPIO_XCLK       (GPIO_NUM_10)
#define XIAO_GPIO_CAM_SCL    (GPIO_NUM_39)
#define XIAO_GPIO_CAM_SDA    (GPIO_NUM_40)
#define XIAO_GPIO_SIOD       (XIAO_GPIO_CAM_SDA)
#define XIAO_GPIO_SIOC       (XIAO_GPIO_CAM_SCL)
#define XIAO_GPIO_DVP_Y9     (GPIO_NUM_48)
#define XIAO_GPIO_DVP_Y8     (GPIO_NUM_11)
#define XIAO_GPIO_DVP_Y7     (GPIO_NUM_12)
#define XIAO_GPIO_DVP_Y6     (GPIO_NUM_14)
#define XIAO_GPIO_DVP_Y5     (GPIO_NUM_16)
#define XIAO_GPIO_DVP_Y4     (GPIO_NUM_18)
#define XIAO_GPIO_DVP_Y3     (GPIO_NUM_17)
#define XIAO_GPIO_DVP_Y2     (GPIO_NUM_15)
#define XIAO_GPIO_DVP_VSYSNC (GPIO_NUM_38)
#define XIAO_GPIO_DVP_HREF   (GPIO_NUM_47)
#define XIAO_GPIO_DVP_PCLK   (GPIO_NUM_13)

/* camera init related */
#define XIAO_PWDN_GPIO_NUM     (XIAO_GPIO_DUMMY)
#define XIAO_RESET_GPIO_NUM    (XIAO_GPIO_DUMMY)
#define XIAO_XCLK_GPIO_NUM     (XIAO_GPIO_XCLK)
#define XIAO_SCCB_SDA_GPIO_NUM (XIAO_GPIO_SIOD)
#define XIAO_SCCB_SCL_GPIO_NUM (XIAO_GPIO_SIOC)
#define XIAO_D7_GPIO_NUM       (XIAO_GPIO_DVP_Y9)
#define XIAO_D6_GPIO_NUM       (XIAO_GPIO_DVP_Y8)
#define XIAO_D5_GPIO_NUM       (XIAO_GPIO_DVP_Y7)
#define XIAO_D4_GPIO_NUM       (XIAO_GPIO_DVP_Y6)
#define XIAO_D3_GPIO_NUM       (XIAO_GPIO_DVP_Y5)
#define XIAO_D2_GPIO_NUM       (XIAO_GPIO_DVP_Y4)
#define XIAO_D1_GPIO_NUM       (XIAO_GPIO_DVP_Y3)
#define XIAO_D0_GPIO_NUM       (XIAO_GPIO_DVP_Y2)
#define XIAO_VSYNC_GPIO_NUM    (XIAO_GPIO_DVP_VSYSNC)
#define XIAO_HREF_GPIO_NUM     (XIAO_GPIO_DVP_HREF)
#define XIAO_PCLK_GPIO_NUM     (XIAO_GPIO_DVP_PCLK)

/* XIAO Resolution */
#define XIAO_RESOLUTION_QVGA (FRAMESIZE_QVGA)  // 320x240
#define XIAO_RESOLUTION_CIF  (FRAMESIZE_CIF)  // 352x288
#define XIAO_RESOLUTION_VGA  (FRAMESIZE_VGA)  // 640x480
#define XIAO_RESOLUTION_SVGA (FRAMESIZE_SVGA)  // 800x600
#define XIAO_RESOLUTION_XGA  (FRAMESIZE_XGA)  // 1024x768
#define XIAO_RESOLUTION_SXGA (FRAMESIZE_SXGA)  // 1280x1024
#define XIAO_RESOLUTION_UXGA (FRAMESIZE_UXGA)  // 1600x1200

/* XIAO Pixel Format */
#define XIAO_PIXEL_FORMAT_RGB565    (PIXFORMAT_RGB565)  // 16-bit RGB
#define XIAO_PIXEL_FORMAT_YUV422    (PIXFORMAT_YUV422)  // YUV 4:2:2
#define XIAO_PIXEL_FORMAT_GRAYSCALE (PIXFORMAT_GRAYSCALE)  // Grayscale
#define XIAO_PIXEL_FORMAT_JPEG      (PIXFORMAT_JPEG)  // JPEG compressed
#define XIAO_PIXEL_FORMAT_RGB888    (PIXFORMAT_RGB888)  // 24-bit RGB
#define XIAO_PIXEL_FORMAT_RAW       (PIXFORMAT_RAW)  // RAW
#define XIAO_PIXEL_FORMAT_RGB444    (PIXFORMAT_RGB444)  // 12-bit RGB
#define XIAO_PIXEL_FORMAT_RGB555    (PIXFORMAT_RGB555)  // 15-bit RGB

/* XIAO Grab Mode */
#define XIAO_GRAB_MODE_WHEN_EMPTY (CAMERA_GRAB_WHEN_EMPTY)
#define XIAO_GRAB_MODE_LATEST     (CAMERA_GRAB_LATEST)

/* XIAO Frame Buffer RAM Mode */
#define XIAO_FB_USE_PSRAM (CAMERA_FB_IN_PSRAM)
#define XIAO_FB_USE_DRAM  (CAMERA_FB_IN_DRAM)

/** User Define Region **/
/* FreeRTOS related */
#define XIAO_START_UP_DELAY (1000)

/* XIAO related */
#define XIAO_RESOLUTION   (XIAO_RESOLUTION_UXGA)
#define XIAO_PIXEL_FORMAT (XIAO_PIXEL_FORMAT_JPEG)
#define XIAO_GRAB_MODE    (XIAO_GRAB_MODE_WHEN_EMPTY)
#define XIAO_QUALITY      (10)  // from 0 to 63
#define XIAO_FB_RAM_MODE  (XIAO_FB_USE_PSRAM)
#define XIAO_FB_COUNT     (3)

/* Xiao Wi-Fi related */
#define XIAO_WIFI_SSID     ("a52s")
#define XIAO_WIFI_PASSWORD ("11111111")

  /**  User Static Variable Region **/

  /**
   * @brief According to ref.1, there's not possible to shutdown camera through
   * GPIO
   *
   * @ref 1.
   * https://forum.seeedstudio.com/t/xiao-esp32s3-sense-camera-sleep-current/271258/21
   *
   */
  static camera_config_t xiao_esp32s3_sense_config = {
      .pin_pwdn = XIAO_PWDN_GPIO_NUM,  // NOT POSSIBLE -> NOT IMPORTANT
      .pin_reset = XIAO_RESET_GPIO_NUM,
      .pin_xclk = XIAO_XCLK_GPIO_NUM,
      .pin_sscb_sda = XIAO_SCCB_SDA_GPIO_NUM,
      .pin_sscb_scl = XIAO_SCCB_SCL_GPIO_NUM,
      .pin_d7 = XIAO_D7_GPIO_NUM,  // MAP to Y9
      .pin_d6 = XIAO_D6_GPIO_NUM,
      .pin_d5 = XIAO_D5_GPIO_NUM,
      .pin_d4 = XIAO_D4_GPIO_NUM,
      .pin_d3 = XIAO_D3_GPIO_NUM,
      .pin_d2 = XIAO_D2_GPIO_NUM,
      .pin_d1 = XIAO_D1_GPIO_NUM,
      .pin_d0 = XIAO_D0_GPIO_NUM,
      .pin_vsync = XIAO_VSYNC_GPIO_NUM,
      .pin_href = XIAO_HREF_GPIO_NUM,
      .pin_pclk = XIAO_PCLK_GPIO_NUM,
      .xclk_freq_hz = XIAO_XCLK_FREQ,
      /* TODO: could be deleted? */
      .ledc_timer = LEDC_TIMER_0,
      .ledc_channel = LEDC_CHANNEL_0,

      .fb_location = XIAO_FB_RAM_MODE,
      .fb_count = XIAO_FB_COUNT,

      .pixel_format = XIAO_PIXEL_FORMAT,
      .frame_size = XIAO_RESOLUTION,
      .jpeg_quality = XIAO_QUALITY,
      .grab_mode = XIAO_GRAB_MODE,
  };

  static bool xiao_camera_inited_flag = false;

  /** Static Function **/

  /**
   * @brief
   *
   * @warning This function should only be called once
   *
   */
  static void xiao_camera_init()
  {
    if (xiao_camera_inited_flag)
    {
      ESP_LOGW("CAMERA_INIT", "Camera Already Initialized");
      return;
    }

    esp_err_t err = esp_camera_init(&xiao_esp32s3_sense_config);
    if (err != ESP_OK)
    {
      ESP_LOGE("CAMERA_INIT", "Camera Init Failed With Error Code: {0x%x}",
               err);
      return;
    }
    ESP_LOGI("CAMERA_INIT", "Camera Init Successful");
    xiao_camera_inited_flag = true;
  }

  static void xiao_capature_new_frame()
  {
    camera_fb_t *fb = esp_camera_fb_get();
    if (!fb)
    {
      ESP_LOGE("CAPTURE_FRAME", "Failed to Get New Frame Buffer!");
      return;
    }

    // return buffer
    esp_camera_fb_return(fb);
  }

  static void xiao_wifi_init() {}

  void app_main()
  {
    // wait for a second if detected usb cable connected
    if (usb_serial_jtag_is_connected())
    {
      vTaskDelay(pdMS_TO_TICKS(XIAO_START_UP_DELAY));
    }

    xiao_camera_init();
  }

#ifdef __cplusplus
}
#endif
