#pragma once

/* ---> Wi-Fi設定 <--- */

// 上傳新代碼時清除舊連線 (Clear ROM)
// 可選: true, false
#define CLEAR_PAIRED_PEER_ON_NEW_UPLOAD true

// 每隔3秒列出Wi-Fi連線狀態
// 可選: true, false
#define SHOW_CONNECTION_STATUS true

// 訊號強度大於該值時將嘗試建立連線
// 單位: 分貝毫瓦 - dBm
// 範圍: 0 to -100
#define MIN_RSSI_TO_INITIATE_CONNECTION -20

/* ---> 內置彩色LED設定 <--- */
// https://www.selecolor.com/en/hsv-color-picker/

// 內置彩色LED - 色相
// 單位: 角度 - °
// 範圍: 0 to 360
#define RGB_LED_HUE 350

// 內置彩色LED - 飽和度
// 單位: 百分比 - %
// 範圍: 0 to 100
#define RGB_LED_SATURATION 75

// 內置彩色LED - 明度
// 單位: 百分比 - %
// 範圍: 0 to 100
#define RGB_LED_VALUE 10
