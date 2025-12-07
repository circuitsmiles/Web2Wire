import os
import time
from flask import Flask, request, jsonify
from flask_cors import CORS
from collections import deque

# --- Configuration ---
app = Flask(__name__)
# Enable CORS for the frontend (running on a different port/origin)
CORS(app) 

# Use a thread-safe deque for the FIFO job queue
job_queue = deque()

# --- Health Check ---
@app.route('/', methods=['GET'])
def health_check():
    """Simple health check endpoint."""
    return jsonify({"status": "Queue API operational", "queue_size": len(job_queue)})

# --- Endpoint 1: Get Current Queue Status (for Frontend) ---
@app.route('/api/queue/status', methods=['GET'])
def get_status():
    """Returns the current number of items in the queue."""
    return jsonify({"queue_size": len(job_queue)}), 200

# --- Endpoint 2: Add Item to Queue (for Control API) ---
@app.route('/api/queue/add', methods=['POST'])
def add_job():
    """Adds a new job payload (from the Control API) to the end of the queue."""
    data = request.get_json()
    if not data:
        return jsonify({"message": "Invalid job payload."}), 400

    # Add timestamp and current size to the job data for logging/tracking
    job_payload = {
        "timestamp": time.time(),
        "id": len(job_queue) + 1,
        "name": data.get('name'),
        "country": data.get('country')
    }
    
    job_queue.append(job_payload)
    print(f"Job added: {job_payload['name']} from {job_payload['country']}. New size: {len(job_queue)}")
    
    return jsonify({"message": "Job added successfully."}), 200

# --- Endpoint 3: Get Next Job (for ESP32 device) ---
@app.route('/api/queue/next', methods=['GET'])
def get_next_job():
    """
    Pops the oldest job from the queue and returns it to the physical device.
    This is the endpoint the ESP32 will constantly poll.
    """
    if job_queue:
        next_job = job_queue.popleft()
        print(f"Job delivered: {next_job['name']} from {next_job['country']}. Remaining: {len(job_queue)}")
        
        # We only send back the critical data the ESP32 needs
        return jsonify({
            "status": "success",
            "job": {
                "name": next_job['name'],
                "country": next_job['country']
            }
        }), 200
    else:
        # If the queue is empty, inform the device
        return jsonify({"status": "empty", "message": "Queue is empty. No jobs available."}), 204


if __name__ == '__main__':
    # Running on port 5001 as configured in the Control API and the frontend (QUEUE_STATUS_URL)
    app.run(port=5001)