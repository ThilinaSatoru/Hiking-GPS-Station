from flask import Flask, request, jsonify
import firebase_admin
from firebase_admin import credentials
from firebase_admin import db
import os

app = Flask(__name__)

# Initialize Firebase Admin SDK
# Replace with your service account key path
cred = credentials.Certificate('hikinggpsmesh-firebase-adminsdk-372xn-f4810366d7.json')
firebase_admin.initialize_app(cred, {
    'databaseURL': 'https://hikinggpsmesh-default-rtdb.asia-southeast1.firebasedatabase.app'
})

@app.route('/api/esp', methods=['POST'])
def post_station_data():
    try:
        # Get JSON data from request
        data = request.get_json()
        
        if not data:
            return jsonify({'error': 'No data provided'}), 400
        
        # Validate required fields
        required_fields = ['type', 'station', 'nodeId', 'lat', 'lng', 'emergency', 'help']
        for field in required_fields:
            if field not in data:
                return jsonify({'error': f'Missing required field: {field}'}), 400
        
        # Validate data types
        if not isinstance(data['station'], int):
            return jsonify({'error': 'Station must be an integer'}), 400
        if not isinstance(data['lat'], (int, float)):
            return jsonify({'error': 'Latitude must be a number'}), 400
        if not isinstance(data['lng'], (int, float)):
            return jsonify({'error': 'Longitude must be a number'}), 400
        if not isinstance(data['emergency'], bool):
            return jsonify({'error': 'Emergency must be a boolean'}), 400
        if not isinstance(data['help'], bool):
            return jsonify({'error': 'Help must be a boolean'}), 400
        
        if data['help'] == True or data['emergency'] == True:

            # Get a reference to the database
            # Using station number as the key in the 'stations' collection
            ref = db.reference(f'stations/{data["station"]}')
            
            # Update or create station data
            ref.set({
                'type': data['type'],
                'nodeId': data['nodeId'],
                'lat': data['lat'],
                'lng': data['lng'],
                'emergency': data['emergency'],
                'help': data['help'],
                'lastUpdated': {'.sv': 'timestamp'}  # Server timestamp
            })
        
            return jsonify({
                'message': f'Data successfully updated for station {data["station"]}',
                'station': data['station']
            }), 200
        
        else:
            return jsonify({
                'message': f'No Emergency for station {data["station"]}',
                'station': data['station']
            }), 200
    
    except Exception as e:
        return jsonify({'error': str(e)}), 500

@app.route('/api/esp/<int:station>', methods=['GET'])
def get_station_data(station):
    try:
        # Get reference to specific station
        ref = db.reference(f'stations/{station}')
        
        # Retrieve station data
        data = ref.get()
        
        if data is None:
            return jsonify({'error': 'Station not found'}), 404
            
        return jsonify(data), 200
    
    except Exception as e:
        return jsonify({'error': str(e)}), 500

@app.route('/api/esp', methods=['GET'])
def get_all_stations():
    try:
        # Get reference to all stations
        ref = db.reference('stations')
        
        # Retrieve all station data
        data = ref.get()
        
        if data is None:
            data = {}
            
        return jsonify(data), 200
    
    except Exception as e:
        return jsonify({'error': str(e)}), 500

if __name__ == '__main__':
    app.run(debug=True, port=5000)