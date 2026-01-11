import threading
import time
import requests
import json
import os
import sys
import hmac
from flask import Flask, jsonify, request
from flask_cors import CORS
from flask_limiter import Limiter
from flask_limiter.util import get_remote_address
import redis 

# --- CONFIGURATION ---

# 🔑 API KEY SECURITY: Read from the Systemd Environment Variable
# The key is securely passed in via the Systemd service file.
SECRET_ENV_VAR = 'ESP32_JOB_COMPLETE_SECRET'
ESP32_API_SECRET = os.environ.get(SECRET_ENV_VAR)

if not ESP32_API_SECRET:
    print(f"[FATAL] Environment variable '{SECRET_ENV_VAR}' is not set!")
    print("[FATAL] Application cannot start without a secure secret key.")
    sys.exit(1) # Terminate the application if the secret is missing

# --- NETWORK CONFIGURATION (Still needed for communicating TO the ESP32) ---
# Note: This IP address is for the server to initiate communication.
ESP32_IP = "192.168.2.13" 
ESP32_PORT = 80
ESP32_JOB_START_URL = f"http://{ESP32_IP}:{ESP32_PORT}/api/job/start"

# Redis Configuration
# Note: Since the control API runs locally, localhost is correct.
REDIS_HOST = '127.0.0.1' 
REDIS_PORT = 6379
# Keys used in Redis for state persistence
REDIS_QUEUE_KEY = 'web2wire:job_queue'
REDIS_STATE_KEY = 'web2wire:device_state'

# Status States
STATUS_IDLE = "IDLE"
STATUS_PROCESSING = "PROCESSING"
MAX_QUEUE_SIZE = 10 # Reject requests if the queue is larger than this limit

# --- APPLICATION STATE & PERSISTENCE (REDIS) ---
app = Flask(__name__)
# Enable CORS for the frontend (running on a different port/origin)
CORS(app) 

# Thread-safe lock is used for critical state updates within this process
state_lock = threading.Lock() 

# --- Rate Limiting Setup (Using Redis) ---
limiter = Limiter(
    key_func=get_remote_address, 
    default_limits=["50 per minute", "1000 per hour"], 
    # Use 127.0.0.1 for local Redis storage
    storage_uri="redis://127.0.0.1:6379" 
)
limiter.init_app(app) 

# Initialize Redis connection
try:
    # Ensure Redis is explicitly pointing to the local loopback address for consistency
    r = redis.Redis(host='127.0.0.1', port=REDIS_PORT, decode_responses=True)
    r.ping()
    print(f"[INIT] Successfully connected to Redis at 127.0.0.1:{REDIS_PORT}.")
    
    # Initialize the device state if it doesn't exist
    if not r.get(REDIS_STATE_KEY):
        r.set(REDIS_STATE_KEY, STATUS_IDLE)
        print(f"[INIT] Device state initialized to {STATUS_IDLE} in Redis.")

except redis.exceptions.ConnectionError as e:
    print(f"[FATAL] Could not connect to Redis: {e}")
    print("[FATAL] Please ensure Redis server is running on 127.0.0.1:6379.")
    r = None # Set r to None to fail subsequent Redis operations

# --- REDIS HELPER FUNCTIONS ---

def _get_device_state():
    """Reads the current device state from Redis."""
    if r:
        # Use a fallback in case the key is missing
        return r.get(REDIS_STATE_KEY) or STATUS_IDLE
    return STATUS_IDLE # Fallback if Redis failed to connect

def _set_device_state(state):
    """Writes the current device state to Redis."""
    if r:
        r.set(REDIS_STATE_KEY, state)
        
def _get_queue_size():
    """Returns the current queue length from Redis."""
    if r:
        return r.llen(REDIS_QUEUE_KEY)
    return 0
    
def _pop_next_job():
    """Pops the next job (FIFO) from the Redis list."""
    if r:
        # LPOP retrieves and removes the oldest element (FIFO)
        job_json = r.lpop(REDIS_QUEUE_KEY)
        if job_json:
            return json.loads(job_json)
    return None

# --- INTERNAL JOB PROCESSING ---

def _send_job_to_esp32(job_data):
    """
    Internal function to send the job request to the ESP32 device.
    Runs in a separate thread.
    """
    
    # 1. Read current state from Redis
    device_state = _get_device_state()
    
    # Check status outside the lock first for efficiency
    if device_state != STATUS_IDLE:
        print(f"[PROCESSOR] Device is busy ({device_state}). Job should have been skipped.")
        return

    # 2. Update state to PROCESSING in Redis
    with state_lock:
        _set_device_state(STATUS_PROCESSING)
        print(f"[PROCESSOR] State set to {STATUS_PROCESSING}. Sending job to ESP32...")

    try:
        # Step 3: Server sends the request to esp32
        print(f"[PROCESSOR] Sending POST request to ESP32 at: {ESP32_JOB_START_URL}")
        
        # job_data now includes 'flag' field
        response = requests.post(
            ESP32_JOB_START_URL, 
            json=job_data,
            timeout=5 
        )
        
        if response.status_code == 202:
            print(f"[ESP32] Job successfully handed over. Status: {response.status_code}")
        else:
            print(f"[ERROR] ESP32 device rejected job. Status: {response.status_code}. Response: {response.text}")
            _handle_device_failure(job_data)

    except requests.exceptions.RequestException as e:
        print(f"[ERROR] Communication failed with ESP32 at {ESP32_IP}: {e}")
        _handle_device_failure(job_data)

