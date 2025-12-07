# **🌐 Web2Wire: Global IoT Interaction Platform**

Web2Wire is an open-source, full-stack project that allows anyone in the world to interact with a physical device—a custom LED and display setup—via a simple web interface. It serves as a proof-of-concept for securely exposing a local device to the internet using modern tunneling technologies while safeguarding the home network.

## **✨ Features & Architecture**

The project is built around three distinct services and embedded firmware, all housed in a single **Monorepo** for unified development.

| Component | Path | Technology | Role |
| :--- | :--- | :--- | :--- |
| **Frontend (UI)** | `/frontend` | HTML, JS, CSS | User-facing interface for submitting personalized requests. Polls queue status. |
| **Control API** | `/backend/control-api` | Python (Flask, Flask-Limiter), Redis | The primary internet-exposed endpoint. Implements Rate Limiting (5 req/min/IP) to prevent abuse and forwards valid requests. |
| **Queue API** | `/backend/queue-api` | Python (Flask, Threading) | Manages and processes the request queue. Sends commands sequentially to the ESP32 to prevent hardware overload. |
| **Firmware** | `/firmware` | Arduino C++ (ESP32) | Runs a simple HTTP server to receive and execute commands (LED control, display updates, servo moves). |

## **🔒 Security Principles**

* **Zero Port Forwarding:** Utilizes a secure, outbound-only tunnel (e.g., **Cloudflare Tunnel**) to expose the service, eliminating the need to open ports on the home router's firewall.  
* **Rate Limiting:** Protects against simple Denial-of-Service (DoS) attacks by restricting request volume per unique IP address.  
* **Asynchronous Processing:** The Queue API shields the physical hardware from sudden bursts of requests, ensuring stable operation.

## **🚀 Local Development Setup**

Follow these steps to get all services running on your **Intel NUC** and test the full pipeline locally.

### **Prerequisites**

1.  **Python 3:** Installed on the NUC.  
2.  **Redis Server:** Installed and running locally (used by Flask-Limiter for rate tracking).  
3.  **ESP32:** Flashed with the firmware (see /firmware).

### **1\. Configure Local DNS**

To simulate the final domain structure, edit your testing device's **hosts file** (/etc/hosts or C:\\Windows\\System32\\drivers\\etc\\hosts) to point the simulated subdomains to your NUC's static local IP (e.g., 192.168.1.10).

```text
# Example for NUC IP 192.168.1.10
192.168.1.10    web.myproject.local
192.168.1.10    api.myproject.local
```

### **2\. Install Python Dependencies**

Navigate to the root project directory and install the necessary libraries using the requirements.txt file.

\# Ensure you are in the root directory (where backend/ is located)  
pip install \-r requirements.txt

### **3\. Start Redis Server**

Ensure your Redis service is running, as the Control API depends on it for rate limiting.

\# This command may vary based on your OS/install method  
redis-server 

### **4\. Start the Queue API (Job Manager)**

The Queue API runs on port **5001** and manages the job backlog. This must be running before the Control API.

cd backend/queue-api  
python queue\_api.py

*(Leave this terminal window open.)*

### **5\. Start the Control API (Rate Limiter)**

The Control API runs on port **5000**, handles rate limiting, and forwards valid requests to the Queue API.

cd backend/control-api  
python control\_api.py

*(Leave this terminal window open.)*

### **6\. Test the Frontend**

Open the /frontend/index.html file in your web browser. Since the API endpoints are configured to hit api.myproject.local, your browser will use the hosts file entry and direct requests to your running Control API on the NUC.