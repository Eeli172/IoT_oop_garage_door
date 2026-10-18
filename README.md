# Automatic Door & Garage Control System

Embedded automatic sliding and garage door controller built using modern C++ principles. The project features deterministic Finite State Machine (FSM) control, hardware interrupt-driven safety sensors (ISRs), RAII resource management, MQTT IoT integration for remote telemetry, and non-volatile EEPROM state recovery during brownouts or hard resets.

---

## Overview

Automated physical barriers, such as sliding gates and garage doors, require absolute safety, sub-millisecond responsiveness to obstacle detection, and deterministic fault recovery. 
This project delivers a complete firmware solution designed to bridge low-level microcontroller drivers with high-level IoT functionality. Interrupt services ensure that limit switches and optical obstacle curtains take hardware priority over asynchronous tasks like MQTT messaging and status publishing.

---

## Key Features

- **Hardware Interrupt-Driven Sensing (ISRs):** Instantaneous limit switch detection and safety curtain triggering with sub-millisecond response time.
- **Object-Oriented Design (OOP):** Modular architecture separating hardware driver abstractions, network handling, and control logic.
- **RAII Resource Management:** Safe initialization and deterministic cleanup of hardware peripherals, GPIO pins, and interrupt instances.
- **Persistent Memory (EEPROM):** Preserves operating state, cycle counts, and calibration parameters across power loss or system resets with checksum verification.
- **Remote Telemetry (MQTT):** Bi-directional communication for remote opening/closing, status monitoring, and real-time fault reporting.
- **Failsafe State Machine:** Deterministic state handling covering timeouts, obstruction reversals, and emergency manual lockouts.

---

## Hardware Architecture

| Component | Purpose / Function |
| :--- | :--- |
| **Microcontroller** | ESP32 / STM32 / ATmega328P |
| **Actuators** | Dual H-Bridge Driver / Relay Module (Open/Close Control) |
| **Limit Switches** | Mechanical/Magnetic End-of-Travel Switches (Interrupt Driven) |
| **Safety Sensor** | Infrared Optical Beam / Safety Edge Sensor |
| **Network Interface** | Wi-Fi / Ethernet Module for MQTT Connection |
| **Status Indicators** | Status LEDs & Local Buzzer |

---

## System State Machine

The door operation is governed by a deterministic Finite State Machine:

```mermaid
stateDiagram-v2
    [*] --> Initializing
    Initializing --> Closed : EEPROM Verified / System Ready
    Closed --> Opening : Remote Command / Switch Trigger
    Opening --> Open : Limit Switch ISR Triggered
    Open --> Closing : Auto-Close Timer / Command
    Closing --> Closed : Limit Switch ISR Triggered
    Closing --> Opening : Obstacle ISR Triggered
    Opening --> Fault : Movement Timeout Exceeded
    Closing --> Fault : Movement Timeout Exceeded
    Fault --> Initializing : Manual Reset Command
