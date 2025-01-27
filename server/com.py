import serial
import json
import time
import sys
import requests
from rich import print, print_json
import logging
from rich.logging import RichHandler
from rich.pretty import pprint

# Configure logging with Rich handler
logging.basicConfig(
    level=logging.INFO,
    format="%(message)s",
    handlers=[RichHandler(rich_tracebacks=False, show_path=False)]
)
log = logging.getLogger("rich")

COM_PORT = 'COM8'
BAUD_RATE = 115200
MAX_RETRIES = 1
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
            log.info("Data posted to Server successfully")
            return True
        else:
            log.error(f"Failed to post data. Status code: {response.status_code}")
            return False
    except requests.exceptions.ConnectionError:
        log.error("Failed to connect to server. Please check if the server is running")
    except requests.exceptions.RequestException as e:
        log.error(f"Error posting data: {str(e)}")


def read_serial_data():
    retries = 0
    while retries < MAX_RETRIES:
        try:
            with serial.Serial(COM_PORT, BAUD_RATE, timeout=1) as ser:
                log.info(f"Connected to {COM_PORT}")
                connection_data = {
                    "station": "main",
                    "online": True
                }
                post_json_data(connection_data)
                log.info("Posted Connection.")
                
                while True:
                    line = ser.readline().decode('utf-8', errors='ignore').strip()
                    if line:
                        try:
                            data = json.loads(line)
                            print_json(data=data)
                            post_json_data(data)
                            response = "true"
                            # Send the boolean response back to the ESP32-S3
                            ser.write(f"{response}\n".encode('utf-8'))
                            log.info(f"Sent response to ESP32-S3: {response}")

                        except json.JSONDecodeError:
                            log.warning(f"Received invalid JSON data: {line}")
                    
                    time.sleep(0.1)

        except KeyboardInterrupt:
            log.info("Program terminated by user")
            sys.exit(0)
            
        except serial.SerialException:
            log.error(f"Cannot connect to {COM_PORT}. Please check the connection")
            log.info(f"Make sure the ESP32-S3 is connected to {COM_PORT} and no other program is using it")

            disconnection_data = {
                "station": "main",
                "online": False
            }
            post_json_data(disconnection_data)
            log.info("Posted disconnection data")
            
            retries += 1
            if retries < MAX_RETRIES:
                log.info(f"Retrying in {RETRY_DELAY} seconds... (Attempt {retries}/{MAX_RETRIES})")
                time.sleep(RETRY_DELAY)
            else:
                log.error("Max retries reached. Exiting")
                break

        except UnicodeDecodeError:
            log.warning("Received invalid data encoding, skipping")
            continue
            
        except Exception as e:
            log.error(f"Unexpected error: {str(e)}")
            break

if __name__ == "__main__":
    read_serial_data()