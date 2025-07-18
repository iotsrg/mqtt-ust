

---

##  Attack Scenario 2: **Lack of Authorization**

Even if authentication is enabled, without **access control (authorization)**, any authenticated user can **publish or subscribe to any topic** — including sensitive or control topics.

---

##  Goal:

* Show how a **legitimate user** can **access or control other devices** by publishing to unauthorized topics.
* Demonstrate **how to restrict topic access** with **Mosquitto ACLs**.

---

##  Setup Before the Attack

###  Current Secure Setup (after Scenario 1):

* MQTT broker (Mosquitto) **requires username/password**
* ESP32 connects using `espuser`
* Anonymous access is disabled

But: No **topic-level restrictions** = Authorization flaw

---

##  Attack Simulation

### Step 1: Create a second user (attacker)

```bash
sudo mosquitto_passwd /etc/mosquitto/passwd attacker
```

###  Step 2: Restart Mosquitto

```bash
sudo systemctl restart mosquitto
```

###  Step 3: Attacker connects and **turns on ESP32’s LED**

```bash
mosquitto_pub -u attacker -P attackerpass -h <RPI_IP> -t /admin/cmd -m "on"
```

✅ **LED on ESP32 turns on**

###  Step 4: Attacker reads DHT data

```bash
mosquitto_sub -u attacker -P attackerpass -h <RPI_IP> -t esp32/sensor
```

 Attacker gets temperature and humidity

---

##  Why This Is Dangerous:

* Even though the attacker is authenticated, they’re **not authorized** to control or read the ESP32
* This can happen in shared environments (e.g., multiple IoT devices using the same broker)

---

##  Remediation: Enable Authorization via ACLs

###  Step 1: Create ACL File

```bash
sudo nano /etc/mosquitto/acl
```

###  Sample ACL Rules:

```bash
# For espuser (owner of ESP32)
user espuser
topic readwrite /admin/cmd
topic readwrite /esp32/sensor

# For attacker (restricted)
user attacker
topic read test/topic
```

###  Step 2: Update Mosquitto Config to use ACLs

Edit `/etc/mosquitto/mosquitto.conf`:

```conf
allow_anonymous false
password_file /etc/mosquitto/passwd
acl_file /etc/mosquitto/acl
```

Restart Mosquitto:

```bash
sudo systemctl restart mosquitto
```

---

##  Test the Fix

###  Attacker tries to publish to /admin/cmd:

```bash
mosquitto_pub -u attacker -P attackerpass -h <RPI_IP> -t /admin/cmd -m "on"
```

You’ll see:

```
Connection Accepted but Publish failed
```

or

```
Error: Not authorized.
```

###  Attacker tries to subscribe to /esp32/sensor:

```bash
mosquitto_sub -u attacker -P attackerpass -h <RPI_IP> -t esp32/sensor
```

 **Access denied**

 Attacker **can still use allowed topics**, like:

```bash
mosquitto_pub -u attacker -P attackerpass -h <RPI_IP> -t test/topic -m "hello"
```

---

##  ESP32 Remains Functional

Make sure `espuser` can still:

* Publish DHT data
* Receive LED commands

---

##  Summary for Workshop

| 🔍                | Item                                                                                |
| ----------------- | ----------------------------------------------------------------------------------- |
| **Vulnerability** | No topic-level access control (authorization)                                       |
| **Impact**        | Any user can control or spy on any device                                           |
| **Demo**          | Attacker user turns on LED and reads DHT data                                       |
| **Fix**           | Implement Mosquitto **ACLs** based on usernames                                     |
| **Bonus**         | Use different **users for different roles/devices** (e.g., dashboard, sensor nodes) |

---



