# Smart Door Lock with RFID 🏠

An automated access control system utilizing an ESP32, an MFRC522 RFID module, and a Servo motor to manage door locking and unlocking, featuring a local web monitor for access logs.

## Overview

This project implements a secure, electronic door lock that grants access only upon presenting an authorized RFID tag. The system uses a servo motor as the locking mechanism and provides immediate feedback through an integrated buzzer. The ESP32 hosts a web server to allow users to remotely monitor the door's state and review the history of access attempts.

## Features

* **RFID Access Control:** Reads 13.56MHz RFID tags (MIFARE) and verifies against a pre-authorized list.
* **Actuator Control:** Uses a **Servo Motor** to reliably lock and unlock the door.
* **Audio Feedback:** A **Buzzer** provides distinct tones for successful access (granted) and failed attempts (denied/alarm).
* **Access Logging:** Maintains a rolling log of recent access attempts, including UIDs and results.
* **Web Monitoring:** A local Wi-Fi web interface displays the current **Door State** and the **Access Log**.

## Hardware Requirements

| Component | Quantity | Notes |
| :--- | :--- | :--- |
| **ESP32** or **ESP8266** | 1 | Microcontroller with Wi-Fi and multiple GPIOs. |
| **MFRC522 RFID Module** | 1 | 13.56MHz RFID Reader/Writer. |
| **Micro Servo Motor (SG90/MG995)** | 1 | Used as the lock mechanism. |
| **Passive Buzzer Module** | 1 | For audible feedback and alerts. |
| **RFID Tags/Cards** | Various | At least one authorized tag for testing. |
| **Jumper Wires** | Various | |

## Wiring Diagram

**1. MFRC522 RFID Module (SPI Connection):**

| MFRC522 Pin | ESP32 Pin | Notes |
| :--- | :--- | :--- |
| **VCC** | **3.3V** | The MFRC522 runs on 3.3V logic. |
| **GND** | **GND** | |
| **RST** | **GPIO 22** | Reset Pin (`RST_PIN`) |
| **MISO** | **GPIO 19** | SPI Master In, Slave Out (MISO) |
| **MOSI** | **GPIO 23** | SPI Master Out, Slave In (MOSI) |
| **SCK** | **GPIO 18** | SPI Clock (SCK) |
| **SDA (SS/CS)** | **GPIO 21** | Slave Select (`SS_PIN`) |

**2. Servo Motor:**
* **VCC** $\rightarrow$ **External 5V** (Recommended for robust operation) or ESP32 5V
* **GND** $\rightarrow$ **ESP32 GND**
* **Signal** $\rightarrow$ **ESP32 GPIO 18** (`SERVO_PIN`)

**3. Passive Buzzer:**
* **VCC** $\rightarrow$ **ESP32 3.3V**
* **GND** $\rightarrow$ **ESP32 GND**
* **Signal (IN)** $\rightarrow$ **ESP32 GPIO 19** (`BUZZER_PIN`)

## Software Setup and Installation

1.  **Arduino IDE:** Ensure the ESP32 board core is installed.
2.  **Libraries:**
    * `WiFi.h`, `WebServer.h`, `SPI.h` (Built-in)
    * `MFRC522` (Install via Library Manager)
    * `ESP32Servo` (Install via Library Manager)
3.  **Configuration:**
    * Update the `ssid` and `password` variables.
    * **Crucial Step:** Use the MFRC522 library's example sketch (`DumpInfo`) to find the 4-byte UID of your authorized tags. Replace the placeholder values in the `authorizedUids` array with your actual UIDs.
    * Adjust `LOCK_POS` and `UNLOCK_POS` to fit your servo/door mechanism.
4.  **Upload:** Upload the code to the ESP32.
5.  **Access:** Open the Serial Monitor to find the assigned **IP Address**. Access this IP in a web browser to view the monitoring log.
