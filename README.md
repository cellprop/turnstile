# Bi-Directional RFID Turnstile

This repository contains the electronic control system for a bi-directional turnstile with RFID authentication for entry and exit.

## Project Overview

This turnstile system includes:
- RFID authentication for both entry and exit
- Bi-directional movement control
- Limit switches for door position sensing
- Motor control for smooth operation
- MQTT communication for remote monitoring and control

## Repository Structure

```
.
├── README.md
├── src/
│   ├── rfid_sensor/
│   ├── limit_switches/
│   ├── motor_control/
│   └── main_controller/
├── schematics/
├── docs/
└── tests/
```

## Installation

1. Clone this repository:
   ```bash
   git clone https://github.com/cellprop/turnstile.git
   ```
2. Install the required dependencies.

## Hardware Requirements
- RFID sensor module
- Limit switches
- Motor and motor driver
- LED Matrix
- Microcontroller (STM32 Board)

## MQTT Connection Error Codes

When setting up MQTT communication, you may encounter connection issues. Below are the possible return codes (`rc` values) when an MQTT connection fails and their meanings:

- **`rc = -4`**: **MQTT_CONNECTION_TIMEOUT**  
  The connection attempt timed out. This could indicate network issues, a firewall blocking the connection, or the broker being down.

- **`rc = -3`**: **MQTT_CONNECTION_LOST**  
  The connection was lost after being established. This can happen due to network interruptions or an unexpected broker disconnection.

- **`rc = -2`**: **MQTT_CONNECT_FAILED**  
  The client failed to establish a connection, which might be caused by incorrect server details or the broker rejecting the connection.

- **`rc = -1`**: **MQTT_DISCONNECTED**  
  The client is disconnected from the broker. This could be due to a manual disconnect or failure to maintain the connection.

- **`rc = 0`**: **MQTT_CONNECTED**  
  The client is successfully connected to the broker.

- **`rc = 1`**: **Connection Refused, Unacceptable Protocol Version**  
  The MQTT broker does not support the protocol version used by the client.

- **`rc = 2`**: **Connection Refused, Identifier Rejected**  
  The client identifier is invalid. Ensure it is not empty or duplicated.

- **`rc = 3`**: **Connection Refused, Server Unavailable**  
  The broker is unavailable, possibly due to maintenance, overload, or connection limits.

- **`rc = 4`**: **Connection Refused, Bad Username or Password**  
  The credentials provided by the client are incorrect.

- **`rc = 5`**: **Connection Refused, Not Authorized**  
  The client lacks permission to connect. Check for incorrect credentials or access restrictions.

These error codes can help diagnose issues during MQTT setup and ensure a stable connection between the turnstile system and the MQTT broker.

## Program Logic:


-------------------------------------------------------
1. Includes and Definitions
-------------------------------------------------------
*/

/* Include necessary header files */
#include "main.h"
#include "dma.h"
#include "tim.h"
#include "usart.h"
#include "gpio.h"
#include "ws28xx.h"   // WS28XX LED library

/* System State Enumeration */
/* 
Defines an enumeration for different system states to be used in the state machine.
*/
typedef enum {
    STATE_READY,
    STATE_READING,
    STATE_OPEN,
    STATE_CLOSED,
    STATE_OVERCAPACITY,
    STATE_SLEEP,
    STATE_EMERGENCY
} SystemState;

/* Constants */
/* 
Sets constants for the LED matrix dimensions and timing delays for animations.
*/
#define LED_ROWS 8
#define LED_COLS 32
#define DELAY_TIME 50
#define BLINK_DELAY 500

/* 
-------------------------------------------------------
2. Global Variables
-------------------------------------------------------
*/

/* Peripheral Handles and State Variables */
/* 
- ws: Handle for the WS28XX LED matrix.
- currentState: Tracks the current state in the state machine, starting with STATE_READY.
*/
WS28XX_HandleTypeDef ws;
SystemState currentState = STATE_READY; // Initialize to Ready State

/* UART Communication Variables */
/* 
Handles UART communication by storing received data, processing it, and managing flags for data reception and authentication results.
*/
uint8_t rxData[14];        // Buffer for UART data reception
uint8_t processedData[14]; // Buffer to store processed UART data
char usermsg[13];          // Message to send via UART
uint8_t flag_rev = 0;      // Flag indicating data reception
uint8_t uart_source = 0;   // Identifies UART source (1 for USART1, 2 for USART2)

