import json
from flask import Flask, jsonify
from flask_cors import CORS
import redis

# --- CONFIGURATION ---
# Redis Configuration (Must match Control API)
REDIS_HOST = 'localhost'
REDIS_PORT = 6379
# Keys used in Redis (Must match control_api.py)
REDIS_QUEUE_KEY = 'web2wire:job_queue'
REDIS_STATE_KEY = 'web2wire:device_state'
# Status States (for fallback)
STATUS_OFFLINE = "OFFLINE"

# --- APPLICATION SETUP ---
app = Flask(__name__)
# Enable CORS for the frontend
CORS(app) 

# Initialize Redis connection
r = None
try:
    r = redis.Redis(host=REDIS_HOST, port=REDIS_PORT, decode_responses=True)
    r.ping()
    print(f"[INIT] Successfully connected to Redis at {REDIS_HOST}:{REDIS_PORT}.")
except redis.exceptions.ConnectionError as e:
    print(f"[FATAL] Could not connect to Redis: {e}")
    print("[FATAL] Please ensure Redis server is running. Status will report OFFLINE.")
    # r remains None

def _get_status_from_redis():
    """Reads the queue size and device state directly from Redis."""
    if not r:
        # Fallback if Redis failed to initialize
        return 0, STATUS_OFFLINE
        
    try:
        # Get queue length
        queue_size = r.llen(REDIS_QUEUE_KEY)
        # Get device state, defaulting if key is missing
        device_state = r.get(REDIS_STATE_KEY) or STATUS_OFFLINE
        return queue_size, device_state
    except Exception as e:
        print(f"[ERROR] Error reading from Redis: {e}")
        return 0, STATUS_OFFLINE


# --- QUEUE API ENDPOINTS (Port 5001) ---

@app.route('/api/queue/status', methods=['GET'])
def queue_status():
    """
    Endpoint called by the UI for real-time status updates, reading state from Redis.
    This provides the separation of concerns required for the Queue API.
    """
    queue_size, device_state = _get_status_from_redis()
    
    return jsonify({
        "queue_size": queue_size,
        "device_state": device_state
    }), 200

# --- Health Check ---
@app.route('/', methods=['GET'])
def health_check():
    """Simple health check endpoint."""
    return jsonify({
        "status": "Queue API operational", 
        "redis_status": "Connected" if r else "Disconnected",
        "provides": "/api/queue/status"
    })


if __name__ == '__main__':
    # Runs on port 5001 
    app.run(host='0.0.0.0', port=5001)