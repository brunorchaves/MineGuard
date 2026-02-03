# MineGuard

**Real-time Fleet Monitoring & Collision Avoidance System for Mining Operations**

[![C++](https://img.shields.io/badge/C++-17-blue.svg)](https://isocpp.org/)
[![.NET](https://img.shields.io/badge/.NET-8.0-purple.svg)](https://dotnet.microsoft.com/)
[![License](https://img.shields.io/badge/License-MIT-green.svg)](LICENSE)

---

## Overview

MineGuard is a full-stack fleet management and safety system designed for open-pit mining operations. It simulates the core functionality of industrial collision avoidance systems (CAS) and fleet management systems (FMS), providing real-time vehicle tracking, collision prediction, and operational analytics.

The system demonstrates how modern mining operations protect workers and equipment through technology: GNSS-based positioning, path prediction algorithms, and intelligent alerting systems that help achieve **zero-harm environments**.

<p align="center">
  <img src="docs/images/dashboard-preview.png" alt="MineGuard Dashboard" width="800">
</p>

---

## Key Features

### 🚛 Vehicle Simulation
- Realistic simulation of mining fleet (haul trucks, excavators, light vehicles)
- GNSS-based positioning with configurable update rates
- Production cycle modeling (load → haul → dump → return)
- Physics-based movement with acceleration, fuel consumption, and payload effects

### 🛡️ Collision Avoidance
- **Path Prediction Algorithm**: Projects vehicle trajectories to detect potential collisions
- **Time-to-Impact (TTI)**: Calculates seconds until potential collision
- **Priority-based Alerts**: LOW → MEDIUM → HIGH → CRITICAL based on TTI and vehicle types
- **Alert Classification**: Approach, crossing, tailgating, blind spot scenarios

### 📊 Fleet Management
- Real-time fleet status and positioning
- Telemetry ingestion at 1 Hz per vehicle
- REST API for integration with external systems
- Live dashboard with interactive map

### 📈 Analytics
- Collision alert history and statistics
- Fleet utilization metrics
- Safety heatmap showing high-risk areas

---

## Architecture

```
┌──────────────────────────────────────────────────────────────────────────┐
│                            MINE SITE                                      │
│                                                                          │
│   ┌─────────┐  ┌─────────┐  ┌─────────┐  ┌─────────┐  ┌─────────┐      │
│   │ HT-101  │  │ HT-102  │  │ HT-103  │  │ EX-101  │  │ LV-101  │      │
│   │ Haul    │  │ Haul    │  │ Haul    │  │ Excav.  │  │ Light   │      │
│   │ Truck   │  │ Truck   │  │ Truck   │  │         │  │ Vehicle │      │
│   └────┬────┘  └────┬────┘  └────┬────┘  └────┬────┘  └────┬────┘      │
│        │            │            │            │            │            │
│        └────────────┴─────┬──────┴────────────┴────────────┘            │
│                           │                                              │
│                    ┌──────┴──────┐                                       │
│                    │  Simulator  │  C++ / Linux                          │
│                    │  (Onboard)  │  • GNSS positioning                   │
│                    │             │  • Collision detection                │
│                    │             │  • Telemetry generation               │
│                    └──────┬──────┘                                       │
│                           │                                              │
└───────────────────────────┼──────────────────────────────────────────────┘
                            │
                            │ TCP/JSON (Port 5000)
                            ▼
┌───────────────────────────────────────────────────────────────────────────┐
│                         CONTROL CENTER                                     │
│                                                                           │
│  ┌─────────────────────────────────────────────────────────────────────┐ │
│  │                      Backend API (.NET 8)                            │ │
│  │                                                                      │ │
│  │  ┌──────────────┐  ┌──────────────┐  ┌──────────────┐              │ │
│  │  │  Telemetry   │  │  Collision   │  │    REST      │              │ │
│  │  │  Receiver    │  │  Processor   │  │    API       │              │ │
│  │  │              │  │              │  │              │              │ │
│  │  │ • TCP Listen │  │ • Alert      │  │ • /vehicles  │              │ │
│  │  │ • Parse JSON │  │   detection  │  │ • /alerts    │              │ │
│  │  │ • Validate   │  │ • Priority   │  │ • /status    │              │ │
│  │  └──────────────┘  └──────────────┘  └──────────────┘              │ │
│  │                                                                      │ │
│  └──────────────────────────────────────┬──────────────────────────────┘ │
│                                         │                                 │
│                                         │ HTTP/REST                       │
│                                         ▼                                 │
│  ┌─────────────────────────────────────────────────────────────────────┐ │
│  │                     Dashboard (HTML/JS)                              │ │
│  │                                                                      │ │
│  │  ┌──────────────┐  ┌──────────────┐  ┌──────────────┐              │ │
│  │  │     Map      │  │    Fleet     │  │    Alert     │              │ │
│  │  │    View      │  │   Status     │  │   Center     │              │ │
│  │  │              │  │              │  │              │              │ │
│  │  │ • Leaflet    │  │ • Vehicle    │  │ • Active     │              │ │
│  │  │ • Real-time  │  │   cards      │  │   alerts     │              │ │
│  │  │   positions  │  │ • Telemetry  │  │ • History    │              │ │
│  │  └──────────────┘  └──────────────┘  └──────────────┘              │ │
│  │                                                                      │ │
│  └─────────────────────────────────────────────────────────────────────┘ │
│                                                                           │
└───────────────────────────────────────────────────────────────────────────┘
```

---

## Technology Stack

| Layer | Technology | Purpose |
|-------|------------|---------|
| **Simulator** | C++17, CMake, POSIX Sockets | Vehicle simulation, collision detection, TCP client |
| **Backend** | .NET 8, ASP.NET Core | Telemetry ingestion, alert processing, REST API |
| **Frontend** | HTML5, JavaScript, Leaflet.js | Real-time dashboard, interactive map |
| **Protocol** | TCP with length-prefixed JSON | Reliable telemetry transmission |

---

## Getting Started

### Prerequisites

- **Linux** (Ubuntu 22.04+ recommended)
- **GCC 11+** and **CMake 3.16+**
- **.NET 8 SDK**
- **Git**

### Quick Start

```bash
# Clone the repository
git clone https://github.com/yourusername/mineguard.git
cd mineguard

# Build and run simulator
cd simulator
mkdir build && cd build
cmake .. && make
./mineguard_sim --local  # Test mode (console output)

# In another terminal, start the backend
cd backend/MineGuard.Api
dotnet run

# Open dashboard
open http://localhost:5000
```

### Running the Full System

```bash
# Terminal 1: Backend
cd backend/MineGuard.Api && dotnet run

# Terminal 2: Simulator (connects to backend)
cd simulator/build && ./mineguard_sim --host localhost --port 5000

# Terminal 3: Open dashboard
xdg-open http://localhost:5000
```

---

## Project Structure

```
mineguard/
├── simulator/                 # C++ Vehicle Simulator
│   ├── include/
│   │   ├── telemetry.hpp     # Data structures
│   │   ├── vehicle.hpp       # Vehicle class
│   │   ├── collision.hpp     # Collision detection
│   │   └── fleet.hpp         # Fleet manager
│   ├── src/
│   │   ├── vehicle.cpp
│   │   ├── collision.cpp
│   │   ├── fleet.cpp
│   │   └── main.cpp
│   └── CMakeLists.txt
│
├── backend/                   # .NET Backend
│   └── MineGuard.Api/
│       ├── Controllers/
│       ├── Services/
│       ├── Models/
│       └── Program.cs
│
├── dashboard/                 # Web Dashboard
│   ├── index.html
│   ├── css/
│   └── js/
│
├── docs/                      # Documentation
│   ├── architecture.md
│   └── api.md
│
└── README.md
```

---

## API Reference

### Vehicles

```http
GET /api/vehicles
```
Returns list of all vehicles with current status and position.

```http
GET /api/vehicles/{id}
```
Returns detailed information for a specific vehicle.

### Alerts

```http
GET /api/alerts
```
Returns active collision alerts.

```http
GET /api/alerts/history
```
Returns historical alerts with filtering options.

### System

```http
GET /api/status
```
Returns system health and statistics.

---

## Collision Detection Algorithm

MineGuard implements a **path prediction** approach to collision avoidance:

1. **Position Projection**: Each vehicle's future position is calculated based on current velocity and heading
2. **Closest Point of Approach (CPA)**: The algorithm finds the minimum distance between projected paths
3. **Time to CPA (TCPA)**: Calculates when vehicles will reach their closest point
4. **Alert Generation**: If TCPA < threshold AND distance at CPA < safety radius, an alert is triggered

```cpp
// Simplified collision check
TimeToImpact calculateTTI(Vehicle& v1, Vehicle& v2) {
    // Project positions forward in time
    for (double t = 0; t < MAX_PREDICTION_TIME; t += 0.1) {
        auto pos1 = v1.predictPosition(t);
        auto pos2 = v2.predictPosition(t);
        
        double distance = calculateDistance(pos1, pos2);
        
        if (distance < SAFETY_RADIUS) {
            return TimeToImpact{t, distance, calculatePriority(t)};
        }
    }
    return TimeToImpact::None();
}
```

### Alert Priority Levels

| Priority | Time to Impact | Action |
|----------|---------------|--------|
| **CRITICAL** | < 3 seconds | Immediate intervention required |
| **HIGH** | 3-5 seconds | Operator must take action |
| **MEDIUM** | 5-10 seconds | Heightened awareness |
| **LOW** | 10-15 seconds | Advisory notification |

---

## Inspired By

This project is inspired by industrial mining safety systems:

- **Hexagon Mining CAS** - Collision Avoidance System
- **Hexagon OP Pro** - Fleet Management System
- **EMESRT** - Earth Moving Equipment Safety Round Table guidelines

---

## Use Cases

- **Learning**: Understand how industrial fleet management systems work
- **Portfolio**: Demonstrate full-stack development skills for mining/industrial tech roles
- **Prototyping**: Base for custom fleet monitoring solutions
- **Education**: Teaching real-time systems, TCP networking, and safety-critical software

---

## Roadmap

- [x] Basic vehicle simulation
- [x] TCP telemetry transmission
- [x] Collision detection algorithm
- [x] REST API
- [x] Real-time dashboard
- [ ] Database persistence (PostgreSQL)
- [ ] Incident replay feature
- [ ] Docker containerization
- [ ] Production cycle analytics

---

## Contributing

Contributions are welcome! Please read the [contributing guidelines](CONTRIBUTING.md) first.

---

## License

This project is licensed under the MIT License - see the [LICENSE](LICENSE) file for details.

---

## Author

**Bruno**  
Electrical Engineering @ UFMG  
Embedded Systems Developer

---

<p align="center">
  <i>"The most important thing to come out of any mine is the miner."</i>
</p>
