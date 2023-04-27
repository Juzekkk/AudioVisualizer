## AudioVisualizer

AudioVisualizer is a real-time audio frequency visualizer that captures system audio and displays a transparent window with bars representing the frequency magnitudes. The project uses GLFW and OpenGL for window and graphics management, and leverages custom AudioCapture and AudioProcessor classes to handle audio input and processing.

##### Windows Only!!!!

### Features

- Real-time audio frequency visualization
- Transparent window display
- Position and size customization through system tray menu
- Additional customization available through settings.ini file

### Dependencies

- GLFW (version 3.3 or later)
- GLAD

### Building

To build the project, follow these steps:

- Install GLFW and GLAD according to their respective documentation.
- Clone the repository: git clone https://github.com/Juzekkk/AudioVisualizer.git
- Navigate to the project directory: cd AudioVisualizer
- Create a build directory and navigate to it: mkdir build && cd build
- Run CMake and compile the project:

```
    cmake ..
    make
```

- Run the executable ./AudioVisualizer

### Usage

Once the application is running, a transparent window will appear on your screen, displaying bars representing the magnitudes of different frequency ranges in real-time. The window will continuously update as the system audio changes.

To close the application, simply find its icon in the system tray rightclick and select close.

### TODO

- Complex customization through system tray menu.
- Multiplatform????
- idk
