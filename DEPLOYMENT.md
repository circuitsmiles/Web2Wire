## **🚀 Web2Wire Deployment Guide: circuitsmiles.dev via Cloudflare Tunnel**

This document details the complete deployment, security, and persistence setup for the Web2Wire application on a Debian server. The application services will be run locally, and securely exposed to the public internet using a Cloudflare Tunnel.

### **📌 Key Information**

| Component | Value |
| :---- | :---- |
| **Domain** | circuitsmiles.dev |
| **Tunnel Name** | web2wire-prod |
| **Tunnel UUID** | use your tunnel id |
| **Admin User** | home (Your main, privileged user) |
| **Low-Power User** | web2wire (Dedicated user for running services) |
| **Local Services** | Frontend (8080), Control API (5000), Queue API (5001) |

### **Phase 1: User and Repository Setup**

| User | Command | Explanation |
| :---- | :---- | :---- |
| **home (Admin)** | sudo adduser web2wire | Creates a new user with reduced privileges (web2wire). This user will own and run all application and tunnel services for security separation. |
| **home (Admin)** | sudo git clone https://github.com/circuitsmiles/Web2Wire.git /home/web2wire/Web2Wire | Clones the source code repository directly into the low-power user's home directory. |
| **home (Admin)** | sudo chown \-R web2wire:web2wire /home/web2wire/Web2Wire | Assigns full ownership of the entire project directory and its contents to the web2wire user and group. This is mandatory for the user to read, write, and execute files. |

### **Phase 2: Local Service Setup and Configuration**

#### **1\. Run Local Redis Service**

We run Redis locally on 127.0.0.1:6379. This is an essential backend data store for the application and is run using Docker.

| User | Command | Explanation |
| :---- | :---- | :---- |
| **home (Admin)** | docker run \-d \-p 127.0.0.1:6379:6379 \--name web2wire-redis \--restart always redis | Downloads, runs, and restarts the official Redis image. By binding only to 127.0.0.1, we ensure Redis is **only accessible from the server itself**, enhancing security. |

#### **2\. Update Application Host Binding**

The application services (APIs) must be bound to the local loopback address (127.0.0.1) so they are **only** reachable by the local Cloudflare Tunnel client and are not exposed to the public internet or local network directly.

**Action:** Locate the app.run command in your API scripts (e.g., control\_api.py, queue\_api.py) and ensure it specifies the host:

\# Change from app.run(port=...) to:  
app.run(host='127.0.0.1', port=5000)

#### **3\. Update Frontend API URLs**

The browser-based frontend needs to know the public, secure addresses of the APIs. This update ensures all client-side calls go through the Cloudflare Tunnel.

**Action:** In your primary frontend file (e.g., index.html or main JS file), update API URLs:

// Update API URLs to point to the secure, public Cloudflare Tunnel addresses  
const API\_BASE\_URL \= '\[https://api.circuitsmiles.dev\](https://api.circuitsmiles.dev)'; // Target the Control API  
const QUEUE\_STATUS\_URL \= '\[https://queue.circuitsmiles.dev/api/queue/status\](https://queue.circuitsmiles.dev/api/queue/status)'; // Target the Queue API

### **Phase 3: Virtual Environment and Dependencies**

This phase is executed as the low-power user (web2wire) to set up the necessary Python environment.

| User | Command | Explanation |
| :---- | :---- | :---- |
| **home (Admin)** | su \- web2wire | Switches the terminal session to the web2wire user. **All subsequent commands in this section are run as web2wire.** |
| **web2wire** | cd /home/web2wire/Web2Wire | Navigate to the project root directory. |
| **web2wire** | python3 \-m venv venv | Creates a new isolated Python virtual environment named venv. |
| **web2wire** | source venv/bin/activate | Activates the virtual environment. |
| **web2wire** | pip install \-r requirements.txt | Installs all necessary Python dependencies into the isolated environment. |
| **web2wire** | deactivate | Deactivates the virtual environment. |
| **web2wire** | exit | Return to the **Admin (home)** user terminal. |

### **Phase 4: Application Persistence and Security (systemd Services)**

#### **Important Security Enhancement: Environment Variables**

The critical /api/job/complete endpoint is secured by an API key (generated with secrets_generator.py). To prevent this secret key from being hardcoded in the Python source code (which exposes it in Git repositories and makes rotation difficult), we now pass the secret key to the application using a secure Systemd **Environment Variable**. Make sure to update the firmware to use that same key in request.

The Python application must be modified to read the key using os.environ.get('ESP32_JOB_COMPLETE_SECRET').

| User | Command | Explanation |
| :---- | :---- | :---- |
| **home (Admin)** | sudo gedit /etc/systemd/system/web2wire-frontend.service | Creates the service file for the static file server running on port 8080\. |

**Content for web2wire-frontend.service:**

```
[Unit]  
Description=Web2Wire Frontend Static Server (Port 8080)  
After=network.target

[Service]  
User=web2wire  
WorkingDirectory=/home/web2wire/Web2Wire/frontend  
ExecStart=/usr/bin/python3 -m http.server 8080  
Restart=always

[Install]  
WantedBy=multi-user.target
```

| User | Command | Explanation |
| :---- | :---- | :---- |
| **home (Admin)** | sudo gedit /etc/systemd/system/web2wire-control.service | Creates the service file for the Control API running on port 5000\. |

**Content for web2wire-control.service:**

```
[Unit]  
Description=Web2Wire Control API (Port 5000)  
After=network.target docker.service

[Service]  
User=web2wire  
WorkingDirectory=/home/web2wire/Web2Wire/backend  
Environment="ESP32_JOB_COMPLETE_SECRET=your_secret_key_here"
ExecStart=/home/web2wire/Web2Wire/venv/bin/python3 control_api.py  
Restart=always

[Install]  
WantedBy=multi-user.target
```

