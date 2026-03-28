# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Project Overview

FreeRTOS-based line-tracking car on STM32F103C8T6 (Cortex-M3, 72MHz, 64KB Flash, 20KB SRAM). Four operating modes selectable via NEC IR remote: line tracking (`*`), remote control (`#`), obstacle avoidance (`0`), and power saving (`O`).

## Build System

- **Toolchain:** Keil µVision MDK-ARM v5.32 — open `MDK-ARM/Track_Car.uvprojx`
- **Code Generation:** STM32CubeMX v6.16.1 — open `Track_Car.ioc` to modify pin/peripheral config
- **Output:** `MDK-ARM/Track_Car/Track_Car.hex` (flash firmware)
- **CubeMX user code sections:** All hand-written code in `Core/Src/*.c` must be placed between `/* USER CODE BEGIN */` and `/* USER CODE END */` markers, or CubeMX regeneration will erase it.

## Architecture

```
Application/Task/       ← FreeRTOS task logic (tracking, mode selection)
Application/Control/    ← Algorithms (PID, power management)
Drivers/BSP/            ← Board Support Package (motor, PWM, sensors, IR remote)
Core/Src/               ← HAL init, FreeRTOS setup, interrupt handlers (CubeMX-managed)
Core/Inc/               ← Headers including FreeRTOSConfig.h
```

**BSP → Control → Task** dependency flow. BSP wraps raw HAL GPIO/TIM calls. Control modules use BSP for algorithms (PID steering). Tasks orchestrate control modules under FreeRTOS scheduling.

## FreeRTOS Task Map

| Task | Function | Priority | Period | Role |
|------|----------|----------|--------|------|
| `vModeSelect` | `freertos.c` | 4 | 20ms | Polls IR remote, creates/deletes mode tasks dynamically |
| `vTrackMode` | `freertos.c` → `task_tracking.c` | 5 | 50ms | Reads 8 IR sensors, runs PD control, drives motors |
| `vRemoteMode` | `freertos.c` | 5 | 50ms | Maps IR remote keys to motor commands |
| `vAvoidMode` | `freertos.c` | 5 | 50ms | Uses ultrasonic distance to navigate around obstacles |

Mode tasks are created/deleted at runtime by `vModeSelect` — only one mode task runs at a time. A `xLengthQueue` (single-item overwrite queue) passes ultrasonic distance data between tasks.

## Key Hardware Mapping

- **Motors:** Left (PA4/PA5 dir, TIM2_CH3 PWM on PA2), Right (PA0/PA1 dir, TIM2_CH4 PWM on PA3). PWM range 0–1000 at 1kHz.
- **IR Line Sensors (×8):** PA9–PA12 + PB12–PB15, packed into a single `uint8_t` bitmask by `GetRedSensorData()`.
- **Ultrasonic (HC-SR04):** Trigger PB1, Echo PB10, timed by TIM4.
- **IR Remote Receiver:** PA7 via TIM3_CH2 input capture. NEC protocol decoded in `HAL_TIM_IC_CaptureCallback()`.
- **Wake-up Button:** PA8 (EXTI falling edge, used to exit stop mode).

## PID Tuning

In `car_pid.c`: Kp=60.0, Ki=0, Kd=30.0. The `getBias()` function in `task_tracking.c` maps the 8-sensor bitmask to a signed error value; `PID()` produces a differential speed applied to left/right motors via `Control_Direction()`.

## Power Modes (power_model.c)

- **Standby Mode:** RTC alarm wake-up. Lowest power; full reset on wake.
- **Stop Mode:** EXTI8 (PA8 button) wake-up. Suspends SysTick, stops PWM, reconfigures clocks on resume.

## Conventions

- All timer prescalers are set to 72 (1µs resolution at 72MHz SYSCLK).
- Motor functions use `HAL_GPIO_WritePin` for direction + `__HAL_TIM_SET_COMPARE` for speed.
- IR remote key map: 17 keys → char array `"123456789*0#+LOR-"` indexed by NEC data byte.
- FreeRTOS heap: 10240 bytes (heap_4 scheme). Max task priority: 56. Tick rate: 1000Hz.
- Language in commit messages and comments is Chinese.