def _handle_device_failure(failed_job_data):
    """Handles communication failure or rejection from the ESP32."""
    with state_lock:
        _set_device_state(STATUS_IDLE)
        # Note: If desired, you could re-queue the job here: r.lpush(REDIS_QUEUE_KEY, json.dumps(failed_job_data))
    print("[ERROR] Device failure handled. State reset to IDLE in Redis.")


def _processor_loop():
    """Continuously checks the queue and starts processing if a job is available."""
    while True:
        device_state = _get_device_state()
        queue_size = _get_queue_size()
        
        # Only attempt to process if the device is IDLE and the queue is not empty
        if device_state == STATUS_IDLE and queue_size > 0:
            
            # Safely pop the job from Redis
            next_job = _pop_next_job()
            
            if next_job:
                print(f"[QUEUE] Popped job for user: {next_job.get('name', 'N/A')}. Country: {next_job.get('country', 'N/A')}. Flag: {next_job.get('flag', 'N/A')}. Queue size remaining: {_get_queue_size()}")
                
                # Start the non-blocking process to send the job to the ESP32
                job_thread = threading.Thread(target=_send_job_to_esp32, args=(next_job,))
                job_thread.start()
            
        time.sleep(5) # Increased sleep for less log spam while debugging

# Start the background processor thread when the server starts
processor_thread = threading.Thread(target=_processor_loop, daemon=True)
processor_thread.start()
print("[INIT] Background processor thread started.")


# --- Health Check ---
@app.route('/', methods=['GET'])
def health_check():
    """Simple health check endpoint."""
    return jsonify({
        "status": "Control API operational (Rate Limited)", 
        "redis_status": "Connected" if r else "Disconnected",
        "rate_limit": "5 requests per minute per IP"
    })

# --- Main Endpoint: Handle new pulse requests from the frontend ---
@app.route('/api/request/new', methods=['POST'])
@limiter.limit("5 per minute", override_defaults=False) 
def new_request():
    """
    Receives a new pulse request from the web frontend.
    1. Validates input (name, country, flag).
    2. Checks the current queue size (soft limit) via Redis.
    3. Pushes the request data directly to the Redis Queue.
    """
    data = request.get_json()
    name = data.get('name')
    country = data.get('country')
    # --- NEW: Extract the 'flag' field (2-letter country code) ---
    flag = data.get('flag') 

    # --- UPDATED VALIDATION ---
    if not name or not country or not flag:
        return jsonify({"message": "Missing 'name', 'country', or 'flag' field (required 2-letter code) in request payload."}), 400

    try:
        current_queue_size = _get_queue_size()

        if current_queue_size >= MAX_QUEUE_SIZE:
            # Soft Limit Exceeded (Queue Full)
            return jsonify({
                "message": f"Queue is full. Current size is {current_queue_size}. Please try again later.",
                "queue_size": current_queue_size
            }), 429 

        # Construct job data
        job_data = {
            'timestamp': time.time(),
            'name': name,
            'country': country,
            'flag': flag # --- NEW: Include flag in the job data ---
        }
        
        # 2. Push valid request to the Redis Queue (RPUSH adds to the right/end for FIFO)
        job_json = json.dumps(job_data)
        if r:
            r.rpush(REDIS_QUEUE_KEY, job_json)

        # The new size is the client's position in the queue
        new_queue_size = current_queue_size + 1 
        
        return jsonify({
            "message": f"Pulse request accepted for {name} from {country} ({flag}).",
            "queue_size": new_queue_size
        }), 200

    except Exception as e:
        print(f"[ERROR] An unexpected server error occurred during request processing: {e}")
        return jsonify({"message": f"An unexpected server error occurred: {e}"}), 500

# --- ESP32 CALLBACK ENDPOINT (SECURED WITH API KEY) ---
@app.route('/api/job/complete', methods=['POST'])
def job_complete():
    """
    [STEP 4] Endpoint called by the ESP32 after it has finished the action.
    This endpoint is STRICTLY restricted by checking the 'Authorization: Bearer <Key>' header.
    """
    
    # 1. Authorization Header Check
    auth_header = request.headers.get('Authorization')
    
    if not auth_header or not auth_header.startswith('Bearer '):
        print("[SECURITY] Access denied. Missing or invalid Authorization header.")
        return jsonify({"message": "Unauthorized. Missing or invalid Authorization header."}), 401

    # Extract the submitted key (Bearer <key> -> <key>)
    submitted_key = auth_header.split('Bearer ')[1]

    # 2. Constant-Time Key Comparison (Prevents Timing Attacks)
    # The stored key is from the secure environment variable (ESP32_API_SECRET).
    try:
        if not hmac.compare_digest(submitted_key.encode('utf-8'), ESP32_API_SECRET.encode('utf-8')):
            print("[SECURITY] Access denied. Invalid API key submitted.")
            return jsonify({"message": "Forbidden. Invalid API Key."}), 403
    except Exception as e:
        # Catch encoding errors or other issues during comparison
        print(f"[SECURITY ERROR] Key comparison failed: {e}")
        return jsonify({"message": "Internal security error."}), 500
        
    # --- Authentication Successful ---
    data = request.json
    
    if data and data.get('status') == 'completed':
        print("[API] Job completion signal received from authorized ESP32. Resetting state.")
        
        # Safely reset device state to IDLE in Redis
        with state_lock:
            _set_device_state(STATUS_IDLE)
        
        # The processor thread will automatically check for the next job.
        return jsonify({
            "message": "Device state reset. Queue processing continues.",
            "queue_size": _get_queue_size(),
            "device_state": _get_device_state()
        }), 200
    
    return jsonify({"message": "Invalid completion status received."}), 400


if __name__ == '__main__':
    # Running on port 5000 
    app.run(host='127.0.0.1', port=5000)