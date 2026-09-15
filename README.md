# Automated Greenhouse Ventilation System (*Fitotoldos*)
**Target Region:** Arequipa Region, Andes Mountains, Peru  
**Target Application:** Microclimate stabilization and automated aeration for high-altitude *fitotoldos* (greenhouses).

---

## 📌 Project Overview
High-altitude farming in the Andes faces extreme diurnal temperature swings, intense UV radiation, and freezing night temperatures. This project automates microclimate ventilation using a solar-powered ESP32 controller driving 4-wire PWM fans and micro-servos. The system dynamically modulates intake/exhaust airflow and louver positions to prevent thermal stress and heat entrapment during high solar-irradiance hours.

---

## 🏗️ System Architecture & Hardware
* **Microcontroller:** ESP32 (Dual-core 240MHz, Wi-Fi/Bluetooth stack)
* **Power Subsystem:** Solar panel array, MPPT charging controller, deep-cycle battery pack
* **Actuation:** 
  * 4-Wire PWM Fans (Tachometer speed feedback & duty-cycle control)
  * Servo Actuators (Automated ventilation louver/dampers)
* **Telemetry & Sensing:** Temperature, relative humidity, and solar irradiance sensors

---

## 📅 Project Timeline & Roadmap
### System Responsibility Split
* **Subsystem A - Ian Knight (Climate Control & Actuation):** ESP32 control logic, sensor acquisition, 4-wire PWM fan control, servo louvers.
* **Subsystem B - Kiera Morrin (Power & Energy Management):** Solar array sizing, MPPT charge controller interface, battery telemetry, power rails.

### Project Milestones & Integration Timeline

| Phase / Target Date | Subsystem A: Actuation & MCU | Subsystem B: Power Subsystem | System Integration & Field Testing |
| :--- | :--- | :--- | :--- |
| **Week 4: Designing Subsystems** | • Implement PWM driver for 4-wire fans<br>• Connect to INA219 current sensors using I2C<br>• Monitor Temperature and Humidity using DHT sensors| • Add a solar array to power model<br> | **Gateway 1:** N/A this week. |
| **Week 5:** | • design a fan servo sweeping mode<br>• communicate from one node to another using an MAX-485 transceiver<br> |• submit bill of materials to EPICs for the components that you are using in the off grid power schematic<br> • check out necessary buck converters from the ECE shop to implement LTspice<br>• Add low-voltage cutoff/protection circuit<br>• Log solar panel charging currents | **Gateway 2:** ESP32 autonomously adjusts ventilation based on sensor input using solar power. |
| **Phase 3: Telemetry & Mobile UI** | • Set up ESP32 SoftAP web server<br>• Build mobile-responsive status dashboard<br>• Expose battery & climate metrics via API | • Integrate power telemetry into shared data bus (I2C/SPI)<br>• Validate multi-day solar autonomy | **Gateway 3:** Full system enclosure integration; phone connects locally to read climate & power stats. |
| **Phase 4: Andean Field Deployment** | • Hardcode fail-safe vent positions<br>• Conduct environmental thermal testing | • Validate outdoor thermal enclosure for battery pack | **Gateway 4:** Installation and field testing in Arequipa *fitotoldo*. |

## 🛠️ Software & Build Setup

### Prerequisites
* [ESP-IDF v6.1](https://docs.espressif.com/projects/esp-idf/en/latest/esp32/)
* Git

### Building & Flashing

1. **Clone the repository:**
   ```bash
   git clone [https://github.com/IANJKNIGHT/senior_design_automated_ventilation.git](https://github.com/IANJKNIGHT/senior_design_automated_ventilation.git)
   cd senior_design_automated_ventilation
