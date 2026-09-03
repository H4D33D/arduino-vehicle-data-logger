# Arduino OBD-II Vehicle Data Logger

![Completed Arduino OBD-II data logger](images/completed-obd2-logger-1.png)

**Project period:** Spring 2024

An embedded automotive data-logging system built with an Arduino Nano and a Freematics OBD-II UART adapter. The device reads live vehicle information, displays key parameters on an OLED screen, and records vehicle speed and engine RPM to a microSD card for later analysis.

I developed this project to combine my interests in automotive technology, embedded systems, software development, and data analysis. I modified and optimized the Freematics reference code, resolved hardware and storage issues, restructured the logging logic, and validated the system through road testing across multiple vehicles.

This repository documents the complete project lifecycle: the [`src`](src/) directory contains the working logger code and required libraries, [`data`](data/) contains raw CSV recordings from real vehicle tests, [`graphs`](graphs/) contains annotated speed and RPM visualizations, [`images`](images/) shows the test vehicles and logger hardware during development and operation, and [`docs`](docs/) contains the detailed project notebook, wiring references, troubleshooting process, and conclusions.

## Project Highlights

- Reads real-time vehicle data through the OBD-II port
- Displays live speed, engine RPM, throttle position, and temperature data when supported by the vehicle
- Records speed and engine RPM to CSV files on a microSD card
- Operates from power supplied through the vehicle's OBD-II port
- Supports post-drive visualization and comparison in Microsoft Excel
- Was tested across vehicles from Chevrolet, Subaru, Lexus, Toyota, and Honda
- Includes documented wiring, test results, raw data, graphs, and development notes

## System Overview

The vehicle's OBD-II port supplies power and diagnostic data to the Freematics UART adapter. The Arduino Nano processes the incoming data, presents live information on the OLED display, and writes selected parameters to the SD card as CSV data. The recorded files can then be transferred to a computer and analyzed as time-series graphs.

```text
Vehicle OBD-II Port
        |
Freematics UART Adapter
        |
    Arduino Nano
      /       \
OLED Display   microSD Card
                    |
               CSV Data
                    |
             Graphs and Analysis
```

## Original Goal and Planned Expansion

In addition to building a functional data logger, I designed the project as a foundation for measuring the effects of modifications to my own car. The primary project and development vehicle was my modified **1999 Lexus GS 300**. By recording the same vehicle parameters before and after a modification, I could compare changes in speed, engine RPM, acceleration, and other supported data rather than relying only on subjective driving impressions.

I also planned to integrate the Freematics Nanotimer functionality. This would record acceleration tests such as 0-60 mph runs and preserve the results for direct before-and-after comparisons following vehicle modifications.

The longer-term hardware concept was to integrate the logger into the car's dashboard as a permanent, visible information display. My initial design would replace the right-side air vent above the center infotainment system with a custom 3D-printed enclosure containing the OLED screen and a front-facing SD card reader. This placement would keep live data visible while allowing the SD card to be removed without disassembling the dashboard.

## Hardware

- Arduino Nano
- Freematics OBD-II UART Adapter
- 128 x 64 OLED display
- 5 V-compatible microSD card module
- microSD card
- Jumper wires and breadboard
- OBD-II-compatible vehicle

## Software and Technologies

- Arduino / Embedded C++
- OBD-II vehicle diagnostics
- UART serial communication
- SPI communication
- I2C communication
- CSV data logging
- Microsoft Excel
- Data visualization and comparative analysis

## Engineering and Development Work

The available reference sketch could display live information, but its SD card logging feature was initially inactive. After enabling data logging, the program exceeded the Arduino Nano's available storage capacity.

To produce a working system, I:

- Diagnosed the disabled logging configuration in `config.h`
- Shortened nonessential OLED status messages
- Removed unused display and binary-formatting functionality
- Reduced the compiled program size from **30,820 bytes to 30,526 bytes**, bringing it below the **30,720-byte** limit
- Examined and documented the overloaded `logData` methods
- Consolidated the required logging behavior into a method focused on speed and engine RPM
- Troubleshot UART wiring, SD card compatibility, and vehicle communication
- Compared behavior across different vehicle manufacturers and model years
- Verified recorded maximum speeds against values observed during test drives

These changes preserved the core display functionality while making reliable CSV data logging possible within the Arduino Nano's memory constraints.

## Testing

![Data logger operating during vehicle testing](images/obd2-logger-live-test.png)

The logger was tested with several vehicles to evaluate communication reliability and data quality. Successful data collection was completed with a Chevrolet Impala, Subaru Outback, and Lexus GS 300. Additional compatibility testing was performed with Toyota, Lexus, Chevrolet, Subaru, and Honda vehicles.

Results varied between vehicles, showing the importance of testing embedded automotive systems across different manufacturers, model years, and implementations of the OBD-II standard.

![Vehicle used for road testing](images/project-test-vehicle-1.png)

## Results

Four primary driving data sets were analyzed. Across these tests:

