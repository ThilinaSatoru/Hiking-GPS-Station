import serial
import json
import time
import sys
import requests
from rich import print, print_json
import logging
from rich.logging import RichHandler
from rich.pretty import pprint
log = logging.getLogger("rich")

COM_PORT = 'COM8'  # Change this if your ESP32-S3 is on a different port
BAUD_RATE = 115200
MAX_RETRIES = 5
RETRY_DELAY = 5  # seconds
API_ENDPOINT = 'http://localhost:5000/api/esp'

def post_json_data(data):
    headers = {
        'Content-Type': 'application/json',
        'Accept': 'application/json',
    }

    try:
        response = requests.post(API_ENDPOINT, json=data, headers=headers)
        if response.status_code == 200:
            print("Data posted successfully.")
        else:
            log.exception(f"Failed to post data. Status code: {response.status_code}")
            print(f"Response: {response.text}")
    except requests.exceptions.RequestException as e:
        log.exception(f"Error while sending POST request: {e}")

def read_serial_data():
    retries = 0
    while retries < MAX_RETRIES:
        try:
            with serial.Serial(COM_PORT, BAUD_RATE, timeout=1) as ser:
                print(f"Connected to {COM_PORT}")
                
                while True:
                    try:
                        line = ser.readline().decode('utf-8', errors='ignore').strip()
                        
                        if line:
                            try:
                                data = json.loads(line)

                                print_json(data=data)
                                post_json_data(data)

                            except json.JSONDecodeError:
                                # If it's not JSON, print the raw line
                                print(f"Raw data: {line}")
                    
                    except UnicodeDecodeError:
                        # Ignore decoding errors and continue
                        pass
                    
                    time.sleep(0.1)
        except KeyboardInterrupt:
            sys.exit(0)    
        except serial.SerialException as e:
            log.exception(f"Error: {e}")
            log.exception(f"Make sure the ESP32-S3 is connected to {COM_PORT} and no other program is using it.")
            retries += 1
            if retries < MAX_RETRIES:
                log.exception(f"Retrying in {RETRY_DELAY} seconds... (Attempt {retries}/{MAX_RETRIES})")
                time.sleep(RETRY_DELAY)
            else:
                log.exception("Max retries reached. Exiting.")
                break
        except Exception:
            log.exception("unable print!")

if __name__ == "__main__":
    read_serial_data()