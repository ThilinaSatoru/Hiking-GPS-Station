import os
import json
import requests
from google.oauth2 import service_account
from google.auth.transport.requests import Request
from dotenv import load_dotenv

# Load environment variables from .env file
load_dotenv()

# Get variables from environment
SERVICE_ACCOUNT_FILE = os.getenv("FIREBASE_SERVICE_ACCOUNT_FILE")
PROJECT_ID = os.getenv("FIREBASE_PROJECT_ID")

# FCM V1 endpoint
FCM_URL = f"https://fcm.googleapis.com/v1/projects/{PROJECT_ID}/messages:send"


# Authenticate and get an access token
def get_access_token():
    credentials = service_account.Credentials.from_service_account_file(
        SERVICE_ACCOUNT_FILE,
        scopes=["https://www.googleapis.com/auth/cloud-platform"]
    )
    credentials.refresh(Request())
    return credentials.token


# Send push notification using FCM V1 API
def send_push_notification(device_token, title, body):
    access_token = get_access_token()

    headers = {
        "Authorization": f"Bearer {access_token}",
        "Content-Type": "application/json"
    }

    payload = {
        "message": {
            "token": device_token,  # Replace with the device token or use "topic" for topic-based messaging
            "data": {
                "notifee": json.dumps({  # Use `json.dumps` to convert the dictionary to a JSON string
                    "title": title,
                    "body": body,
                    "android": {
                        "channelId": "alerts",
                        "importance": 4,
                    }
                })
            }
        }
    }

    response = requests.post(FCM_URL, headers=headers, json=payload)

    if response.status_code == 200:
        print("Notification sent successfully:", response.json())
        return response.status_code
    else:
        print("Failed to send notification:", response.status_code, response.text)
        return response.status_code
