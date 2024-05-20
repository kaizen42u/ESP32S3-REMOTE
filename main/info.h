#pragma once

/* ---> Wi-Fi設定 <--- */

// 上傳新代碼時清除舊連線 (Clears ROM)
// 可選: true, false
// 默認: true
#define CLEAR_PAIRED_PEER_ON_NEW_UPLOAD true

// 每隔3秒列出Wi-Fi連線狀態
// 可選: true, false
// 默認: true
#define SHOW_CONNECTION_STATUS false

// 訊號強度大於該值時將嘗試建立連線
// 單位: 分貝毫瓦 - dBm
// 範圍: 0 to -100
// 默認: -20
#define MIN_RSSI_TO_INITIATE_CONNECTION -20

/* ---> 內置彩色LED設定 <--- */
// https://www.selecolor.com/en/hsv-color-picker/

// 內置彩色LED - 色相
// 單位: 角度/相位 - °
// 範圍: 0 to 360
// 默認: 350
#define RGB_LED_HUE 350

// 內置彩色LED - 飽和度
// 單位: 百分比 - %
// 範圍: 0 to 100
// 默認: 75
#define RGB_LED_SATURATION 75

// 內置彩色LED - 明度
// 單位: 百分比 - %
// 範圍: 0 to 100
// 默認: 10
#define RGB_LED_VALUE 10
