# Motor Control Widget

A modular Qt widget for controlling stepper motors via serial communication, designed for easy integration into other projects.

## Features

- **Modular Design**: Can be easily integrated into any Qt application
- **Serial Communication**: Direct G-code command interface
- **Axis Control**: Independent X, Y, Z axis control with direction correction
- **Direct Commands**: Send custom G-code commands via built-in terminal
- **Position Monitoring**: Real-time position feedback and tracking
- **Emergency Stop**: Safety features for immediate motor stop
- **Directional Controls**: 8-direction movement pad with home function

## Project Structure

```
├── MotorControlWidget.h/cpp    # Main motor control widget (modular)
├── TinybeeController.h/cpp     # Serial communication controller
├── ExampleIntegration.h/cpp    # Example showing integration into other projects
├── main.cpp                    # Standalone application entry point
├── CMakeLists.txt              # Build configuration
└── README.md                   # This file
```

## Quick Start

### Standalone Application

```bash
mkdir build && cd build
cmake ..
make
./ControlMotor
```

### Integration into Your Project

1. **Add files to your project:**

   - `MotorControlWidget.h/cpp`
   - `TinybeeController.h/cpp`

2. **Add Qt dependencies:**

   ```cmake
   find_package(Qt6 REQUIRED COMPONENTS Widgets SerialPort)
   target_link_libraries(your_app Qt6::Widgets Qt6::SerialPort)
   ```

3. **Use in your code:**

   ```cpp
   #include "MotorControlWidget.h"

   // Create widget
   MotorControlWidget* motorControl = new MotorControlWidget();

   // Connect signals
   connect(motorControl, &MotorControlWidget::connectionStatusChanged,
           this, &YourClass::onMotorStatusChanged);
   connect(motorControl, &MotorControlWidget::positionChanged,
           this, &YourClass::onPositionUpdate);

   // Show motor control
   motorControl->show();
   ```

## API Reference

### MotorControlWidget

#### Public Methods

```cpp
bool isConnected() const;                      // Check connection status
void showWidget();                             // Show the control widget
void hideWidget();                             // Hide the control widget
void connectToPort(const QString& portName);   // Connect to specific port
void disconnectFromPort();                     // Disconnect from port
void sendCustomCommand(const QString& cmd);    // Send G-code command
```

#### Signals

```cpp
void connectionStatusChanged(bool connected);           // Connection status change
void positionChanged(const QString& axis, double pos); // Position updates
void errorOccurred(const QString& error);              // Error notifications
void commandExecuted(const QString& cmd, const QString& response); // Command feedback
```

## Motor Direction Configuration

The widget automatically handles direction correction for different motor setups:

- **X and Z axes**: Direction is reversed (+ button moves negative direction)
- **Y axis**: Normal direction (+ button moves positive direction)

This can be modified in `MotorControlWidget::axisMoveStep()` and `MotorControlWidget::directionalClicked()`.

## Direct Command Interface

The widget includes a built-in command terminal for sending custom G-code:

- **G1 X10 Y20 F3000**: Move to position
- **G28**: Home all axes
- **G28 X**: Home X axis only
- **M114**: Request current position
- **M112**: Emergency stop

## Supported G-codes

| Command     | Description                              |
| ----------- | ---------------------------------------- |
| G1 X Y Z F  | Linear move to coordinates with feedrate |
| G28 [X Y Z] | Home axes (all if no axis specified)     |
| M114        | Get current position                     |
| M112        | Emergency stop                           |
| M115        | Get firmware info                        |

## Integration Example

See `ExampleIntegration.h/cpp` for a complete example of how to integrate the motor control widget into an existing application.

### Key Integration Points

1. **Button to open/close motor control**
2. **Status monitoring in main application**
3. **Position updates in main UI**
4. **Error handling**

## Serial Configuration

- **Baud Rate**: 115200
- **Data Bits**: 8
- **Parity**: None
- **Stop Bits**: 1
- **Flow Control**: None

## Dependencies

- Qt6 (or Qt5) Widgets
- Qt6 (or Qt5) SerialPort
- C++17 compatible compiler

## Building

```bash
# Create build directory
mkdir build && cd build

# Configure with CMake
cmake ..

# Build
make

# Run
./ControlMotor
```

## Customization

### Changing Motor Directions

Edit the direction logic in:

- `MotorControlWidget::axisMoveStep()` - for +/- buttons
- `MotorControlWidget::directionalClicked()` - for directional pad

### Adding New Commands

Add custom G-code commands in:

- `TinybeeController::buildCommandString()` - for predefined commands
- Use `sendCustomCommand()` for arbitrary commands

### UI Customization

The widget layout can be modified in `MotorControlWidget::setupUI()`.

## License

This project is open source. Please refer to your organization's licensing requirements.
