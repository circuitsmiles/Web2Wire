# 🌐 Web2Wire: Global IoT Interaction Platform

Web2Wire is an open-source, full-stack project that allows anyone in the world to interact with a physical device—a custom LED and display setup—via a simple web interface. It serves as a proof-of-concept for securely exposing a local device to the internet using modern tunneling technologies while safeguarding the home network.

## ✨ Features & Architecture

The project is built around three distinct services and embedded firmware, all housed in a single **Monorepo** for unified development. 

| Component | Path | Technology | Role |
| :--- | :--- | :--- | :--- |
| **Frontend (UI)** | `/frontend` | HTML, JS, CSS | User-facing interface for submitting personalized requests. Polls queue status. |
| **Control API** | `/backend/control-api` | Python (Flask, **Flask-Limiter**), Redis | The primary internet-exposed endpoint. Implements **Rate Limiting** (5 req/min/IP) to prevent abuse and forwards valid requests. |
| **Queue API** | `/backend/queue-api` | Python (Flask, Threading) | Manages and processes the request queue. Sends commands sequentially to the ESP32 to prevent hardware overload. |
| **Firmware** | `/firmware` | Arduino C++ (ESP32) | Runs a simple HTTP server to receive and execute commands (LED control, display updates, servo moves). |

---

## 🔒 Security Principles

* **Zero Port Forwarding:** Utilizes a secure, outbound-only tunnel (e.g., **Cloudflare Tunnel**) to expose the service, eliminating the need to open ports on the home router's firewall.
* **Rate Limiting:** Protects against simple Denial-of-Service (DoS) attacks by restricting request volume per unique IP address.
* **Asynchronous Processing:** The Queue API shields the physical hardware from sudden bursts of requests, ensuring stable operation.

---

## 🚀 Local Development Setup

Follow these steps to get all services running on your **Intel NUC** and test the full pipeline locally.

### Prerequisites

1.  **Python 3:** Installed on the NUC.
2.  **Redis Server:** Installed and running locally (used by `Flask-Limiter` for rate tracking).
3.  **ESP32:** Flashed with the firmware (see `/firmware`).

### 1. Configure Local DNS

To simulate the final domain structure, edit your testing device's **hosts file** (`/etc/hosts` or `C:\Windows\System32\drivers\etc\hosts`) to point the simulated subdomains to your NUC's static local IP (e.g., `192.168.1.10`).