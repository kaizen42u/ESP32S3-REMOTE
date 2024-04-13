# ESP32S3 Robot Car Remote Controller Implementation

This project is about creating a remote that can control a [robot car](https://github.com/kaizen42u/ESP32S3-CAR) using **ESP-NOW** protocol. ESP-NOW is a peer-to-peer communication protocol that allows devices to send and receive data without connecting to a Wi-Fi network. ~~This project supports multiple remote connections (up to 6 concurrent) to the same robot car.~~

## Features

- Remote with a joystick, buttons
- ESP32S3 module with built-in Wi-Fi and Bluetooth
- ESP-NOW protocol for fast and reliable communication
- ~~Multiple remote connections to the same robot car~~
- RSSI for pairing

## Requirements

- Hardware:
  - Joystick + Button module
  - ESP32S3 module
  - ~~Breadboard and wires~~
- Software:
  - Visual Studio Code
  - ESP-IDF Version 1.7.0

## Installation

- Download and install [Visual Studio Code](https://code.visualstudio.com/)
- Install [ESP-IDF extension](https://marketplace.visualstudio.com/items?itemName=espressif.esp-idf-extension)
- Connect the ESP32S3 module to your computer using USB cables
- Open the sketch folder (this) in Visual Studio Code
- Select the correct board (ESP32S3 Dev Module) and port
- Upload the sketch to the ESP32S3 module

## Usage

- Turn on the robot car and the remote
- Hold the remote close to the robot car ~~and start pressing buttons~~
- Wait for the ESP-NOW connection to be established (Pink LED lights up)
- Use the joystick and the buttons on the remote to control the robot car
- Enjoy!

## License

This project is licensed under the MIT License - see the LICENSE file for details

## Car Github Page

For more information about the robot car implementation, please visit the [robot car github page](https://github.com/kaizen42u/ESP32S3-CAR)
