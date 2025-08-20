
---

# Attack Scenario 3: **Retained Message Abuse**

###  Goal:

Show how an attacker can **inject a persistent command** into a topic (like `/admin/cmd`) using a **retained message**, so that the ESP32 executes the command **automatically on reconnect** - even if the attacker is no longer active.

---

##  Why This Matters

* In MQTT, a **retained message** stays on the broker and is **delivered to every new subscriber** immediately after subscription.
* This means a malicious command (e.g., `led = on`) can be stored and **replayed silently** to a device **forever**, until it's overwritten or cleared.

---

##  Attack Setup

###  Assumptions:

* ESP32 subscribes to: `/admin/cmd`
* MQTT broker is running and allows `attacker3` to publish to that topic
* ESP32 interprets the message and turns on the LED

---

###  ESP32 Snippet (Already in your code):

```cpp
if (String(topic) == "/admin/cmd") {
  if (msg == "on") digitalWrite(LEDPIN, HIGH);
  else if (msg == "off") digitalWrite(LEDPIN, LOW);
}
```

---

###  Temporarily Allow Attacker3 in ACL (for demo)

Modify `/etc/mosquitto/acl`:

```conf
user attacker3
topic readwrite /admin/cmd
```

Restart Mosquitto:

```bash
sudo systemctl restart mosquitto
```

---

###  Attacker Action: Inject Retained Command

```bash
mosquitto_pub -u attacker3 -P attackerpass -h 192.168.1.105 -t /admin/cmd -m "on" -r
```

✅ `-r` = retained
This stores `"on"` on topic `/admin/cmd` **on the broker**

---

###  Demo the Effect

1. **Reboot the ESP32** (or power cycle it)
2. It reconnects and resubscribes to `/admin/cmd`
3. Immediately, broker sends `"on"` → LED turns on
    Even though no attacker is online!

You’ll see no activity on the attacker’s terminal - it’s **silent, stealthy persistence**

---

##  Impact

* Malicious control payloads persist **indefinitely**
* Devices can be hijacked **long after** the attacker is gone
* Victims may not even realize where the command is coming from

---

##  Remediation: Handle Retained Messages Securely

### ✅ 1. **Clear Retained Messages Explicitly**

To delete the retained message from the broker:

```bash
mosquitto_pub -u attacker3 -P attackerpass -h 192.168.1.105 -t /admin/cmd -n -r
```

 `-n` = send **null (empty)** payload
✅ This **removes** the retained message

---

### ✅ 2. **Disable Retained Message Handling on ESP32**

If you want ESP32 to **ignore retained messages**, you can:

#### Option A: Programmatically check message flags (advanced):

Unfortunately, `PubSubClient` for Arduino does **not expose MQTT flags**, so this is **not easy with PubSubClient**.

#### Option B: Clear retained messages periodically on the broker (admin-side fix)

---

### ✅ 3. **Restrict Retained Publish Rights**

Update ACLs:

```conf
user attacker3
topic write test/#
# Do not allow access to esp32/led anymore since topic level access is not granted /admin/cmd
```

---

### ✅ 4. **Use Per-Topic Retain Policy (Mosquitto 2.x)**

Advanced: In Mosquitto 2.0+, you can **prevent retention** for specific topics.

```conf
topic /admin/cmd retain false
```

---

##  Summary for Workshop

| Item              | Desc                                                                |
| ----------------- | ------------------------------------------------------------------- |
| **Vulnerability** | MQTT retained messages can be used to persist malicious commands    |
| **Impact**        | Device executes commands long after attacker is gone                |
| **Demo**          | Publish `"on"` to `/admin/cmd` with `-r`; reboot ESP32; LED turns on |
| **Fixes**         | Clear retained messages, restrict publish access, clear on reboot   |

---

> "MQTT retains messages by design - great for sensor dashboards, but dangerous for control topics unless explicitly managed."

---
