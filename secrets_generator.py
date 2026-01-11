import secrets
import base64
import sys

# Define the number of random bytes you want. 
# 48 bytes is a good standard length (48 * 8 = 384 bits of entropy).
# Base64 encoding this will result in 64 characters (since every 3 bytes -> 4 characters).
NUM_BYTES = 48 

# 1. Generate cryptographically secure random bytes
random_bytes = secrets.token_bytes(NUM_BYTES)

# 2. Base64 encode the bytes to get a secure, URL-safe string of characters
# We use standard base64 encoding (base64.b64encode) for a clean string.
secret_key_bytes = base64.b64encode(random_bytes)

# 3. Decode to a string for easy use in code
secret_key_str = secret_key_bytes.decode('utf-8')

# 4. Print the key and some metadata
print("--- ESP32 API Secret Key Generator ---")
print(f"Entropy (bits): {NUM_BYTES * 8}")
print(f"Key Length: {len(secret_key_str)} characters")
print("-" * 40)
print(f"🔑 NEW API KEY: {secret_key_str}")
print()
print("-" * 40)