uint8_t responseData;      // Stores response from NOS via USART3
uint8_t flagSuccess = 0;   // Flag indicating successful authentication
uint8_t flagFailure = 0;   // Flag indicating failed authentication

/* LED Animation Variables */
/* 
Stores the LED indices that form specific shapes (arrow and cross) on the LED matrix for animations.
*/
int arrow[] = { /* Indices representing the arrow shape on the LED matrix */ };
int topCross[] = { /* Indices for the top part of the cross shape */ };
int middleCross[] = { /* Indices for the middle part of the cross shape */ };
int bottomCross[] = { /* Indices for the bottom part of the cross shape */ };

/* Motor Control Variables */
/* 
- counter: Keeps track of encoder pulses to determine motor position.
- rev: Can be used to count complete motor revolutions if needed.
*/
volatile int counter = 0; // Counts encoder pulses
volatile int rev = 0;     // Tracks motor revolutions (unused in this code)

/* 
-------------------------------------------------------
3. LED Matrix Functions
-------------------------------------------------------
*/

/* Function to calculate the correct LED index based on the zig-zag pattern */
/* 
Calculates the correct LED index based on the row and column, accounting for the zig-zag wiring pattern of the LED matrix.
*/
int Get_LED_Index(int row, int col) {
    if (row % 2 == 0) {
        return (row * LED_COLS / 2) + col;
    } else {
        return (row * LED_COLS / 2) + (LED_COLS / 2 - 1 - col);
    }
}

/* Function to draw the arrow */
/* 
Lights up the LEDs corresponding to the arrow shape with a specified color and brightness.
*/
void Draw_Arrow(WS28XX_HandleTypeDef *ws, int color, int brightness) {
    for (int i = 0; i < 26; i++) {
        if (arrow[i] >= 16 && arrow[i] <= 239) {
            WS28XX_SetPixel_RGBW_565(ws, arrow[i], color, brightness);
        }
    }
    WS28XX_Update(ws);
}

/* Function to clear the arrow */
/* 
Turns off the LEDs corresponding to the arrow shape.
*/
void Clear_Arrow(WS28XX_HandleTypeDef *ws) {
    for (int i = 0; i < 26; i++) {
        if (arrow[i] >= 16 && arrow[i] <= 239) {
            WS28XX_SetPixel_RGBW_565(ws, arrow[i], COLOR_RGB565_BLACK, 0);
        }
    }
    WS28XX_Update(ws);
}

/* Function to shift the arrow forward */
/* 
Moves the arrow shape forward on the LED matrix to create a moving animation effect.
*/
void Shift_Arrow() {
    for (int i = 0; i < 26; i++) {
        arrow[i] += 8;
        if (arrow[i] >= 240) {
            arrow[i] = arrow[i] % 240 + 16; // Wrap around to stay within valid indices
        }
    }
}

/* Function for arrow animation */
/* 
Manages the arrow animation by periodically shifting and redrawing the arrow. The reset parameter resets the animation state when needed.
*/
void Arrow_Animation(uint8_t reset) {
    static uint32_t lastUpdate = 0;
    static uint8_t initialized = 0;

    if (reset) {
        initialized = 0;
        lastUpdate = HAL_GetTick();
        return;
    }

    // Initialize on first call
    if (!initialized) {
        Draw_Strips(&ws, COLOR_RGB565_CYAN, 255);
        initialized = 1;
    }

    // Update animation based on DELAY_TIME
    if (HAL_GetTick() - lastUpdate >= DELAY_TIME) {
        lastUpdate = HAL_GetTick();

        Clear_Arrow(&ws);
        Shift_Arrow();
        Draw_Arrow(&ws, COLOR_RGB565_GREEN, 255);
    }
}

