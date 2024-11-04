from flask import Flask, request, jsonify
import firebase_admin
from firebase_admin import credentials
from firebase_admin import db
import os
import logging
from rich.logging import RichHandler
from datetime import datetime

# Configure logging
logging.basicConfig(
    level=logging.INFO,
    format="%(asctime)s - %(message)s",
    datefmt="%Y-%m-%d %H:%M:%S",
    handlers=[RichHandler(rich_tracebacks=False, show_path=False)]
)
log = logging.getLogger("rich")

app = Flask(__name__)

# Initialize Firebase Admin SDK
try:
    cred = credentials.Certificate('hikinggpsmesh-firebase-adminsdk-372xn-f4810366d7.json')
    firebase_admin.initialize_app(cred, {
        'databaseURL': 'https://hikinggpsmesh-default-rtdb.asia-southeast1.firebasedatabase.app'
    })
    log.info("Firebase initialized successfully")
except Exception as e:
    log.error(f"Firebase initialization failed: {str(e)}")
    raise

@app.route('/api/esp', methods=['POST'])
def post_station_data():
    try:
        data = request.get_json()
        
        if not data:
            log.warning("POST request received with no data")
            return jsonify({'error': 'No data provided'}), 400


        # Handle status Main
        if all(key in data for key in ["station", "online"]):
            # offline Main
            if data.get('station') == "main":
                if data.get('online') is False:
                    try:
                        ref = db.reference('stations/main')
                        ref.set({
                            "station": "main",
                            'online': False
                        })
                        log.info("Main station marked as Offline")
                        return jsonify({
                            'message': 'Station marked as offline',
                            'station': data['station']
                        }), 200
                    except Exception as e:
                        log.error(f"Failed to update offline status: {str(e)}")
                        raise
                # online Main    
                else:
                    try:
                        ref = db.reference('stations/main')
                        ref.set({
                            "station": "main",
                            'online': True
                        })
                        log.info("Main station marked as Online")
                        return jsonify({
                            'message': 'Station marked as offline',
                            'station': data['station']
                        }), 200
                    except Exception as e:
                        log.error(f"Failed to update offline status: {str(e)}")
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
                    log.info("Main station marked as offline")
                    return jsonify({
                        'message': 'Station marked as offline',
                        'station': data['station']
                    }), 200
                except Exception as e:
                    log.error(f"Failed to update offline status: {str(e)}")
                    raise
            # Online Station
            else:
                try:
                    ref = db.reference(f'stations/{data["station"]}')
                    ref.update({
                        'online': True,
                        'lastUpdated': {'.sv': 'timestamp'}
                    })
                    log.info("Main station marked as offline")
                    return jsonify({
                        'message': 'Station marked as offline',
                        'station': data['station']
                    }), 200
                except Exception as e:
                    log.error(f"Failed to update offline status: {str(e)}")
                    raise

        # Handle emergency situation ######################################################
        if data.get('emergency') is True:
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
                ref.set(station_data)
                log.info(f"Emergency data updated for station {data['station']}")
                return jsonify({
                    'message': f'Emergency data updated for station {data["station"]}'
                }), 200
            except Exception as e:
                log.error(f"Failed to update emergency data: {str(e)}")
                raise
        
        log.info(f"Non-emergency data received from station {data['station']}")
        return jsonify({
            'message': f'No emergency for station {data["station"]}',
            'station': data['station']
        }), 200
    # #######################################################################################
    
    except Exception as e:
        log.error(f"Error processing POST request: {str(e)}")
        return jsonify({'error': 'Internal server error'}), 500

@app.route('/api/esp/<int:station>', methods=['GET'])
def get_station_data(station):
    try:
        ref = db.reference(f'stations/{station}')
        data = ref.get()
        
        if data is None:
            log.warning(f"Station {station} not found")
            return jsonify({'error': 'Station not found'}), 404
            
        log.info(f"Retrieved data for station {station}")
        return jsonify(data), 200
    
    except Exception as e:
        log.error(f"Error retrieving station {station} data: {str(e)}")
        return jsonify({'error': 'Internal server error'}), 500

@app.route('/api/esp', methods=['GET'])
def get_all_stations():
    try:
        ref = db.reference('stations')
        data = ref.get()
        
        if data is None:
            log.info("No stations found in database")
            data = {}
        else:
            log.info(f"Retrieved data for {len(data)} stations")
            
        return jsonify(data), 200
    
    except Exception as e:
        log.error(f"Error retrieving all stations data: {str(e)}")
        return jsonify({'error': 'Internal server error'}), 500



# Error handlers
@app.errorhandler(404)
def not_found_error(error):
    log.warning(f"404 error: {request.url}")
    return jsonify({'error': 'Resource not found'}), 404

@app.errorhandler(500)
def internal_error(error):
    log.error(f"500 error: {str(error)}")
    return jsonify({'error': 'Internal server error'}), 500

if __name__ == '__main__':
    log.info("Starting Flask server on port 5000...")
    app.run(debug=True, port=5000)