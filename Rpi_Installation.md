Steps to install MQTT in Rpi:

To install an MQTT broker (like Mosquitto) on Raspberry Pi OS (formerly Raspbian), follow these steps:

---

### ✅ Step-by-Step: Installing Mosquitto on Raspberry Pi

📍 These steps work for Raspberry Pi OS (Lite or Desktop).

---

### 1. Update your system

Open terminal and run:

```bash
sudo apt update
sudo apt upgrade -y
```

---

### 2. Install Mosquitto broker and client

```bash
sudo apt install -y mosquitto mosquitto-clients
```

This installs both:

* The broker: `mosquitto`
* CLI tools: `mosquitto_pub`, `mosquitto_sub`

---

### 3. Enable Mosquitto to start on boot

```bash
sudo systemctl enable mosquitto
```

To start it now:

```bash
sudo systemctl start mosquitto
```

To check status:

```bash
sudo systemctl status mosquitto
```

---

### 4. Test the MQTT Broker Locally

In one terminal:

```bash
mosquitto_sub -h localhost -t test/topic
```

In another terminal:

```bash
mosquitto_pub -h localhost -t test/topic -m "Hello from Pi"
```

You should see the message received in the subscriber terminal.

---

### 5. (Optional) Allow remote devices (like ESP32) to connect

Edit Mosquitto config:

```bash
sudo nano /etc/mosquitto/mosquitto.conf
```

At the bottom, add:

```conf
listener 1883
allow_anonymous true
```

Save & exit (`Ctrl+O`, `Enter`, `Ctrl+X`), then restart:

```bash
sudo systemctl restart mosquitto
```

---
