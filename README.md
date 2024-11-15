# Turnstile Project

Welcome to the **Turnstile System** repository. This project focuses on developing an advanced turnstile system with state-of-the-art access control mechanisms, designed for secure and efficient operations.

---

## Project Overview

The turnstile system is a robust access control mechanism that supports multiple states and features:

- **Ready State**: Displays green arrows and waits for RFID authentication.
- **Reading State**: Reads RFID data and communicates with the Network Operating System (NOS) for validation.
- **Open State**: Grants access by opening the turnstile upon successful authentication.
- **Close State**: Keeps the doors closed for failed authentication or overcapacity situations.
- **Sleep State**: Triggered remotely or during emergencies; puts the system in standby mode.
- **Emergency State**: Opens doors in any direction to allow emergency evacuation.

The system integrates visual indicators, bi-directional motorized controls, and reliable communication with a centralized server, ensuring seamless and safe operation.

---

## Features

### Core Features
- **Access Control**: Bi-directional motorized gates controlled by RFID-based authentication integrated with NOS.
- **Multi-State Operation**: Ready, Reading, Open, Close, Sleep, and Emergency states.
- **Visual Indicators**: LED indicators for system status:
  - Green for success.
  - Red for authentication failure.
  - Orange for overcapacity or warnings.
- **Emergency Override**: Manual override to open doors in emergencies.

### Hardware Integration
- **Motorized Turnstile**: Uses PWM for motor speed and direction control.
- **Encoder Feedback**: Rotary encoder tracks angular position for precise control.
- **LED Indicators**: GPIO-controlled visual feedback for system status.

### Communication
- **ESP WiFi Support**: Handles server communication via MQTT and HTTP APIs.
- **UART Integration**: Debugging and data exchange with peripherals.

---

## Repository Structure

```plaintext
turnstile_project/
├── code/
│   ├── src/                   # Core application source code
│   │   ├── main.c             # Main program logic for the turnstile
│   │   ├── states.c           # Implementation of turnstile states
│   │   ├── rfid.c             # RFID reader integration and handling
│   │   ├── motor_control.c    # Bi-directional motor control logic
│   │   ├── indicators.c       # LED and indicator control
│   │   └── utils.c            # Utility functions for state management
│   ├── include/               # Header files for source code
│   │   ├── main.h             # Header for main.c
│   │   ├── states.h           # Definitions and prototypes for states.c
│   │   ├── rfid.h             # RFID integration header
│   │   ├── motor_control.h    # Motor control definitions
│   │   └── indicators.h       # Indicator control prototypes
│   ├── esp/                   # ESP module-specific code
│   │   ├── connections/       # Communication logic
│   │   │   ├── wifi.c         # WiFi setup and connection logic
│   │   │   ├── mqtt.c         # MQTT client integration
│   │   │   ├── http.c         # HTTP API handling for NOS
│   │   │   └── credentials.h  # Network and server credentials
│   │   └── esp_main.c         # ESP module’s main control logic
│   └── Makefile               # Build automation for the project
├── turnstile_stm_program/
│   ├── src/                   # STM32 source code
│   │   ├── main.c             # Core application logic
│   │   ├── stm32f4xx_it.c     # Interrupt handling for STM32 peripherals
│   │   ├── encoder.c          # Rotary encoder integration
│   │   ├── pwm.c              # PWM configuration and control logic
│   │   ├── gpio.c             # GPIO configuration for peripherals
│   │   └── uart.c             # UART communication setup and handling
│   ├── include/               # Header files for STM32 source code
│   │   ├── main.h             # Header file for main.c
│   │   ├── stm32f4xx_it.h     # Interrupt definitions
│   │   ├── encoder.h          # Encoder function prototypes
│   │   ├── pwm.h              # PWM-related definitions
│   │   ├── gpio.h             # GPIO definitions and setup
│   │   └── uart.h             # UART communication prototypes
│   ├── config/                # STM32-specific configuration files
│   │   ├── FreeRTOSConfig.h   # FreeRTOS kernel configuration
│   │   └── system_stm32f4xx.c # STM32 system initialization
│   └── Makefile               # STM32 project build configuration
├── hardware/
│   ├── schematics/            # Circuit schematics and wiring
│   │   ├── turnstile_schematic.pdf # Detailed circuit diagram
│   │   └── wiring_diagram.png      # Visual wiring diagram
│   ├── parts_list.csv         # List of hardware components
│   └── setup_guide.md         # Hardware setup guide
├── tests/                     # Unit and integration tests
│   ├── unit_tests/            # Unit tests for individual modules
│   │   ├── test_states.c      # Tests for state transitions
│   │   ├── test_rfid.c        # Tests for RFID operations
│   │   └── test_motor_control.c # Tests for motor control
│   ├── integration_tests/     # Integration tests for the complete system
│   │   ├── test_full_system.c # Full system integration tests
│   └── test_plan.md           # Comprehensive testing plan
├── docs/                      # Project documentation
│   ├── design/                # Design-related documents
│   │   ├── system_architecture.md # System architecture details
│   │   └── state_diagrams.png # State machine diagrams
│   ├── user_manual.md         # User operation manual
│   └── troubleshooting.md     # Troubleshooting guide
├── assets/                    # Visual and media assets
│   ├── images/                # Images for documentation
│   │   ├── system_render.png  # Turnstile rendered image
│   │   └── indicator_lights.png # Diagram showing indicator lights
│   ├── videos/                # Video demonstrations
│   │   └── demo.mp4           # Turnstile demo video
├── LICENSE                    # Project license
└── README.md                  # Project overview (this file)