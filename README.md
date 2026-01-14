# IoT Smart Vending Machine

[![Hardware](https://img.shields.io/badge/Hardware-ESP32-blue)](https://www.espressif.com/en/products/socs/esp32)
[![PCB](https://img.shields.io/badge/PCB-Altium-orange)](https://www.altium.com/)
[![Database](https://img.shields.io/badge/Database-SQLite-green)](https://www.sqlite.org/)
[![License](https://img.shields.io/badge/License-Academic-yellow)](LICENSE)

A complete IoT vending machine system with **custom PCB design**, **ESP32 microcontroller**, **web interface**, and **SQLite database** for the ETRO laboratory snack bar.

---

## 📋 Project Overview

This project modernizes the ETRO laboratory snack bar by replacing the traditional paper-based ordering system with a smart vending machine. Users authenticate via keypad, select snacks from an e-paper display, and track orders through a web interface.

### Key Features

- ✅ **Custom PCB Design** - Double-layer PCB with ESP32, MCP23008 I/O expander
- ✅ **Low-Power E-Paper Display** - 4.37" 4-color Waveshare screen
- ✅ **Web Interface** - 9 responsive HTML pages
- ✅ **Secure Database** - SQLite with SHA-256 password hashing
- ✅ **WiFi Connectivity** - ESP32 web server at http://snackbaresp32/home
- ✅ **Multi-User Support** - User authentication and order tracking

---

## ⚡ Technical Metrics

### System Performance

| Metric | Value |
|--------|-------|
| **Microcontroller** | ESP32 (240 MHz dual-core, 16MB flash) |
| **Program Memory Usage** | 99% (optimized for concurrent operations) |
| **Flash Storage** | 16MB (HTML pages + SQLite database) |
| **Communication Protocols** | I2C @ 100kHz, SPI @ 4MHz, WiFi 802.11 b/g/n |
| **Web Pages** | 9 responsive HTML pages (~5KB total) |
| **Database Tables** | 4 relational tables with foreign keys |
| **Response Time** | <100ms (keypad), 14s (display update) |
| **Concurrent Operations** | 4 simultaneous (I2C, SPI, HTTP, SQL) |

### Protocol Implementation

| Protocol | Application | Performance Optimization |
|----------|-------------|--------------------------|
| **I2C** | Keypad via MCP23008 I/O expander | Reduced GPIO from 8 to 2 pins (75% reduction) |
| **SPI** | 4.37" E-paper display control | Low-power: 0mW idle, updates only |
| **HTTP** | Async web server (9 pages) | Non-blocking request handling |
| **SQL** | Embedded SQLite queries | SHA-256 hashing, injection prevention |

### Resource Optimization

| Optimization | Impact | Implementation |
|--------------|--------|----------------|
| **GPIO Pin Reduction** | 75% savings | MCP23008 expands 2 I2C pins → 8 GPIO |
| **Memory Management** | 60% program memory freed | HTML/DB stored in flash vs program memory |
| **Power Efficiency** | <10mA idle | E-paper retains image without power |
| **Interrupt Architecture** | <100ms response | Event-driven keypad handling |

**Key Achievements:**
- ✅ **99% program memory utilization** - Maximum efficiency in resource-constrained environment
- ✅ **75% GPIO reduction** - I2C multiplexing enabled compact PCB design
- ✅ **Zero idle power** for display - E-paper technology for battery operation
- ✅ **Concurrent multi-protocol** handling - I2C, SPI, WiFi, and SQL simultaneously

---

## 🏗️ System Architecture

### Hardware Components

| Component | Model | Function | Interface |
|-----------|-------|----------|-----------|
| **Microcontroller** | DFRobot FireBeetle ESP32 | 240MHz dual-core, WiFi, 16MB flash | Master controller |
| **Display** | Waveshare 4.37" E-Paper | 512×368px, 4-color, low-power | SPI (4-wire) |
| **Input** | 4×4 Keypad Matrix | 16-key user authentication | Via I2C expander |
| **I/O Expander** | MCP23008 (8-bit) | I2C address 0x20 | I2C (SCL/SDA) |

### Circuit Schematic

![Circuit Schematic](docs/diagrams/PCBSchematic22.jpg)

*Complete circuit schematic showing ESP32, I2C I/O expander (MCP23008), 4×4 keypad matrix, and SPI e-paper connections designed in Altium Designer.*

### PCB Layout

![PCB Layout](docs/diagrams/PCB2D.jpg)

*Double-layer PCB layout with optimized trace routing, decoupling capacitors for noise reduction, and component placement for I2C/SPI signal integrity.*

---

## 💻 Software Architecture

### System Workflow

![Order Process](docs/diagrams/Command.png)

*Complete order flow: Menu Display → User Authentication → Snack Selection → Order Confirmation → Database Update → Web Dashboard*

![System Integration](docs/diagrams/SchemaGlobal.jpg)

*System-wide data flow between hardware (keypad/display), embedded firmware, web server, and SQLite database.*

### Technology Stack

| Layer | Technology | Lines of Code / Size |
|-------|------------|----------------------|
| **Embedded Firmware** | C/C++ (Arduino framework) | ~2,000 lines |
| **Web Interface** | HTML/CSS | 9 pages, ~5KB |
| **Database** | SQLite3 (embedded) | 4 tables, relational schema |
| **Web Server** | ESP32 AsyncWebServer | Async HTTP request handling |
| **Communication** | I2C/SPI/WiFi | Hardware protocol implementation |

---

## 🌐 Web Interface

The system provides **9 comprehensive web pages**:

| Page | Screenshot | Description |
|------|------------|-------------|
| **Home** | ![Home](docs/images/Home.png) | Landing page with system introduction |
| **Login** | ![Login](docs/images/Login.png) | User authentication (ID + SHA-256 hashed PIN) |
| **User Dashboard** | ![User](docs/images/Userpage.png) | Order history & payment tracking |
| **Admin Dashboard** | ![Admin](docs/images/AdminPage2.png) | Monthly orders overview & analytics |
| **Snack Inventory** | ![Snacks](docs/images/Snacks.png) | Real-time stock management |

---

## 🗄️ Database Schema

The database consists of **4 interconnected tables**:
```
Customer (Client_ID PK, Pin_Code, Email, Name)
    ↓ 1:N
Orders (Order_ID PK, Date, Status, Client_ID FK)
    ↓ N:M
Product_Order (Order_ID FK, Product_ID FK, Quantity)
    ↑ N:M
Product (Product_ID PK, Name, Unit_Price, Stock)
```

### Security Features

- ✅ **SHA-256 password hashing** - Secure credential storage
- ✅ **Session token management** - Random tokens prevent session hijacking
- ✅ **SQL injection prevention** - Parameterized queries
- ✅ **Input validation** - Client-side and server-side checks

---

## 📂 Project Structure
```
IOT-Project-2023-2024/
├── src/                      # Embedded C/C++ code
│   └── Biometric_Project_V5.ino  # Main firmware (~2000 lines)
│
├── web/                      # HTML pages (9 total)
│   ├── home.html            # Landing page
│   ├── login.html           # Authentication
│   ├── userpage.html        # User dashboard
│   ├── adminpage.html       # Admin dashboard
│   └── ...                  # Additional pages
│
├── database/                 # SQLite database
│   ├── snacks.db            # Production database
│   └── snacks.sqbpro        # Database project file
│
└── docs/                     # Documentation
    ├── diagrams/            # Hardware schematics & PCB
    │   ├── PCBSchematic22.jpg
    │   ├── PCB2D.jpg
    │   ├── Command.png
    │   └── SchemaGlobal.jpg
    └── images/              # Web interface screenshots
```

---

## 🚀 Getting Started

### Hardware Setup

1. **Assemble PCB components:**
   - ESP32 FireBeetle microcontroller
   - MCP23008 I2C I/O expander
   - Decoupling capacitors (noise reduction)
   - Pin headers for keypad and e-paper

2. **Connect peripherals:**
   - Keypad matrix → I/O Header 1 (via MCP23008)
   - E-paper display → I/O Header 3 (SPI interface)

3. **Power options:**
   - USB 5V (development)
   - Li-Ion battery 3.7V (portable operation)

### Software Installation
```bash
# 1. Clone repository
git clone https://github.com/GlodiSala/IOT-Project-2023-2024.git
cd IOT-Project-2023-2024

# 2. Install dependencies (Arduino IDE)
# - ESP32 board support
# - SQLite3 library
# - AsyncWebServer library
# - Wire library (I2C)
# - SPI library

# 3. Configure WiFi credentials
# Edit src/Biometric_Project_V5.ino:
#define WIFI_SSID "your_network"
#define WIFI_PASSWORD "your_password"

# 4. Upload to ESP32
# Arduino IDE: Tools → Board → ESP32 Dev Module
#              Tools → Upload Speed → 921600
#              Sketch → Upload
```

### Access Web Interface

1. **Connect to WiFi network** configured in firmware
2. **Navigate to:** http://snackbaresp32/home
3. **Login credentials:** Stored in SQLite database (Customer table)

---

## 🔧 Technical Specifications

### Hardware

| Specification | Value |
|--------------|-------|
| **Microcontroller** | ESP32 (240MHz dual-core, 520KB SRAM, 16MB flash) |
| **Communication** | I2C (100kHz), SPI (4MHz), WiFi 802.11 b/g/n |
| **Display** | 4.37" E-Paper, 512×368 pixels, 4-color (B/W/R/Y) |
| **Input** | 4×4 matrix keypad (16 keys) |
| **Power** | USB 5V or Li-Ion 3.7V (3.3V regulated) |
| **PCB** | 2-layer, FR-4 substrate |
| **Dimensions** | Custom PCB designed for compact enclosure |

### Software

| Component | Details |
|-----------|---------|
| **Language** | C/C++ (Arduino framework) |
| **Database** | SQLite3 (embedded, file-based) |
| **Web Server** | ESP32 AsyncWebServer (non-blocking) |
| **Security** | SHA-256 hashing, session tokens |
| **Memory** | 99% program, HTML/DB in 16MB flash |
| **Update Rate** | Keypad: <100ms, Display: 14s |

---

## 🛠️ Challenges & Engineering Solutions

| Challenge | Technical Constraint | Solution Implemented | Impact |
|-----------|---------------------|----------------------|--------|
| **E-Paper Update Speed** | 14-second refresh (4-color) | Added LED status indicator | Improved UX responsiveness |
| **Program Memory** | 99% utilization at peak | Moved HTML/DB to flash storage | Freed 60% program memory |
| **GPIO Limitations** | Limited ESP32 pins | MCP23008 I2C expander | 75% GPIO reduction |
| **Power Efficiency** | Battery operation required | E-paper (0mW idle) | Extended battery life |
| **Concurrent Operations** | I2C + SPI + WiFi + SQL | Interrupt-driven architecture | <100ms response time |

---

## 👥 Authors

**Miranda Ndayisaba**  
📧 miranda.ndayisaba@ulb.be  
🏫 ULB-VUB Brussels

**Glodi Sala Mangituka**  
📧 glodi.sala.mangituka@ulb.be  
🔗 [LinkedIn](https://www.linkedin.com/in/glodi-sala-mangituka-762616280/)  
🐙 [GitHub](https://github.com/GlodiSala)

---

## 🎓 Academic Context

**Course:** PROJ-H415 - Project Electronics and Telecommunication  
**Institution:** Université Libre de Bruxelles (ULB) / Vrije Universiteit Brussel (VUB)  
**Academic Year:** 2023-2024  
**Deployment:** ETRO Laboratory snack bar (production use)

**Supervisor:** Ir. Jonathan Vrijsen, ETRO Laboratory

---

## 📜 License

This project is for **academic purposes only**.  
All rights reserved to the authors and ULB-VUB Brussels.

---

## 📚 References

1. [ESP32 Technical Documentation](https://www.espressif.com/en/products/socs/esp32) - Espressif Systems
2. [SQLite3 Embedded Database](https://www.sqlite.org/docs.html) - SQLite Consortium
3. [Altium Designer PCB Software](https://www.altium.com/documentation) - Altium Limited
4. [Waveshare 4.37" E-Paper Module](https://www.waveshare.com/wiki/4.37inch_e-Paper_Module_(G)_Manual) - Waveshare Electronics
5. [MCP23008 I2C I/O Expander Datasheet](https://ww1.microchip.com/downloads/en/DeviceDoc/21919e.pdf) - Microchip Technology

---
