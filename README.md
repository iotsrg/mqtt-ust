# mqtt-ust
# MQTT Mayhem: Publish & Pwn
_Unleash chaos in MQTT ecosystems, learn exploitation and secure it right._

---

## 🧰 Workshop Summary

- **Title:** MQTT Mayhem: Publish & Pwn  
- **Duration:** 2 Hours  
- **Level:** Intermediate  
- **Objective:** Attack insecure MQTT setups and apply immediate defenses

---

## ⚙️ Requirements

- **OS:** Kali / Parrot / IoTPTv2  
- **Tools:**  
  - `mosquitto`, `mqtt-explorer`, `mqtt-spy`  
  - `nmap`, `wireshark`, `evilmqtt`, `mitmproxy` `mqtt-pwn`, `ioxy`
  - Python 3 + `paho-mqtt`  
- **Network:** Local MQTT broker or lab VM  
- **Browser Access:** Optional for GUI clients
- **hardware Gadgets** ESP32

---

## 🧱 Agenda (2 Hours)

### ⏱ 00:00–00:10 — Lab Setup

- Launch Mosquitto broker (`mosquitto -v`)
- Open `mqtt-explorer` and connect to `localhost:1883`
- Test message publish and subscribe

---

### 🌐 00:10–00:25 — MQTT Protocol Essentials

- Broker ↔ Client architecture  
- MQTT Packet Types: CONNECT, SUBSCRIBE, PUBLISH  
- QoS levels 0, 1, 2 demo  
- Wildcard topics: `#`, `+`  
- Retained messages and session states

---

### 💣 00:25–00:55 — Attacks in Action

#### ✅ 1. Unauthenticated Broker Abuse

```bash
# Subscribe to all topics
mosquitto_sub -h <IP> -t '#'