| User | Command | Explanation |
| :---- | :---- | :---- |
| **home (Admin)** | sudo gedit /etc/systemd/system/web2wire-queue.service | Creates the service file for the Queue API running on port 5001\. |

**Content for web2wire-queue.service:**

```
[Unit]  
Description=Web2Wire Queue API (Port 5001)  
After=network.target docker.service

[Service]  
User=web2wire  
WorkingDirectory=/home/web2wire/Web2Wire/backend  
ExecStart=/home/web2wire/Web2Wire/venv/bin/python3 queue_api.py  
Restart=always

[Install]  
WantedBy=multi-user.target
```

| User | Command | Explanation |
| :---- | :---- | :---- |
| **home (Admin)** | sudo systemctl daemon-reload | Notifies the operating system to load the newly created service definitions. |
| **home (Admin)** | sudo systemctl enable web2wire-control web2wire-queue web2wire-frontend | Configures the services to start automatically every time the server boots. |
| **home (Admin)** | sudo systemctl start web2wire-control web2wire-queue web2wire-frontend | Immediately starts all three application services. |
| **home (Admin)** | sudo systemctl status web2wire-control web2wire-queue web2wire-frontend | Verifies that all three services are running and active. |

### **Phase 5: Firewall Security Setup**

We must secure the server by denying all external access, relying solely on the Cloudflare Tunnel for ingress.

| User | Command | Explanation |
| :---- | :---- | :---- |
| **home (Admin)** | sudo apt update && sudo apt install ufw | Ensures the Uncomplicated Firewall (UFW) is installed. |
| **home (Admin)** | sudo ufw default deny incoming | **Crucial Security Step:** Blocks all incoming traffic by default. This is how we prevent direct access to ports 8080/5000/5001. |
| **home (Admin)** | sudo ufw default allow outgoing | Allows the server to initiate all necessary outbound connections (e.g., to Cloudflare, Redis downloads). |
| **home (Admin)** | sudo ufw allow from 192.168.2.0/24 to any port 22 | **Allows SSH access** only from your specific local network subnet (192.168.2.0/24). **⚠️ ADJUST THIS SUBNET IF YOUR ROUTER IS DIFFERENT.** |
| **home (Admin)** | sudo ufw enable | Activates the firewall with the defined rules. |

### **Phase 6: Cloudflare Tunnel Deployment**

This phase involves configuring the tunnel client to use the local services.

| User | Command | Explanation |
| :---- | :---- | :---- |
| **home (Admin)** | su \- web2wire | Switch to the low-power user. **All subsequent commands in this section are run as web2wire.** |
| **web2wire** | cloudflared tunnel create web2wire-prod | Creates the named tunnel and generates the necessary credentials file ({tunnel_id}.json). |
| **web2wire** | nano \~/.cloudflared/config.yml | Opens the configuration file for editing. |

**Content for config.yml (Ensure the UUID is correct):**

```
tunnel: {tunnel_id}
credentials-file: /home/web2wire/.cloudflared/{tunnel_id}.json

ingress:  
  - hostname: circuitsmiles.dev  
    service: http://localhost:8080  
      
  - hostname: www.circuitsmiles.dev  
    service: http://localhost:8080

  - hostname: api.circuitsmiles.dev  
    service: http://localhost:5000

  - hostname: queue.circuitsmiles.dev  
    service: http://localhost:5001

  - service: http_status:404

```

| User | Command | Explanation |
| :---- | :---- | :---- |
| **web2wire** | cloudflared tunnel route dns web2wire-prod circuitsmiles.dev | Creates the DNS CNAME records required by Cloudflare for the bare domain and subdomains. |
| **web2wire** | cloudflared tunnel route dns web2wire-prod www.circuitsmiles.dev | ... |
| **web2wire** | cloudflared tunnel route dns web2wire-prod api.circuitsmiles.dev | ... |
| **web2wire** | cloudflared tunnel route dns web2wire-prod queue.circuitsmiles.dev | ... |
| **web2wire** | exit | Return to the **Admin (home)** user terminal. |

### **Phase 7: Tunnel Persistence**

We manually create the systemd service for the tunnel to ensure it runs as the low-power web2wire user and starts automatically on boot.

| User | Command | Explanation |
| :---- | :---- | :---- |
| **home (Admin)** | sudo nano /etc/systemd/system/cloudflared.service | Creates the service definition file for the Cloudflare Tunnel. |

**Content for cloudflared.service:**

```
[Unit]  
Description=Cloudflare Tunnel Service  
After=network.target

[Service]  
TimeoutStartSec=0  
Type=notify  
User=web2wire  
ExecStart=/usr/local/bin/cloudflared tunnel --config /home/web2wire/.cloudflared/config.yml run web2wire-prod  
Restart=on-failure  
RestartSec=5s

[Install]  
WantedBy=multi-user.target
```

| User | Command | Explanation |
| :---- | :---- | :---- |
| **home (Admin)** | sudo systemctl daemon-reload | Reloads systemd to recognize the new tunnel service file. |
| **home (Admin)** | sudo systemctl start cloudflared | Starts the Cloudflare Tunnel process. |
| **home (Admin)** | sudo systemctl enable cloudflared | Ensures the tunnel service starts automatically on server boot. |
| **home (Admin)** | sudo systemctl status cloudflared | Confirms the tunnel service is active (running). |

### **Final Verification**

Access the application via the following public URLs to confirm successful deployment:

* https://circuitsmiles.dev  
* https://www.circuitsmiles.dev  
* https://api.circuitsmiles.dev  
* https://queue.circuitsmiles.dev