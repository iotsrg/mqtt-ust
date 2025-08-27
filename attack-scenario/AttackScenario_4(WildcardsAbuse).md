
#  Attack Scenario 4: Wildcard Subscription Exploitation (Topic Snooping)

Even with authentication enabled, a user who’s allowed to **subscribe too broadly** (e.g., `#` or `+/+`) can **see everything** flowing through the broker - including **admin commands** on `/admin/cmd` and your **sensor data**.

---

##  Goal

Show how an “attacker” user can:

1. Discover topic structure with wildcards
2. Eavesdrop on **/admin/cmd** and sensor topics
   …and then fix it with tight **ACLs** so they only see what they’re supposed to.

---

## Lab Setup (what you already have)

* ESP32:

  * Publishes DHT data to `esp32/sensor`
  * Subscribes to **`/admin/cmd`** for control (LED on/off)
* Mosquitto on Raspberry Pi with usernames/passwords (anonymous disabled)


---

## The Attack: Wide-Open Wildcards

### 1) Loosen ACLs

Give the attacker4 read access to **everything** to show impact. Edit `/etc/mosquitto/acl`:

```conf
# Legit device user (ESP32)
user espuser
topic read /admin/cmd
topic write esp32/sensor

# Attacker4 (for the demo – too broad!)
user attacker4
topic read #
```

Reload:

```bash
sudo systemctl restart mosquitto
```

> (Ensure `acl_file /etc/mosquitto/acl` is set in `/etc/mosquitto/mosquitto.conf` and `allow_anonymous false`.)

### 2) Attacker enumerates topics and eavesdrops

Open a terminal (attacker machine or same Pi):

```bash
mosquitto_sub -u attacker4 -P attackerpass -h <RPI_IP> -t "#" -v
```

Now **trigger an admin command** from your admin user or any authorized client:

```bash
mosquitto_pub -u espuser -P your_password -h <RPI_IP> -t /admin/cmd -m "on"
```

**Attacker4 sees:**

```
/admin/cmd on
```

Also sees **all telemetry**:

```
/esp32/sensor {"temp":28.7,"hum":88.9}
```

### 3) Snoop broker internals

If not restricted, `$SYS/#` reveals broker stats:

```bash
mosquitto_sub -u attacker4 -P attackerpass -h <RPI_IP> -t '$SYS/#' -v
```

If it is restricted and configured properly, only Authenticated admins can read the internal stats of MQTT

<img width="1595" height="919" alt="image" src="https://github.com/user-attachments/assets/f3fd3db5-1af4-40ff-b9d8-9f61160f4325" />


> This can leak client IDs, uptime, subscriptions, message rates, etc.

---

## Why this matters

* One overly-permissive subscription exposes **all device traffic** (privacy leak).
* Attackers learn **topic names** and structure to craft further attacks.
* Seeing `/admin/cmd` lets them understand your control semantics.

---

## Remediation: Principle of Least Privilege (No Wildcards)

### 1) Tighten ACLs - **Exact topics only**

Replace your `/etc/mosquitto/acl` with **narrow rules**:

```conf
# --- Device user (ESP32) ---
user espuser
topic read /admin/cmd
topic write /esp32/sensor

# --- Dashboard user (read-only UI), example ---
user dashboard
topic read /esp32/sensor
# (No access to /admin/cmd)

# --- Admin operator (can issue commands) ---
user admin
topic readwrite /admin/cmd
topic read /esp32/sensor

# --- Attacker4 (restricted for demo) ---
user attacker4
# Only allowed to read a harmless test area:
topic read test/#

# --- Optionally: lock down $SYS to admins only ---
user admin
topic read $SYS/#
```

Reload:

```bash
sudo systemctl restart mosquitto
```

### 2) Verify the fix

**Attacker tries wildcards:**

```bash
mosquitto_sub -u attacker4 -P attackerpass -h <RPI_IP> -t "#" -v -d
mosquitto_sub -u attacker4 -P attackerpass -h <RPI_IP> -t "/admin/#" -v -d
mosquitto_sub -u attacker4 -P attackerpass -h <RPI_IP> -t "/esp32/#" -v -d
```

* The client will connect (CONNACK 0), but **won’t receive anything**.
* With logging enabled, your broker will show entries like:

  ```
  Denied SUBSCRIBE to '/admin/#' for username 'attacker4'
  ```

**Attacker can still read allowed test topics:**

```bash
mosquitto_sub -u attacker4 -P attackerpass -h <RPI_IP> -t "test/#" -v
```

### 3) (Optional) Separate roles by users

* Use **different usernames** for ESP32 devices, dashboards, admins
* Keep ACLs **exact** (no `#` unless you truly need it)
* Avoid granting `readwrite` to broad prefixes

### 4) (Optional) Hide `$SYS/#`

If you enabled `$SYS`, **do not** grant `$SYS/#` to untrusted users.
Only admins should have:

```conf
user admin
topic read $SYS/#
```

---

## Workshop Flow

1. Show attacker subscribing to `#` and `$SYS/#` → watch everything fly by
2. Send `/admin/cmd on` and show attacker sees it
3. Tighten ACLs, restart
4. Attacker tries again → **blocked** (no data; logs show “Denied SUBSCRIBE”)
5. Attacker can still read only `test/#` (prove least privilege, not outage)

---

## Useful commands (recap)

```bash
# Follow broker logs (new terminal)
sudo tail -f /var/log/mosquitto/mosquitto.log

# Attacker snooping (before fix)
mosquitto_sub -u attacker4 -P attackerpass -h <RPI_IP> -t "#" -v

# Send admin command (authorized)
mosquitto_pub -u admin -P adminpass -h <RPI_IP> -t /admin/cmd -m "off"

# After fix: Attacker tries, sees nothing
mosquitto_sub -u attacker4 -P attackerpass -h <RPI_IP> -t "/admin/#" -v -d
```

---

##  Key Takeaways

* **Don’t grant wildcard subscriptions** to non-admin users.
* **Exact topic ACLs** enforce least privilege:

  * ESP32: `read /admin/cmd`, `write /esp32/sensor`
  * Admin: `readwrite /admin/cmd`
  * Dashboard: `read /esp32/sensor`
* Optionally restrict `$SYS/#` to admins.

---

