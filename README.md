# Smart Embedded Door Controller (RP2040 / C++)

An industrial-grade, embedded automatic door controller running on the **Raspberry Pi Pico (RP2040 MCU)**. Written in modern C++ utilizing the Pico SDK, this firmware implements deterministic Finite State Machines (FSM), interrupt-driven hardware events, dual-layer position & collision detection (Limit Switches + Rotary Encoder), EEPROM persistent recovery, and remote IoT telemetry via MQTT over Wi-Fi.

---

## Table of Contents

- [Overview](#overview)
- [Key Technical Features](#key-technical-features)
- [Hardware & Sensing Architecture](#hardware--sensing-architecture)
- [System State Machine (FSM)](#system-state-machine-fsm)
- [Software Architecture Highlights](#software-architecture-highlights)
  - [1. Dual-Layer Obstacle & Limit Detection](#1-dual-layer-obstacle--limit-detection)
  - [2. Crash Recovery & Power-Off Persistence](#2-crash-recovery--power-off-persistence)
  - [3. Modern C++ & Memory Management](#3-modern-c--memory-management)
- [MQTT Topics & Interface](#mqtt-topics--interface)
- [Repository Structure](#repository-structure)
- [Main Application Loop (main.cpp)](#main-application-loop-maincpp)
- [Getting Started](#getting-started)
  - [Prerequisites](#prerequisites)
  - [Build and Flash](#build-and-flash)
- [License](#license)

---

## Overview

Automated physical barriers demand absolute safety, real-time responsiveness, and seamless fault handling. This firmware controls a 4-coil stepper motor to drive a door mechanism between physical bounds while constantly monitoring movement metrics.

By coupling physical limit switch interrupts with real-time feedback from a quadrature **Rotary Encoder**, the system mathematically verifies motor execution step-by-step. If an obstacle prevents physical movement, the system instantly detects step desynchronization, triggers a safety fault, and notifies the network via MQTT.

---

## Key Technical Features

- **Dual-Layer Position & Obstacle Tracking:** Combines hardware interrupts from physical limit switches with step-to-encoder pulse validation to detect stuck doors/obstructions instantly.
- **Deterministic Finite State Machine (FSM):** Fully isolated calibration phases (`NOT_CALIBRATED`, `CALIBRATING`, `CALIBRATED`) and door operational states (`CLOSED`, `IN_BETWEEN`, `OPEN`).
- **EEPROM Fault Recovery:** Stores critical variables (state, direction, steps-per-encoder ratio, end-to-end limits) in non-volatile storage to allow automatic recovery (`recoverFromPowerOff`) following brownouts.
- **Non-Blocking Asynchronous Operations:** Utilizes Pico absolute timers (`absolute_time_t`) for non-blocking status LED updates, Wi-Fi heartbeats, and MQTT event polling.
- **Remote IoT Control (MQTT):** Bi-directional command parsing (`OPEN`, `CLOSE`, `REVERSE`) and status publishing via Wi-Fi.

---

## Hardware & Sensing Architecture

| Component | Hardware Interface | Purpose / Description |
| :--- | :--- | :--- |
| **RP2040 MCU** | Raspberry Pi Pico SDK | Core microcontroller executing real-time control loops and ISR queues. |
| **Stepper Motor** | GPIOs (`PIN_COIL_1..4`) | 4-wire stepper motor for precise position-controlled opening/closing. |
| **Rotary Encoder** | Quadrature GPIOs (`PIN_ENCODER_A/B`) | Optical feedback tracking physical door travel relative to motor steps. |
| **Limit Switches** | Interrupt GPIOs (`PIN_SW0`, `PIN_SW1`, `PIN_SW2`) | Metallic end-stop plates triggering hardware interrupts at full physical bounds. |
| **EEPROM** | I2C / SPI Memory | Non-volatile storage for door state, direction, and encoder calibration data. |
| **Wi-Fi / MQTT** | On-board Wi-Fi | Wireless interface for remote MQTT broker communications. |

---

## System State Machine (FSM)

The top-level architecture operates on a nested Finite State Machine governing calibration and door operations:

```mermaid
stateDiagram-v2
    [*] --> NOT_CALIBRATED
    
    state Calibration_FSM {
        NOT_CALIBRATED --> CALIBRATING : Dual Button Press (SW0 + SW2)
        CALIBRATING --> NOT_CALIBRATED : Invalid Step Ratio / Error
        CALIBRATING --> CALIBRATED : Calibrated & Saved to EEPROM
    }

    state Door_Operation_FSM {
        CALIBRATED --> CLOSED : Init Default
        CLOSED --> IN_BETWEEN : Open Cmd (SW1 / MQTT)
        IN_BETWEEN --> OPEN : Reached Open Limit Switch
        IN_BETWEEN --> CLOSED : Reached Close Limit Switch / Reversed
        IN_BETWEEN --> NOT_CALIBRATED : Stuck Detected (Encoder Mismatch)
        OPEN --> IN_BETWEEN : Close Cmd (SW1 / MQTT)
    }
