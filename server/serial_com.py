import serial
import json
import time
import sys
from rich import print, print_json
import logging
from rich.logging import RichHandler
from rich.pretty import pprint
from firebase import initialize_firebase
from firebase_admin import db
import logging
from rich.logging import RichHandler

# Configure logging with Rich handler
logging.basicConfig(
    level=logging.INFO,
    format="%(asctime)s - %(message)s",
    datefmt="%Y-%m-%d %H:%M:%S",
    handlers=[RichHandler(rich_tracebacks=False, show_path=False)]
)
log = logging.getLogger("rich")

COM_PORT = 'COM8'
BAUD_RATE = 115200
MAX_RETRIES = 1
RETRY_DELAY = 5  # seconds
# Global serial connection object
ser = None


def read_serial_data():
    global ser
    retries = 0
    while retries < MAX_RETRIES:
        try:
            with serial.Serial(COM_PORT, BAUD_RATE, timeout=1) as ser:
                log.info(f"Connected to {COM_PORT}")

                while True:
                    line = ser.readline().decode('utf-8', errors='ignore').strip()
                    if line:
                        log.debug(f"Raw data: {line}")
                        try:
                            # Parse JSON data
                            data = json.loads(line)

                            # Validate and process only if "type" is "data"
                            if data.get("type") == "data":
                                required_keys = {
                                    "type", "accepted", "station", "nodeId",
                                    "lat", "lng", "emergency", "emergencyDuration", "battery"
                                }
                                if required_keys.issubset(data.keys()):
                                    # Process valid data
                                    print_json(data=data)
                                    post_station_data(data)

                                else:
                                    log.warning(f"Missing required keys in: {data}")
                            else:
                                log.debug(f"Ignored non-data message: {data}")

                        except json.JSONDecodeError:
                            log.warning(f"Invalid JSON format: {line}")

                    time.sleep(0.1)

        except KeyboardInterrupt:
            log.info("Program terminated by user")
            break

        except serial.SerialException:
            log.error(f"Cannot connect to {COM_PORT}. Please check the connection")
            retries += 1
            if retries < MAX_RETRIES:
                log.info(f"Retrying in 5 seconds... (Attempt {retries}/{MAX_RETRIES})")
                time.sleep(5)
            else:
                log.error("Max retries reached. Exiting.")
                break

        except Exception as e:
            log.error(f"Unexpected error: {str(e)}")
            break


def post_station_data(data):
    try:
        if not data:
            log.warning("No data provided")
            return {'error': 'No data provided'}, 400

        if "type" in data and data["type"] == "error":
            log.error(f"Error from station: {data['message']}")
            return {'error': data["message"]}, 400

        required_keys = {"station", "type", "accepted", "emergency", "nodeId", "lat", "lng"}
        missing_keys = required_keys - data.keys()
        if missing_keys:
            log.info(data.get("accepted"))
            log.error(f"Missing keys in data: {missing_keys}")
            return {"Missing keys in data: {missing_keys}"}, 400

        # Handle status Main
        if all(key in data for key in ["station", "online"]):
            # offline Main
            if data.get('station') == "main":
                if data.get('online') is False:
                    try:
                        ref = db.reference('stations/main')
                        ref.update({
                            "station": "main",
                            'online': False
                        })
                        log.info("Main station marked as Offline")
                    except Exception as e:
                        log.error(f"Failed to update offline status: {str(e)}")
                        raise
                # online Main    
                else:
                    try:
                        ref = db.reference('stations/main')
                        ref.update({
                            "station": "main",
                            'online': True
                        })
                        log.info("Main station marked as Online")
                    except Exception as e:
                        log.error(f"Failed to update online status: {str(e)}")
                        raise

        # Handle Status Station
        if all(key in data for key in ["station", "online", "type"]):
            # Offline Station
            if data.get('type') == "status" and data.get('online') is False:
                try:
                    ref = db.reference(f'stations/{data["station"]}')
                    ref.update({
                        'online': False,
                        'lastUpdated': {'.sv': 'timestamp'}
                    })
                    log.info(f"Station {data['station']} marked as offline")
                except Exception as e:
                    log.error(f"Failed to update offline status: {str(e)}")
                    raise
            # Online Station
            else:
                try:
                    ref = db.reference(f'stations/{data["station"]}')
                    ref.update({
                        'online': True,
                        'accepted': "false",
                        'lastUpdated': {'.sv': 'timestamp'}
                    })
                    log.info(f"Station {data['station']} marked as online")
                except Exception as e:
                    log.error(f"Failed to update online status: {str(e)}")
                    raise

        if data.get("accepted") is False:
            try:
                log.warning(f'WAITING FOR ACCEPT...... : {data.get("accepted")}')
                ref = db.reference(f'stations/{data["station"]}')
                accepted = ref.child('accepted').get()
                if accepted == "true":
                    send_response_to_esp32("true")
                    ref.update({"accepted": "false"})
                    log.info(f'ACCEPTED!: {True}')
            except Exception as e:
                log.error(f"Failed to accept data: {str(e)}")
                raise

        # Handle emergency situation
        if data.get('emergency') in (1, 2, 3):
            log.warning(f'Emergency Type: {data.get('emergency')}')
            try:
                ref = db.reference(f'stations/{data["station"]}')
                station_data = {
                    'online': True,
                    'nodeId': data['nodeId'],
                    'lat': data['lat'],
                    'lng': data['lng'],
                    'emergency': data['emergency'],
                    'lastUpdated': {'.sv': 'timestamp'}
                }
                ref.update(station_data)
                log.info(f"Emergency data updated for station {data['station']}")
            except Exception as e:
                log.error(f"Failed to update emergency data: {str(e)}")
                raise
        else:
            log.info(f"Non-emergency data received from station {data['station']}")
            return {
                'message': f'No emergency for station {data["station"]}',
                'station': data['station']
            }, 200

    except Exception as e:
        log.error(f"Error processing data: {str(e)}")
        return {'error': 'Internal server error'}, 500


def send_response_to_esp32(response):
    """
    Sends a response back to the ESP32-S3 over the serial connection.
    """
    global ser
    if ser is not None and ser.is_open:
        try:
            ser.write(f"{response}\n".encode('utf-8'))
            log.info(f"Sent response to ESP32-S3: {response}")
            return "true"
        except serial.SerialException as e:
            log.error(f"Failed to send response to ESP32-S3: {str(e)}")
            return "false"
    else:
        log.error("Serial connection is not open. Cannot send response.")
        return False


if __name__ == "__main__":
    initialize_firebase()
    read_serial_data()
