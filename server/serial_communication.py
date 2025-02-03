import os
import serial
import json
import time
import logging
from rich.logging import RichHandler
from firebase_admin import db
from dataclasses import dataclass
from typing import Optional, Set, List
from enum import IntEnum
from notification import send_push_notification
from dotenv import load_dotenv

# Load environment variables from .env file
load_dotenv()

# Configure logging with Rich handler
logging.basicConfig(
    level=logging.INFO,
    format="%(asctime)s - %(message)s",
    datefmt="%Y-%m-%d %H:%M:%S",
    handlers=[RichHandler(rich_tracebacks=False, show_path=False)]
)
log = logging.getLogger("rich")


class EmergencyType(IntEnum):
    NONE = 0
    EMERGENCY = 1
    SECONDARY = 2
    TERTIARY = 3



@dataclass
class StationData:
    station: str
    node_id: str
    lat: float
    lng: float
    emergency: EmergencyType
    battery: float
    accepted: bool = False
    online: bool = True

    def __eq__(self, other):
        """Compare all relevant fields for state changes"""
        if not isinstance(other, StationData):
            return False
        return (
                self.station == other.station and
                self.node_id == other.node_id and
                self.lat == other.lat and
                self.lng == other.lng and
                self.emergency == other.emergency and
                self.battery == other.battery and
                self.accepted == other.accepted and
                self.online == other.online
        )

    def has_significant_changes(self, other) -> bool:
        """
        Check if there are significant changes in emergency-related fields
        Returns True if any critical fields have changed
        """
        if not isinstance(other, StationData):
            return True

        # Define which fields are considered significant for emergency updates
        return (
                self.emergency != other.emergency or
                self.lat != other.lat or
                self.lng != other.lng or
                self.accepted != other.accepted or
                abs(self.battery - other.battery) > 5  # Only update if battery change > 5%
        )