/* Function for cross animation */
/* 
Creates a blinking cross animation by toggling the cross LEDs on and off at intervals defined by BLINK_DELAY. The reset parameter resets the animation state.
*/
void Cross_Animation(uint8_t reset) {
    static uint32_t lastUpdate = 0;
    static uint8_t isOn = 0;
    static uint8_t initialized = 0;

    if (reset) {
        initialized = 0;
        isOn = 0;
        lastUpdate = HAL_GetTick();
        return;
    }

    // Initialize on first call
    if (!initialized) {
        Draw_Strips(&ws, COLOR_RGB565_CYAN, 255);
        initialized = 1;
    }

    // Toggle the cross based on BLINK_DELAY
    if (HAL_GetTick() - lastUpdate >= BLINK_DELAY) {
        lastUpdate = HAL_GetTick();

        if (isOn) {
            Clear_Cross(&ws, topCross);
            Clear_Cross(&ws, middleCross);
            Clear_Cross(&ws, bottomCross);
            isOn = 0;
        } else {
            Draw_Cross(&ws, topCross, COLOR_RGB565_RED, 255);
            Draw_Cross(&ws, middleCross, COLOR_RGB565_RED, 255);
            Draw_Cross(&ws, bottomCross, COLOR_RGB565_RED, 255);
            isOn = 1;
        }
    }
}

/* 
-------------------------------------------------------
4. Motor Control Functions
-------------------------------------------------------
*/

/* Function to control motor speed */
/* 
Adjusts the motor speed by setting the PWM duty cycle.
*/
void Speed_Control(int a) {
    __HAL_TIM_SetCompare(&htim4, TIM_CHANNEL_1, a);
}

/* Function to set motor direction */
/* 
Sets the motor rotation direction by controlling a GPIO pin.
*/
void Direction(int a) {
    if (a == 1) {
        HAL_GPIO_WritePin(GPIOC, GPIO_PIN_12, GPIO_PIN_SET);
    }
    if (a == 0) {
        HAL_GPIO_WritePin(GPIOC, GPIO_PIN_12, GPIO_PIN_RESET);
    }
}

/* Encoder interrupt callback function */
/* 
Interrupt handler that is called when a GPIO pin configured for external interrupt is triggered.
*/
void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin) {
    if (GPIO_Pin == GPIO_PIN_3) {
        encoder();
    }
}

/* Encoder function */
/* 
Increments the counter each time an encoder pulse is detected. Stops the motor after counting 588 pulses.
*/
void encoder(void) {
    counter++;
    if (counter == 588) {
        Speed_Control(0);
        counter = 0;
    }
}

/* Functions to rotate the motor */
/* 
Controls the motor to rotate a quarter cycle in clockwise (cw) or anticlockwise (acw) direction at a set speed.
*/
void quarter_cycle_cw(void) {
    Direction(0);
    Speed_Control(1000);
}

void quarter_cycle_acw(void) {
    Direction(1);
    Speed_Control(1000);
}

/* 
-------------------------------------------------------
5. UART Communication
-------------------------------------------------------
*/

/* UART receive complete callback function */
/* 
Handles UART reception interrupts for three UART instances (USART1, USART2, USART3).
*/
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart) {
    if (huart->Instance == USART1) { // Data received from USART1
        // Process the received data
        for (int i = 0; i < 12; i++) {
            processedData[i] = rxData[i + 1]; // Skip the start byte
        }

        uart_source = 1;
        processedData[12] = uart_source + '0'; // Convert to character '1'
        processedData[13] = '\0';              // Null-terminate the string

        // Use sprintf to format usermsg with identification flag
        sprintf(usermsg, "%s", processedData);

        flag_rev = 1; // Set flag to indicate data has been received

        // Re-enable UART reception for USART1
        HAL_UART_Receive_IT(&huart1, rxData, sizeof(rxData));
    }
    else if (huart->Instance == USART2) { // Data received from USART2
        // Process the received data
        for (int i = 0; i < 12; i++) {
            processedData[i] = rxData[i + 1]; // Skip the start byte
        }

        uart_source = 2;
        processedData[12] = uart_source + '0'; // Convert to character '2'
        processedData[13] = '\0';              // Null-terminate the string

        // Use sprintf to format usermsg with identification flag
        sprintf(usermsg, "%s", processedData);

        flag_rev = 1; // Set flag to indicate data has been received

        // Re-enable UART reception for USART2
        HAL_UART_Receive_IT(&huart2, rxData, sizeof(rxData));
    }
    else if (huart->Instance == USART3) { // NOS Response (USART3)
        if (responseData == 1) {
            flagSuccess = 1;
        } else if (responseData == 0) {
            flagFailure = 1;
        }

        // Re-enable UART reception for USART3
        HAL_UART_Receive_IT(&huart3, &responseData, 1);
    }
}

