# PLCDriverApp Documentation

## Overview

PLCDriverApp is a C++ application designed for controlling motor drivers via Modbus RTU protocol, utilizing Fast DDS for efficient communication between nodes. The application is built with a focus on real-time performance and reliability.

## Prerequisites

- Linux operating system
- Fast DDS and its dependencies
- `yaml-cpp` for YAML configuration file parsing
- `socat` for creating virtual serial ports for testing
- `diagslave` Modbus slave simulator for testing

## Installation

### Fast DDS

Ensure Fast DDS and its dependencies are installed. For installation instructions, refer to the [official Fast DDS documentation](https://fast-dds.docs.eprosima.com/en/latest/installation/binaries/binaries_linux.html).

### yaml-cpp

Install `yaml-cpp` using your system's package manager or build from source. For more details, visit [yaml-cpp GitHub repository](https://github.com/jbeder/yaml-cpp).

### socat

Install `socat` using your package manager:

```sh
sudo apt-get update
sudo apt-get install socat
```

### diagslave

Download and compile `diagslave` from the [official website](https://www.modbusdriver.com/diagslave.html) or use the pre-compiled binary if available for your system.

## Configuration

Create a `config.yaml` file in the root directory of the application with the following structure:


```yaml
modbus:
  device: "/dev/ptyX"
  baud_rate: 19200
  parity: 'N'
  data_bits: 8
  stop_bits: 1
```
Replace /dev/ptyX with the actual device path created by socat.

##Building the Application

Navigate to the project directory and use CMake to build the application:
```sh
mkdir build && cd build
cmake ..
cmake --build .
```

##Testing
Setting Up Virtual Serial Ports
Use socat to create a pair of connected virtual serial ports:

```sh
socat -d -d pty,rawer,echo=0 pty,rawer,echo=0
```

This command will output the paths to the created virtual serial ports (e.g., /dev/pts/7 and /dev/pts/8). Use one for the PLCDriverApp and the other for diagslave.

##Running diagslave
Run diagslave using one of the virtual serial ports:

```sh
sudo x86_64-linux-gnu/diagslave -m rtu -p none -b 19200 /dev/pts/8
```
Adjust the path /dev/pts/8 according to the output from socat.

##Running PLCDriverApp
Before running the application, ensure the config.yaml file is pointing to the correct virtual serial port (e.g., /dev/pts/7). Then, from the build directory, execute the application:

```sh
./PLCDriverApp
```
The application should now communicate with diagslave simulating Modbus RTU slave responses.


##Troubleshooting
Ensure all prerequisites are correctly installed and configured.
Verify the virtual serial ports are correctly set up and accessible by the application and diagslave.
Check the config.yaml for correct configuration parameters.
Review application logs for any error messages or warnings.
