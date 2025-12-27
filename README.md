# ðŸ›’ IoT Smart Vending Machine

[![Hardware](https://img.shields.io/badge/Hardware-ESP32-blue)](https://www.espressif.com/en/products/socs/esp32)
[![PCB](https://img.shields.io/badge/PCB-Altium-orange)](https://www.altium.com/)
[![Database](https://img.shields.io/badge/Database-SQLite-green)](https://www.sqlite.org/)
[![License](https://img.shields.io/badge/License-Academic-yellow)](LICENSE)

A complete IoT vending machine system with **custom PCB design**, **ESP32 microcontroller**, **web interface**, and **SQLite database** for the ETRO laboratory snack bar.

---

## ðŸ“‹ Project Overview

This project modernizes the ETRO laboratory snack bar by replacing the traditional paper-based ordering system with a smart vending machine. Users authenticate via keypad, select snacks from an e-paper display, and track orders through a web interface.

### âœ¨ Key Features

- âœ… **Custom PCB Design** - Double-layer PCB with ESP32, MCP23008 I/O expander
- âœ… **Low-Power E-Paper Display** - 4.37" 4-color Waveshare screen
- âœ… **Web Interface** - 9 responsive HTML pages
- âœ… **Secure Database** - SQLite with SHA-256 password hashing
- âœ… **WiFi Connectivity** - ESP32 web server at `http://snackbaresp32/home`
- âœ… **Multi-User Support** - User authentication and order tracking

---

## ðŸ—ï¸ System Architecture

### Hardware Components

| Component | Model | Function |
|-----------|-------|----------|
| **Microcontroller** | DFRobot FireBeetle ESP32 | WiFi, web server, 16MB flash |
| **Display** | Waveshare 4.37" E-Paper (4-color) | Low-power menu display |
| **Input** | 4x4 Keypad Matrix | User authentication & selection |
| **I/O Expander** | MCP23008 (8-bit) | I2C keypad communication |

### Circuit Schematic

![Circuit Schematic](docs/diagrams/PCBSchematic22.jpg)
*Complete circuit schematic showing ESP32, I/O expander, keypad matrix, and e-paper connections designed in Altium Designer.*

![Detailed Schematic](docs/diagrams/PCBSchematic33.jpg)
*Enlarged view of component interconnections with net labels.*

### PCB Layout

![PCB Layout 2D](docs/diagrams/PCB2D.jpg)
*Double-layer PCB layout with optimized trace routing and component placement.*

![PCB 3D View](docs/diagrams/PCB44.jpg)
*3D visualization of the assembled PCB.*

---

## ðŸ’» Software Architecture

### System Components

1. **Hardware Management** (C/C++)
   - Keypad input via I2C interrupts
   - E-paper display via SPI (14s update time)
   - LED status indicator

2. **Web Server** (HTTP)
   - 9 HTML pages in flash memory
   - GET/POST request handling
   - Session management with tokens

3. **Database** (SQLite)
   - 4 tables: Customer, Product, Orders, Product_Order
   - SHA-256 password hashing
   - Relational data integrity

### System Workflow

![Order Process](docs/images/Command.png)
*Complete order flow: Menu â†’ Authentication â†’ Selection â†’ Confirmation â†’ Database Update*

![System Integration](docs/images/SchemaGlobal.jpg)
*System-wide data flow between hardware, web server, and database.*

---

## ðŸŒ Web Interface

The system provides **9 comprehensive web pages** for complete functionality:

### User Pages

| Page | Screenshot | Description |
|------|------------|-------------|
| **Home** | ![Home](docs/images/Home.png) | Landing page with system introduction |
| **Login** | ![Login](docs/images/Login.png) | User authentication (ID + PIN) |
| **Signup** | ![Signup](docs/images/Sign%20in.png) | New user registration |
| **Welcome** | ![Welcome](docs/images/welcome.png) | Registration confirmation with unique ID |
| **User Dashboard** | ![User](docs/images/Userpage.png) | Order history & payment status |
| **Order Details** | ![Details](docs/images/Order%20Details.png) | Detailed order breakdown |

### Admin Pages

| Page | Screenshot | Description |
|------|------------|-------------|
| **Snacks Stock** | ![Stock](docs/images/Snacks.png) | Inventory management |
| **Add Snack** | ![Add](docs/images/Add%20snack.png) | Add new products to database |
| **Admin Dashboard** | ![Admin](docs/images/AdminPage2.png) | Monthly orders overview |

---

## ðŸ—„ï¸ Database Schema
```
â”Œâ”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”      â”Œâ”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”
â”‚  Customer       â”‚      â”‚  Product         â”‚
â”œâ”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”¤      â”œâ”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”¤
â”‚ Client_ID (PK)  â”‚      â”‚ Product_ID (PK)  â”‚
â”‚ Pin_Code (Hash) â”‚      â”‚ Name             â”‚
â”‚ Email_Address   â”‚      â”‚ Unit_Price       â”‚
â”‚ Name, Surname   â”‚      â”‚ Stock            â”‚
â””â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”˜      â””â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”˜
         â”‚                        â”‚
         â”‚                        â”‚
         â–¼                        â–¼
â”Œâ”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”      â”Œâ”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”
â”‚  Orders         â”‚â—„â”€â”€â”€â”€â–ºâ”‚ Product_Order    â”‚
â”œâ”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”¤      â”œâ”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”¤
â”‚ Order_ID (PK)   â”‚      â”‚ Order_ID (FK)    â”‚
â”‚ Date            â”‚      â”‚ Product_ID (FK)  â”‚
â”‚ Status          â”‚      â”‚ Quantity         â”‚
â”‚ Client_ID (FK)  â”‚      â””â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”˜
â””â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”˜
```

### Security Features

- âœ… **SHA-256 Hashing** - PIN codes never stored in plaintext
- âœ… **Session Tokens** - Random tokens prevent session hijacking
- âœ… **Input Validation** - SQL injection prevention

---

## ðŸ“‚ Project Structure
```
IOT-Project-2023-2024/
â”œâ”€â”€ src/                   # Embedded C/C++ code
â”‚   â”œâ”€â”€ main.cpp          # Main program
â”‚   â”œâ”€â”€ hardware.cpp      # Keypad & E-paper drivers
â”‚   â”œâ”€â”€ webserver.cpp     # HTTP request handlers
â”‚   â””â”€â”€ database.cpp      # SQLite queries
â”‚
â”œâ”€â”€ web/                   # HTML pages (9 total)
â”‚   â”œâ”€â”€ home.html
â”‚   â”œâ”€â”€ login.html
â”‚   â”œâ”€â”€ user.html
â”‚   â””â”€â”€ ...
â”‚
â”œâ”€â”€ hardware/              # PCB design files
â”‚   â”œâ”€â”€ altium/           # Altium Designer project
â”‚   â”œâ”€â”€ schematic.pdf
â”‚   â””â”€â”€ gerber/           # Manufacturing files
â”‚
â”œâ”€â”€ docs/                  # Documentation
â”‚   â”œâ”€â”€ diagrams/         # Hardware schematics
â”‚   â”œâ”€â”€ images/           # Web interface screenshots
â”‚   â””â”€â”€ Project_Report.pdf
â”‚
â””â”€â”€ database/
    â””â”€â”€ schema.sql        # Database structure
```

---

## ðŸš€ Getting Started

### Hardware Setup

1. **Assemble PCB**
   - Solder ESP32, MCP23008, capacitors
   - Connect keypad to I/O Header 1
   - Connect e-paper to I/O Header 3

2. **Power Configuration**
   - USB or external lithium battery
   - Integrated battery management circuit

### Software Installation
```bash
# 1. Clone repository
git clone https://github.com/GlodiSala/IOT-Project-2023-2024.git
cd IOT-Project-2023-2024

# 2. Install dependencies (PlatformIO)
pio lib install

# 3. Configure WiFi (edit config.h)
#define WIFI_SSID "your_network"
#define WIFI_PASSWORD "your_password"

# 4. Upload to ESP32
pio run --target upload
```

### Access Web Interface

1. Connect to WiFi network configured in `config.h`
2. Navigate to: **http://snackbaresp32/home**
3. Default admin credentials in documentation

---

## ðŸ› ï¸ Technical Specifications

### Hardware

- **Microcontroller:** ESP32 (240MHz, 16MB Flash)
- **Communication:** I2C (keypad), SPI (display), WiFi
- **Display:** 4.37" E-Paper, 512Ã—368 pixels, 4-color
- **Input:** 4Ã—4 matrix keypad (16 keys)
- **Power:** USB 5V or Li-Ion battery (3.7V)

### Software

- **Language:** C/C++ (Arduino framework)
- **Web Server:** ESP32 AsyncWebServer
- **Database:** SQLite3 (embedded)
- **Security:** SHA-256 cryptographic hashing
- **Memory Usage:** 99% program memory, files in flash

---

## ðŸ“Š Performance & Challenges

### Achievements

âœ… Fully functional vending machine  
âœ… Secure user authentication  
âœ… Real-time inventory management  
âœ… Responsive web interface  
âœ… NIST-compliant encryption  

### Challenges Solved

âš ï¸ **E-Paper Update Speed (14s)** â†’ Added LED status indicator  
âš ï¸ **Memory Limitations (99% usage)** â†’ Stored HTML/DB in flash  
âš ï¸ **Component Integration** â†’ Custom PCB design  

### Future Improvements

- ðŸ’¡ Dual ESP32 setup (one for web, one for hardware)
- ðŸ’¡ Faster e-paper display (4.2" with 5s update)
- ðŸ’¡ Bluetooth communication between controllers

---

## ðŸ‘¥ Authors

**Miranda Ndayisaba**  
ðŸ“§ miranda.ndayisaba@ulb.be  
ðŸ« ULB-VUB Brussels

**Glodi Sala Mangituka**  
ðŸ“§ glodi.sala.mangituka@ulb.be  
ðŸ”— [LinkedIn](https://linkedin.com/in/glodi-sala-mangituka)  
ðŸ™ [GitHub](https://github.com/GlodiSala)

---

## ðŸŽ“ Academic Context

**Course:** PROJ-H415 - Project Electronics and Telecommunication  
**Institution:** UniversitÃ© Libre de Bruxelles (ULB) / Vrije Universiteit Brussel (VUB)  
**Academic Year:** 2023-2024

### Supervisor

**Ir. Jonathan Vrijsen**  
Project Supervisor, ETRO Laboratory

---

## ðŸ“œ License

This project is for **academic purposes only**.  
All rights reserved to the authors and ULB-VUB Brussels.

---

## ðŸ™ Acknowledgments

Special thanks to:
- **ETRO Laboratory** researchers for continuous support
- **Ir. Jonathan Vrijsen** for guidance and supervision
- **ULB-VUB** teaching staff
- Component manufacturers: DFRobot, Waveshare, Microchip

---

## ðŸ“š Documentation & Resources

- ðŸ“„ **[Full Project Report](docs/Project_Report.pdf)** - Complete technical documentation
- ðŸ”§ **[Hardware Schematics](docs/diagrams/)** - PCB design files
- ðŸ’» **[Source Code](https://github.com/GlodiSala/IOT-Project-2023-2024)** - Complete codebase
- ðŸŒ **[Web Interface Screenshots](docs/images/)** - All 9 pages

### References

1. [ESP32 Technical Reference](https://www.espressif.com/en/products/socs/esp32)
2. [SQLite Documentation](https://www.sqlite.org/docs.html)
3. [Altium Designer](https://www.altium.com/documentation)
4. [Waveshare E-Paper](https://www.waveshare.com/wiki/4.37inch_e-Paper_Module_(G)_Manual)

---

â­ **If you find this project interesting, please give it a star!**

ðŸ“§ **Questions?** Open an issue or contact the authors.

ðŸš€ **Want to contribute?** Fork the repository and submit a pull request.