/* 
-------------------------------------------------------
6. State Machine Implementation
-------------------------------------------------------
*/

/* Main loop */
/* 
Continuously checks the currentState variable and calls the corresponding state function in an infinite loop.
*/
int main(void) {
    // Initialization code...

    while (1) {
        switch (currentState) {
            case STATE_READY:
                ready_state();
                break;
            case STATE_READING:
                reading_state();
                break;
            case STATE_OPEN:
                open_state();
                break;
            case STATE_CLOSED:
                closed_state();
                break;
            // Other states can be added here
            default:
                break;
        }
    }
}

/* State functions */

/* Ready state function */
/* 
- Displays the arrow animation to indicate the system is ready.
- Monitors for incoming data (flag_rev == 1) and transitions to STATE_READING.
*/
void ready_state(void) {
    static uint8_t state_initialized = 0;

    if (state_initialized == 0) {
        // Reset the arrow animation
        Arrow_Animation(1); // Reset animation
        state_initialized = 1;
    }

    // Call the Arrow Animation function
    Arrow_Animation(0); // Continue animation

    // Check if data has been received
    if (flag_rev == 1) {
        state_initialized = 0; // Reset for next time
        currentState = STATE_READING; // Transition to Reading State
    }
}

/* Reading state function */
/* 
- Sends the received user message (usermsg) to the NOS system via USART3.
- Waits for authentication response and transitions to STATE_OPEN or STATE_CLOSED accordingly.
*/
void reading_state(void) {
    if (flag_rev == 1) {
        HAL_UART_Transmit_IT(&huart3, (uint8_t *)usermsg, strlen(usermsg));
        flag_rev = 0;
    }
    HAL_Delay(1000); // Delay as needed

    // Check for authentication result
    if (flagSuccess == 1) {
        flagSuccess = 0;
        currentState = STATE_OPEN; // Transition to Open State
    }
    else if (flagFailure == 1) {
        flagFailure = 0;
        currentState = STATE_CLOSED; // Transition to Closed State
    }
}

/* Open state function */
/* 
- Manages the opening and closing of doors upon successful authentication.
- Uses the motor control functions to open the doors, waits for a predetermined time, then closes the doors.
- Transitions back to STATE_READY after completing the cycle.
*/
void open_state(void) {
    static uint8_t state_initialized = 0;
    static uint8_t door_stage = 0;
    static uint32_t timestamp = 0;

    if (state_initialized == 0) {
        // State initialization
        quarter_cycle_cw(); // Open doors
        door_stage = 0;
        state_initialized = 1;
    }

    if (door_stage == 0 && counter >= 588) {
        // Doors opened
        Speed_Control(0); // Stop motor
        counter = 0;
        door_stage = 1;
        timestamp = HAL_GetTick(); // Record time
    }

    if (door_stage == 1) {
        // Wait for person to pass (e.g., 5 seconds)
        if (HAL_GetTick() - timestamp >= 5000) {
            quarter_cycle_acw(); // Close doors
            door_stage = 2;
        }
    }

    if (door_stage == 2 && counter >= 588) {
        // Doors closed
        Speed_Control(0); // Stop motor
        counter = 0;
        door_stage = 0;
        state_initialized = 0;
        currentState = STATE_READY; // Transition back to Ready State
    }
}

/* Closed state function */
/* 
- Displays the cross animation to indicate access denial.
- After a delay, resets the animation and transitions back to STATE_READY.
*/
void closed_state(void) {
    static uint8_t state_initialized = 0;
    static uint32_t timestamp = 0;

    if (state_initialized == 0) {
        // Reset the cross animation
        Cross_Animation(1); // Reset animation
        timestamp = HAL_GetTick(); // Record time
        state_initialized = 1;
    }

    // Call the Cross Animation function
    Cross_Animation(0); // Continue animation

    // Wait for some time (e.g., 3 seconds)
    if (HAL_GetTick() - timestamp >= 3000) {
        // Clear cross
        Cross_Animation(1); // Reset animation for next time
        state_initialized = 0;
        currentState = STATE_READY; // Transition back to Ready State
    }
}

