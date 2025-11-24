
# MQTT Mayhem: Publish & Pwn  [DIY Labs]
_Exploiting and Securing MQTT in IoT Environments_

---

## Overview


**Target Audience:** IoT Pentesters, Firmware Developers, Embedded Security Engineers  
**Goal:** Demonstrate practical MQTT misconfigurations, vulnerabilities, and defenses through real-world demos using ESP32 and MQTT broker setup.

---

## Lab Setup

### Required Tools:
- MQTT Broker 
- MQTT clients and visualizers 
- MQTT pentest tools (MQTT-Fuzz - FUME)
- ESP32 development board (acting as a vulnerable MQTT client)
- Raspberrypi (acts as MQTT Broker)

### Steps to setup MQTT Client [ESP32]:
### [Client Setup](https://github.com/iotsrg/mqtt-ust/blob/main/Rpi_Installation.md)
---

### Steps to setup MQTT Broker [Rpi]:
### [Broker Setup](https://github.com/iotsrg/mqtt-ust/blob/main/ESP32_Installation-MQTT_DHT(Client).md)
---

##  Steps

### Setup 
- Start the broker and connect it to the ESP32 client
- Setup topic subscriptions and sensor/actuator emulation
- Confirm working communication flow between ESP32 and broker

---

### MQTT Protocol Deep Dive (Good to know before pentesting/Labs)
- Cover the MQTT architecture: clients, broker, topics, sessions
- Explore the publish/subscribe model
- Explore QoS levels, retained messages, wildcard subscriptions
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
## Best Practices:
### [Link](https://github.com/iotsrg/mqtt-ust/blob/main/Best_Practices.md)
---

## References


- https://www.instructables.com/Secure-Mosquitto-MQTT-Server-for-IoT-Devices-ESP32/
- https://github.com/PBearson/FUME-Fuzzing-MQTT-Brokers
- https://gbhackers.com/vulnerabilities-hardy-barth-ev-station/
- https://www.onekey.com/resource/critical-vulnerabilities-in-ev-charging-stations-analysis-of-echarge-controllers
