# CrowPanel ESP32-S3 HMI (5.0" / 7.0") - PlatformIO & LVGL Demo

This repository contains a robust starter template and a **Drawing/Paint Demo** for the **Elecrow CrowPanel ESP32-S3 HMI** display (RGB interface version).

It utilizes **PlatformIO**, the **Arduino Framework**, and **LVGL 8.3** to demonstrate high-performance graphics and accurate touch handling using the GT911 controller.

![License](https://img.shields.io/badge/license-MIT-green)
![Platform](https://img.shields.io/badge/platform-Espressif32-blue)
![Framework](https://img.shields.io/badge/framework-Arduino-orange)

## 🎯 Features

*   **Display Driver:** High-speed RGB interface using [LovyanGFX](https://github.com/lovyan03/LovyanGFX).
*   **Touch Driver:** Capacitive touch support via **GT911** (I2C) with coordinate calibration.
*   **Graphics Library:** Integrated **LVGL 8.3.6**.
*   **Memory Management:** Full PSRAM utilization for the frame buffer and canvas (Essential for 800x480 resolution).
*   **Demo Application:** A "Paint" app featuring:
    *   Linear interpolation for smooth drawing (no gaps when moving fast).
    *   Canvas stored in PSRAM.
    *   "Clear Screen" functionality.

## 🛠 Hardware Support

*   **Board:** Elecrow CrowPanel ESP32-S3 HMI (5.0" or 7.0")
*   **MCU:** ESP32-S3 (Dual Core, 240MHz)
*   **Resolution:** 800 x 480 px
*   **Memory:** 8MB/16MB Flash + 8MB PSRAM (OPI)

## 📂 Project Structure

```text
├── boards/
│   └── esp32-s3-devkitc-1-myboard.json  # Custom Board definition for PlatformIO
├── include/
│   └── touch.h                          # GT911 Touch Controller logic and pinout
├── src/
│   └── main.cpp                         # Main application (LVGL Setup + Paint Logic)
├── platformio.ini                       # Project configuration and library dependencies
└── README.md                            # Documentation
