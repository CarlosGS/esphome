#ifdef USE_ARDUINO

#include "gps.h"
#include "esphome/core/log.h"
#include "esphome/core/time.h"

namespace esphome {
namespace gps {

static const char *const TAG = "gps";

TinyGPSPlus &GPSListener::get_tiny_gps() { return this->parent_->get_tiny_gps(); }

void GPS::update() {
  if (this->latitude_sensor_ != nullptr)
    this->latitude_sensor_->publish_state(this->latitude_);

  if (this->longitude_sensor_ != nullptr)
    this->longitude_sensor_->publish_state(this->longitude_);

  if (this->speed_sensor_ != nullptr)
    this->speed_sensor_->publish_state(this->speed_);

  if (this->course_sensor_ != nullptr)
    this->course_sensor_->publish_state(this->course_);

  if (this->altitude_sensor_ != nullptr)
    this->altitude_sensor_->publish_state(this->altitude_);

  if (this->satellites_sensor_ != nullptr)
    this->satellites_sensor_->publish_state(this->satellites_);
}

void GPS::loop() {
  while (this->available() && !this->has_time_) {
    if (this->tiny_gps_.encode(this->read())) {
      if (tiny_gps_.time.isValid() && tiny_gps_.date.isValid() && tiny_gps_.time.isUpdated() && tiny_gps_.date.year() > 2023) {
        ESPTime val{};
        val.year = tiny_gps_.date.year();
        val.month = tiny_gps_.date.month();
        val.day_of_month = tiny_gps_.date.day();
        // Set these to valid value for  recalc_timestamp_utc - it's not used for calculation
        val.day_of_week = 1;
        val.day_of_year = 1;
      
        val.hour = tiny_gps_.time.hour();
        val.minute = tiny_gps_.time.minute();
        val.second = tiny_gps_.time.second();
        val.recalc_timestamp_utc(false);
        uint32_t epoch = val.timestamp;
        struct timeval timev {
          .tv_sec = static_cast<time_t>(epoch), .tv_usec = 0,
        };
        ESP_LOGD(TAG, "GPS epoch %" PRIu32, epoch);
        /*struct timezone tz = {0, 0};
        int ret = settimeofday(&timev, &tz);
        if (ret == EINVAL) {
          // Some ESP8266 frameworks abort when timezone parameter is not NULL
          // while ESP32 expects it not to be NULL
          ret = settimeofday(&timev, nullptr);
        }
        if (ret != 0) {
          ESP_LOGW(TAG, "setimeofday() failed with code %d", ret);
        }c*/
      }
      if (tiny_gps_.location.isUpdated()) {
        this->latitude_ = tiny_gps_.location.lat();
        this->longitude_ = tiny_gps_.location.lng();

        ESP_LOGD(TAG, "Location:");
        ESP_LOGD(TAG, "  Lat: %f", this->latitude_);
        ESP_LOGD(TAG, "  Lon: %f", this->longitude_);
      }

      if (tiny_gps_.speed.isUpdated()) {
        this->speed_ = tiny_gps_.speed.kmph();
        ESP_LOGD(TAG, "Speed:");
        ESP_LOGD(TAG, "  %f km/h", this->speed_);
      }
      if (tiny_gps_.course.isUpdated()) {
        this->course_ = tiny_gps_.course.deg();
        ESP_LOGD(TAG, "Course:");
        ESP_LOGD(TAG, "  %f °", this->course_);
      }
      if (tiny_gps_.altitude.isUpdated()) {
        this->altitude_ = tiny_gps_.altitude.meters();
        ESP_LOGD(TAG, "Altitude:");
        ESP_LOGD(TAG, "  %f m", this->altitude_);
      }
      if (tiny_gps_.satellites.isUpdated()) {
        this->satellites_ = tiny_gps_.satellites.value();
        ESP_LOGD(TAG, "Satellites:");
        ESP_LOGD(TAG, "  %d", this->satellites_);
      }

      for (auto *listener : this->listeners_)
        listener->on_update(this->tiny_gps_);
    }
  }
}

}  // namespace gps
}  // namespace esphome

#endif  // USE_ARDUINO
