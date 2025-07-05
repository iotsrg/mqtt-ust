
# 🚨 MQTT Mayhem: Publish & Pwn  
_Exploiting and Securing MQTT in IoT Environments_

---

## 🧠 Overview

**Duration:** 2 Hours  
**Target Audience:** IoT Pentesters, Firmware Developers, Embedded Security Engineers  
**Workshop Goal:** Demonstrate practical MQTT misconfigurations, vulnerabilities, and defenses through real-world demos using ESP32 and MQTT broker setup.

---

## 🧰 Lab Setup

### Required Tools:
- MQTT Broker (e.g., Mosquitto)
- MQTT clients and visualizers (e.g., MQTT Explorer, MQTT Spy)
- MQTT pentest tools (e.g., EvilMQTT, Ioxy, MQTT-Fuzz)
- ESP32 development board (acting as a vulnerable MQTT client)

### Network Environment:
- Local broker accessible to the ESP32 and attack machine
- Basic UART or OTA-based logging on ESP32
- Optional rogue access point or DNS spoofing for redirection demos

---

## 🧱 Workshop Agenda

### Setup and Warm-Up
- Start the broker and connect it to the ESP32 client
- Demonstrate topic subscriptions and sensor/actuator emulation
- Confirm working communication flow between ESP32 and broker

---

### MQTT Protocol Deep Dive
- Cover the MQTT architecture: clients, broker, topics, sessions
- Explore the publish/subscribe model
- Explain QoS levels, retained messages, wildcard subscriptions
- Identify common areas where MQTT implementations go wrong

---

### Exploiting MQTT Vulnerabilities

#### Anonymous Access
- Demonstrate how brokers without authentication can be freely accessed by attackers

#### Retained Message Injection
- Show how attackers can persistently inject payloads that execute every time a client reconnects

#### Topic Flooding Attack
- Simulate excessive topic creation or publish actions to overload the broker or client

#### Rogue Broker Injection
- Redirect a client (e.g., ESP32) to a malicious broker that can spoof responses or inject commands

#### Client Behavior Hijack
- Publish malicious commands to subscribed topics to control or crash the target device

---

### MQTT Broker Hardening

#### Authentication and Authorization
- Enable password protection for clients
- Use access control lists (ACLs) to restrict publish/subscribe rights

#### Encryption
- Secure MQTT with TLS to prevent eavesdropping and MITM attacks

#### Retained Message Management
- Disable or strictly control retained message behavior for critical topics

#### Payload Validation
- Validate message formats and lengths on the client side (e.g., inside ESP32 firmware)

---

###  Wrap-Up and Takeaways

#### Key Lessons:
- MQTT is inherently insecure without proper configuration
- Retained messages and wildcard topics are common abuse vectors
- Encryption, authentication, and ACLs are must-haves in production
- Firmware on devices must enforce strict input validation

#### Resources:
- Link to cheatsheets, tools, GitHub lab setup, and slides
- Encourage testing with production-grade brokers and hardened configs
- Recommend continuous MQTT monitoring and anomaly detection

---

## 🔧 Tools Mentioned

- **Mosquitto**: Lightweight MQTT broker
- **MQTT Explorer**: GUI client for debugging
- **ralmqtt** - https://github.com/Red-Alert-Labs/ralmqtt/
- **EvilMQTT**: Rogue MQTT broker for exploitation
- **Ioxy**: MITM proxy for MQTT traffic inspection
- **Boofuzz / MQTT-Fuzz**: Used for protocol fuzzing and broker stress tests
- **ESP32**: IoT device to emulate real-world MQTT clients
- **IoT-MQTT-Healthcare-Simulator**: https://github.com/ErickJO16/IoT-MQTT-Healthcare-Simulator
- **ICS-Ninja-Scanner**: https://github.com/MottaSec/ICS-Ninja-Scanner


---

## 📁 Workshop Deliverables

- Vulnerable firmware for ESP32
- Pre-configured Mosquitto broker configuration
- Python scripts for retained message injection and topic flooding
- Quick-reference MQTT attack and defense cheat sheet
