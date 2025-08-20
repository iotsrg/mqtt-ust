
---

#  MQTT Security Best Practices

### 1. **Authentication**

* Require **username + strong password** for all clients.
* Avoid anonymous connections (`allow_anonymous false` in Mosquitto).
* Use **unique credentials per client/device** (not shared accounts).
* Store secrets securely on devices (not hardcoded in plain text if possible).

---

### 2. **Authorization**

* Use **Access Control Lists (ACLs)** to restrict clients to specific topics.

  * Example: `/esp32/sensor` only for ESP32.
  * Example: `/admin/cmd` only for admin clients.
* Enforce **principle of least privilege** (no `#` subscriptions for regular clients).
* Separate **publish** and **subscribe** permissions.

---

### 3. **Confidentiality & Integrity (TLS/SSL)**

* Always enable **TLS (port 8883)** to encrypt data in transit.
* Use trusted CA-signed or well-distributed self-signed certificates.
* Pin broker’s CA certificate in IoT devices (ESP32).
* Enforce **certificate validation** to prevent MITM attacks.
* Optionally require **mutual TLS (client certificates)** for high-security use cases.

---

### 4. **Client Identity & Session Management**

* Use **unique client IDs** for each device.
* Disable `clean_session` for critical devices (so they don’t lose subscriptions).
* Limit **maximum connections per user** to prevent abuse.
* Set appropriate **keepalive intervals** to detect dropped clients quickly.

---

### 5. **QoS & Reliability**

* Choose **QoS 1 or 2** only when necessary (to avoid flooding or replay).
* Don’t let untrusted users abuse high QoS levels.
* Limit **retained messages** to avoid attackers leaving malicious payloads behind.

---

### 6. **Broker Hardening**

* Disable unused listeners (only expose required ports: 8883 instead of 1883).
* Bind broker to **internal network** if devices don’t need internet exposure.
* Limit **maximum message size** to prevent memory exhaustion attacks.
* Limit **max\_inflight\_messages** and **max\_queued\_messages** to resist flooding.
* Enable **logging** and monitor unusual connection patterns.

---

### 7. **Protection Against DoS / Flooding**

* Rate-limit publish frequency per client.
* Restrict **wildcard subscriptions** (`#` or `+`) for normal users.
* Use firewalls or reverse proxies (like **NGINX or EMQX Gateway**) to filter traffic.

---

### 8. **Device Security**

* Secure ESP32 firmware (avoid hardcoding secrets in plain).
* Use **secure boot** and **flash encryption** (ESP32 supports both).
* Regularly rotate credentials/certificates.
* Update firmware to patch vulnerabilities.

---

### 9. **Monitoring & Logging**

* Enable Mosquitto’s **detailed logs** (`log_type all`).
* Monitor for failed logins, repeated publish attempts, topic floods.
* Integrate logs into a SIEM for anomaly detection.

---

### 10. **Deployment Practices**

* Isolate MQTT broker on a **separate VLAN/segment** (esp. in OT/ICS networks).
* Don’t expose MQTT directly to the internet — use a **VPN** or secure gateway.
* If remote access is needed, put MQTT behind an **authenticated reverse proxy**.
* Run broker as a **non-root user** to limit impact of compromise.

---

## Quick Remediation Mapping (from labs/Demos)

* **Anonymous access** → Enforce auth + ACLs.
* **Unauthorized publish/subscribe** → ACLs.
* **Topic flooding** → Rate-limit + resource caps.
* **Sniffing MITM** → TLS/SSL with certs.
* **Wildcards abuse** → Restrict `#` usage.
* **DoS risk** → Limit inflight, queued messages, use monitoring.

---