- Average vehicle speed ranged from approximately **20 to 32 mph**
- Average engine speed ranged from approximately **1,200 to 1,700 RPM**
- The highest recorded engine-speed peak was **3,821 RPM**
- Recorded maximum speeds matched the values observed during the corresponding test drives
- The graphs showed the expected relationship between vehicle speed and engine RPM

The tests were observational rather than laboratory-controlled. Traffic, route length, idle duration, vehicle response rate, and driving conditions varied between runs.

### Example Data Visualization

![Annotated speed and RPM graph](graphs/graph-1-annotated.jpg)

Additional graphs and their associated data are available in the [`graphs`](graphs/) and [`data`](data/) directories.

## Repository Structure

```text
.
├── src/
│   ├── logger/       Nanologger sketch and configuration files
│   └── libraries/    Required Arduino libraries
├── data/      Recorded CSV data from vehicle tests
├── graphs/    Annotated speed and RPM visualizations
├── images/    Hardware, vehicle, and testing photos
├── docs/      Project notebook and supporting documentation
└── README.md  Project overview
```

## Wiring Connections

The following worded schematic is based on the final working wiring configuration documented during the project.

### Freematics OBD-II UART Adapter

The adapter wires were identified by color because the individual wires were not labeled.

| Adapter wire | Arduino Nano connection |
|---|---|
| White | TX1 / D1 |
| Green | RX0 / D0 |
| Red | 5V |
| Black | GND |

If the live-data screen initializes and begins displaying vehicle information, the UART wiring is communicating correctly. If the logger remains on its welcome screen, verify the white and green serial connections.

### OLED Display

| OLED connection | Arduino Nano connection |
|---|---|
| VCC | 3.3V |
| GND | GND |
| SCL | A5 |
| SDA | A4 |

At startup, the OLED displays a welcome screen and reports the connection status of the OBD-II adapter, accelerometer, and SD card. After initialization, it displays live vehicle data.

### microSD Card Module

| SD module connection | Arduino Nano connection |
|---|---|
| CS | D10 |
| SCK | ICSP SCK |
| MOSI | ICSP MOSI |
| MISO | ICSP MISO |
| VCC | ICSP 5V |
| GND | ICSP GND |

The microSD module uses the Arduino Nano's ICSP/SPI interface. Match the connections by their signal labels—SCK, MOSI, MISO, 5V, and GND—and use the ICSP orientation diagram in the [`project notebook`](docs/project-notebook.pdf) to confirm the header position. The triangle or circular orientation marker shown in the documentation should align with the Nano before power is connected.

## Running the Project

> **Safety:** Assemble and secure the hardware before driving. Do not operate or adjust the logger while the vehicle is moving. Loose wiring can disconnect during turns or acceleration.

Download or clone the **entire `src` directory**. The project depends on both `src/logger` and `src/libraries`; copying only the `.ino` file will not provide all of the files required to compile the logger.

1. Install the Arduino IDE.
2. Add the included `src/libraries` directory to the Arduino environment, or copy its contents into the Arduino libraries directory.
3. Connect the OBD-II adapter, OLED display, and SD card module according to the wiring documentation in [`docs`](docs/).
4. Open `src/logger/nanologger.ino` in the Arduino IDE.
5. Select the correct Arduino Nano board and processor settings.
6. Compile and upload the sketch.
7. Connect the adapter to an OBD-II-compatible vehicle manufactured in 1996 or later.
8. Start the vehicle and confirm that the display reports successful OBD-II and SD card initialization.
9. After testing, remove the microSD card and open the generated CSV file on a computer for analysis.

## Documentation

The [`docs`](docs/) directory contains the complete development notebook, including:

- Initial research and planning
- Wiring details and diagrams
- Troubleshooting notes
- Program-memory optimization
- Vehicle compatibility observations
- Test results and graphs
- Conclusions and proposed improvements

## Future Improvements

- Store both km/h and mph consistently in the output data
- Record additional parameters such as engine temperature, distance traveled, and selected gear when supported
- Add stronger protection against interrupted or incomplete data writes
- Integrate the Nanotimer functionality to record acceleration tests and compare performance before and after vehicle modifications
- Build a compact 3D-printed dashboard enclosure with an integrated display and front-facing SD card reader
- Evaluate replacing the right-side air vent above the center infotainment system with the custom enclosure
- Secure the hardware for repeatable in-vehicle testing
- Conduct controlled tests using the same route and driving conditions
- Automate CSV processing and graph generation

## What This Project Demonstrates

This project demonstrates practical experience with:

- Embedded software development
- Automotive systems and OBD-II diagnostics
- Hardware integration and wiring
- Debugging under memory and storage constraints
- Serial and peripheral communication
- Iterative testing across real-world systems
- Technical documentation
- Data collection, visualization, and interpretation

## Acknowledgments

This project uses and modifies reference code and libraries provided by [Freematics](https://freematics.com/). The implementation, configuration, optimization, testing, documentation, and data analysis in this repository reflect my project work. Any third-party source files retain their original ownership and licensing requirements.

## Author

**Hadeed Fawad**  
Computer Science graduate interested in software engineering, embedded systems, automotive technology, data analysis, and technical problem-solving.
