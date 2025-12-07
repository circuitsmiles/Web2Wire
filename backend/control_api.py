import os
from flask import Flask, request, jsonify
from flask_cors import CORS
import requests
from flask_limiter import Limiter
from flask_limiter.util import get_remote_address

# --- Configuration ---
# Target the Queue API, which should be running on localhost:5001
QUEUE_API_URL = 'http://127.0.0.1:5001/api/queue' 
MAX_QUEUE_SIZE = 10 # Reject requests if the queue is larger than this limit

app = Flask(__name__)
# Enable CORS for the frontend (running on a different port/origin)
CORS(app) 

# --- Rate Limiting Setup (Using Redis) ---
# Initialize Limiter. Redis is used as the storage backend.
# Assuming Redis is running on default port (6379) on localhost.
limiter = Limiter(
    app,
    key_func=get_remote_address, # Identify users by their IP address
    default_limits=["5 per minute", "100 per hour"], # Global limits for all endpoints
    storage_uri="redis://localhost:6379" # Redis connection URI
)

# --- Health Check ---
@app.route('/', methods=['GET'])
def health_check():
    """Simple health check endpoint."""
    return jsonify({
        "status": "Control API operational (Rate Limited)", 
        "queue_target": QUEUE_API_URL,
        "rate_limit": "5 requests per minute per IP"
    })

# --- Main Endpoint: Handle new pulse requests from the frontend ---
# Apply a specific rate limit to this critical endpoint.
@app.route('/api/request/new', methods=['POST'])
@limiter.limit("5 per minute", override_defaults=False) 
def new_request():
    """
    Receives a new pulse request from the web frontend.
    1. Validates input.
    2. Checks the current queue size (soft limit).
    3. Pushes the request data to the Queue API.
    """
    data = request.get_json()
    name = data.get('name')
    country = data.get('country')

    # Input Validation
    if not name or not country:
        # Note: Flask-Limiter will handle HTTP 429 errors first if the rate limit is hit.
        return jsonify({"message": "Missing 'name' or 'country' field in request payload."}), 400

    try:
        # 1. Check Queue Status for Soft Limiting (Queue Depth Check)
        status_response = requests.get(f"{QUEUE_API_URL}/status")
        status_response.raise_for_status() 
        current_queue_size = status_response.json().get('queue_size', 0)

        if current_queue_size >= MAX_QUEUE_SIZE:
            # Soft Limit Exceeded (Queue Full)
            return jsonify({
                "message": f"Queue is full. Current size is {current_queue_size}. Please try again later.",
                "queue_size": current_queue_size
            }), 429 

        # 2. Push valid request to the Queue API
        add_response = requests.post(f"{QUEUE_API_URL}/add", json=data)
        add_response.raise_for_status()

        # The new size is the client's position in the queue
        new_queue_size = current_queue_size + 1 
        
        return jsonify({
            "message": f"Pulse request accepted for {name} from {country}.",
            "queue_size": new_queue_size
        }), 200

    except requests.exceptions.ConnectionError:
        return jsonify({"message": "Error: Cannot connect to Queue API at 127.0.0.1:5001."}), 503
    except requests.exceptions.RequestException as e:
        return jsonify({"message": f"An error occurred during queue interaction: {e}"}), 500
    except Exception as e:
        return jsonify({"message": f"An unexpected server error occurred: {e}"}), 500


if __name__ == '__main__':
    # Running on port 5000 as configured in the frontend (API_BASE_URL)
    app.run(port=5000)