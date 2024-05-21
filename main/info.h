#pragma once

/* ---> Wi-Fi Settings <--- */

// Show Wi-Fi connection status every 3 seconds
// Options: true, false
// Default: true
#define SHOW_CONNECTION_STATUS true

// Clear old connections when uploading new code (Clears ROM)
// Options: true, false
// Default: true
#define CLEAR_PAIRED_PEER_ON_NEW_UPLOAD true

// Attempt to establish a connection when signal strength is greater than this value
// Unit: decibel-milliwatts - dBm
// Range: 0 to -100
// Default: -20
#define MIN_RSSI_TO_INITIATE_CONNECTION -20

/* ---> Built-in RGB LED Settings <--- */
// https://www.selecolor.com/en/hsv-color-picker/

// Built-in RGB LED - Hue
// Unit: angle/phase - °
// Range: 0 to 360
// Default: 350
#define RGB_LED_HUE 350

// Built-in RGB LED - Saturation
// Unit: percentage - %
// Range: 0 to 100
// Default: 75
#define RGB_LED_SATURATION 75

// Built-in RGB LED - Brightness / Value
// Unit: percentage - %
// Range: 0 to 100
// Default: 10
#define RGB_LED_VALUE 10
