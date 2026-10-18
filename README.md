# Smart Embedded Door Controller (RP2040 / C++)

Embedded automatic door controller running on the **Raspberry Pi Pico (RP2040 MCU)**. Written in modern C++ utilizing the Pico SDK, this firmware implements deterministic Finite State Machines (FSM), interrupt-driven hardware events, dual-layer position & collision detection (Limit Switches + Rotary Encoder), EEPROM persistent recovery, and remote IoT telemetry via MQTT over Wi-Fi.

---

## Table of Contents

- [Overview](#overview)
- [Key Technical Features](#key-technical-features)
- [Hardware & Sensing Architecture](#hardware--sensing-architecture)
- [System State Machine (FSM)](#system-state-machine-fsm)
- [Software Architecture Highlights](#software-architecture-highlights)
  - [1. Mid-Travel Stop & Direction Reversal](#1-mid-travel-stop--direction-reversal)
  - [2. Dual-Layer Obstacle & Limit Detection](#2-dual-layer-obstacle--limit-detection)
  - [3. Crash Recovery & Power-Off Persistence](#3-crash-recovery--power-off-persistence)
- [MQTT Topics & Interface](#mqtt-topics--interface)
- [Repository Structure](#repository-structure)
- [Main Application Loop (main.cpp)](#main-application-loop-maincpp)
- [Getting Started](#getting-started)
  - [Prerequisites](#prerequisites)
  - [Build and Flash](#build-and-flash)
- [License](#license)

---

## Overview

Automated physical barriers demand absolute safety, real-time responsiveness, and seamless fault handling. This firmware controls a 4-coil stepper motor to drive a door mechanism between physical bounds while constantly monitoring movement metrics and user controls.

By coupling physical limit switch interrupts with real-time feedback from a quadrature **Rotary Encoder**, the system mathematically verifies motor execution step-by-step. In addition to physical end-stop protection, the door allows mid-travel dynamic manual control: users can interrupt movement mid-way via physical switches or MQTT commands (`STOP`), and dynamically toggle movement direction (`REVERSE`).

---

## Key Technical Features

- **Dynamic Mid-Travel Control (`STOP` & `REVERSE`):** Manual user override via switch (`SW1`) or MQTT command (`STOP`) halts movement immediately. Subsequent triggers (`REVERSE`) reverse the drive direction mid-travel.
- **Dual-Layer Position & Obstacle Tracking:** Combines hardware interrupts from physical limit switches with step-to-encoder pulse validation to detect stuck doors/obstructions instantly.
- **Deterministic Finite State Machine (FSM):** Fully isolated calibration phases (`NOT_CALIBRATED`, `CALIBRATING`, `CALIBRATED`) and door operational states (`CLOSED`, `IN_BETWEEN`, `OPEN`).
- **EEPROM Fault Recovery:** Stores critical variables (state, direction, steps-per-encoder ratio, end-to-end limits) in non-volatile storage to allow automatic recovery (`recoverFromPowerOff`) following brownouts.
- **Non-Blocking Asynchronous Operations:** Utilizes Pico