class SerialMonitor:
    REQUIRED_KEYS: Set[str] = {
        "type", "accepted", "station", "nodeId",
        "lat", "lng", "emergency", "emergencyDuration", "battery"
    }

    def __init__(self, port: str, baud_rate: int, max_retries: int = 1, retry_delay: int = 5):
        self.port = port
        self.baud_rate = baud_rate
        self.max_retries = max_retries
        self.retry_delay = retry_delay
        self.ser = None
        self.emergency_data: List[StationData] = []
        self.main_online = False

    def connect(self) -> Optional[serial.Serial]:
        """Establish serial connection with retry mechanism"""
        for attempt in range(self.max_retries):
            try:
                self.ser = serial.Serial(self.port, self.baud_rate, timeout=1)
                log.info(f"Connected to {self.port}")
                return self.ser
            except serial.SerialException:
                self.main_online = False
                log.error(f"Cannot connect to {self.port}. Attempt {attempt + 1}/{self.max_retries}")
                if attempt < self.max_retries - 1:
                    time.sleep(self.retry_delay)
        return None

    def process_data(self, data: dict) -> None:
        """Process validated JSON data"""
        if data.get("type") != "data":
            log.debug(f"Ignored non-data message: {data}")
            return

        if not self.REQUIRED_KEYS.issubset(data.keys()):
            log.warning(f"Missing required keys in: {data}")
            return

        station_data = StationData(
            station=data["station"],
            node_id=data["nodeId"],
            lat=data["lat"],
            lng=data["lng"],
            emergency=EmergencyType(data["emergency"]),
            battery=data["battery"],
            accepted=data["accepted"]
        )

        # self.update_station_status(station_data)
        self.handle_emergency(station_data)

    def update_station_status(self, station_data: StationData) -> None:
        """Update station status in Firebase"""
        try:
            ref = db.reference(f'stations/{station_data.station}')
            ref.update({
                'online': station_data.online,
                'nodeId': station_data.node_id,
                'lat': station_data.lat,
                'lng': station_data.lng,
                'emergency': station_data.emergency,
                'battery': station_data.battery,
                'lastUpdated': {'.sv': 'timestamp'}
            })
        except Exception as e:
            log.error(f"Failed to update station status: {str(e)}")
            raise

    def handle_emergency(self, new_data: StationData) -> None:
        """
        Handle emergency situations with state comparison
        Only updates database when there are significant changes
        """
        # Find existing entry for this station               
        existing_index = next(
            (i for i, data in enumerate(self.emergency_data)
             if data.station == new_data.station),
            None
        )

        if new_data.emergency in (EmergencyType.EMERGENCY, EmergencyType.SECONDARY, EmergencyType.TERTIARY):
            if existing_index is None:
                # New emergency - add to array and update database
                self.emergency_data.append(new_data)
                self._update_emergency_database(new_data)
                self._push_emergency_record(new_data)
                log.warning(f"New emergency Type {new_data.emergency} for station {new_data.station}")

                if send_push_notification(
                    os.getenv("FIREBASE_DEVICE_TOKEN"),
                    "Emergency Alert",
                    f"An emergency type {new_data.emergency} occurred at device {EmergencyType(new_data.emergency).name}."
                ) == 200:
                    self.send_response("true")
                else:
                    log.error(f"Error sending Notification for station: {new_data.station}")
            else:
                # Check if existing emergency has significant changes
                existing_data = self.emergency_data[existing_index]
                if existing_data.has_significant_changes(new_data):
                    self.emergency_data[existing_index] = new_data
                    self._update_emergency_database(new_data)
                    log.warning(f"Updated emergency data for station {new_data.station} due to significant changes")
                else:
                    log.debug(f"No significant changes for station {new_data.station}, skipping update")

        elif new_data.emergency == EmergencyType.NONE and existing_index is not None:
            # Remove emergency and update database
            self.emergency_data.pop(existing_index)
            self._clear_emergency_database(new_data.station)
            log.info(f"Emergency resolved for station {new_data.station}")

    @staticmethod
    def _update_emergency_database(station_data: StationData) -> None:
        """Update emergency information in database"""
        try:
            ref = db.reference(f"stations/{station_data.station}")
            ref.update({
                'nodeId': station_data.node_id,
                'lat': station_data.lat,
                'lng': station_data.lng,
                'emergency': station_data.emergency,
                'battery': station_data.battery,
                'lastUpdated': {'.sv': 'timestamp'}
            })
        except Exception as e:
            log.error(f"Failed to update emergency data: {str(e)}")
            raise

    @staticmethod
    def _push_emergency_record(station_data: StationData) -> None:
        """Record emergency information in database"""
        try:
            # Reference to the station's history sub-path
            ref = db.reference(f"history")
            # Generate a new record with a unique key (Firebase automatically handles timestamps)
            new_entry = ref.push({
                'nodeId': station_data.node_id,
                'station': station_data.station,
                'lat': station_data.lat,
                'lng': station_data.lng,
                'emergency': station_data.emergency,
                'battery': station_data.battery,
                'timestamp': {'.sv': 'timestamp'}  # Firebase server timestamp
            })
            log.info(f"New emergency data saved with key: {new_entry.key}")

        except Exception as e:
            log.error(f"Failed to update emergency data: {str(e)}")
            raise

    @staticmethod
    def _clear_emergency_database(station: str) -> None:
        """Clear emergency information from database"""
        try:
            ref = db.reference(f"stations/{station}")
            ref.update({
                'emergency': EmergencyType.NONE,
                'accepted': True
            })
        except Exception as e:
            log.error(f"Failed to clear emergency data: {str(e)}")
            raise

    def update_main_status(self, online) -> None:
        try:
            ref = db.reference(f"stations/main")
            ref.update({
                'online': online,
            })
        except Exception as e:
            log.error(f"Failed to update data: {str(e)}")
            raise

    def send_response(self, response: str) -> bool:
        """Send response to ESP32"""
        if self.ser and self.ser.is_open:
            try:
                self.ser.write(f"{response}\n".encode('utf-8'))
                log.info(f"Sent response to ESP32: {response}")
                return True
            except serial.SerialException as e:
                log.error(f"Failed to send response: {str(e)}")
        return False

    def run(self) -> None:
        """Main loop for reading serial data"""
        if not self.connect():

            log.error("Failed to establish serial connection")
            return

        try:
            while True:
                line = self.ser.readline().decode('utf-8', errors='ignore').strip()
                self.main_online = True
                self.update_main_status(self.main_online)
                if not line:
                    continue

                try:
                    data = json.loads(line)
                    self.process_data(data)
                except json.JSONDecodeError:
                    log.warning(f"Invalid JSON format: {line}")

                time.sleep(0.1)

        except KeyboardInterrupt:
            self.main_online = False
            log.info("Program terminated by user")
        except serial.SerialException:
            self.main_online = False
            self.update_main_status(self.main_online)
        finally:
            if self.ser:
                self.ser.close()
            self.main_online = False


def main():
    from firebase import initialize_firebase
    initialize_firebase()

    monitor = SerialMonitor(
        port='COM8',
        baud_rate=115200,
        max_retries=2,
        retry_delay=5
    )
    monitor.run()


if __name__ == "__main__":
    main()