/* 
-------------------------------------------------------
7. System Initialization
-------------------------------------------------------
*/

/* Peripheral Initialization */
/* 
Initializes the necessary peripherals for GPIO, DMA, UART communications, and timers.
*/
void System_Init(void) {
    MX_GPIO_Init();
    MX_DMA_Init();
    MX_USART2_UART_Init();
    MX_TIM3_Init();
    MX_TIM4_Init();
    MX_USART1_UART_Init();
    MX_USART3_UART_Init();
}

/* LED Matrix Initialization */
/* 
Initializes the WS28XX LED matrix with the specified timer settings.
*/
void LED_Matrix_Init(void) {
    WS28XX_Init(&ws, &htim3, 72, TIM_CHANNEL_1, 256);
}

/* UART Reception Initialization */
/* 
Begins UART reception in interrupt mode for all UART interfaces.
*/
void UART_Reception_Init(void) {
    // Start UART reception for RFID Reader (USART1)
    HAL_UART_Receive_IT(&huart1, rxData, sizeof(rxData));

    // Start UART reception for Additional UART (USART2)
    HAL_UART_Receive_IT(&huart2, rxData, sizeof(rxData));

    // Start UART reception for NOS response (USART3)
    HAL_UART_Receive_IT(&huart3, &responseData, 1);
}

/* 
-------------------------------------------------------
8. Error Handling
-------------------------------------------------------
*/

/* Error Handler Function */
/* 
Disables interrupts and enters an infinite loop in case of a critical error.
*/
void Error_Handler(void) {
    __disable_irq();
    while (1) {
        // Stay here in case of error
    }
}

/* 
-------------------------------------------------------
9. Additional Notes
-------------------------------------------------------

- UART Buffer Management:
  - A single buffer rxData is used for both USART1 and USART2. This could lead to data corruption if data is received on both UARTs simultaneously.
  - Recommendation: Use separate buffers for each UART to avoid potential conflicts.

- Buffer Sizes:
  - The usermsg buffer is declared with a size of 13, but there is a potential for buffer overflow if more data is written to it.
  - Recommendation: Ensure that the buffer sizes are sufficient to hold all data, including any additional characters and the null terminator.

- Data Conversion:
  - Adding '0' to an integer (uart_source + '0') converts the integer to its corresponding ASCII character. For example, 1 + '0' results in '1'.

- Unused or Placeholder Functions:
  - Functions like overcapacity_state, sleep_state, and emergency_state are declared but not implemented.
  - Recommendation: Implement these functions or remove them if they are not needed.

- Commented-Out Code:
  - The limit_switch function contains commented-out code.
  - Recommendation: Remove or uncomment and implement the code as necessary.

- Include Necessary Headers:
  - Functions like memcpy are used without including the <string.h> header file.
  - Recommendation: Add #include <string.h> to include string manipulation functions.
*/

/* 
-------------------------------------------------------
10. Overall Program Flow
-------------------------------------------------------

1. Initialization:
   - The system initializes peripherals and variables.
   - UART reception is started for all UART interfaces.
   - The LED matrix is initialized.
   - The system enters the STATE_READY state.

2. State Machine Execution:
   - The main loop continuously checks the currentState and calls the corresponding state function.

3. Ready State (STATE_READY):
   - Displays an arrow animation on the LED matrix.
   - Waits for data reception (flag_rev == 1).
   - Transitions to STATE_READING when data is received.

4. Reading State (STATE_READING):
   - Sends the received user message (usermsg) over USART3 for authentication.
   - Waits for authentication response (flagSuccess or flagFailure).
   - Transitions to STATE_OPEN on success or STATE_CLOSED on failure.

5. Open State (STATE_OPEN):
   - Opens the doors using motor control functions.
   - Waits for a set time to allow a person to pass through.
   - Closes the doors after the wait period.
   - Returns to STATE_READY after completing the door cycle.

6. Closed State (STATE_CLOSED):
   - Displays a cross animation to indicate access denial.
   - Waits for a set time before resetting.
   - Returns to STATE_READY after the wait period.
*/




