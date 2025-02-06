import os
import firebase_admin
from firebase_admin import credentials
from firebase_admin import firestore
import logging
from rich.logging import RichHandler
from dotenv import load_dotenv

# Load environment variables from .env file
load_dotenv()

# Configure logging
logging.basicConfig(
    level=logging.INFO,
    format="%(asctime)s - %(message)s",
    datefmt="%Y-%m-%d %H:%M:%S",
    handlers=[RichHandler(rich_tracebacks=False, show_path=False)]
)
log = logging.getLogger("rich")

# Path to Firebase service account key
cred_path = os.getenv("FIREBASE_SERVICE_ACCOUNT_FILE")
# Firebase Realtime Database URL
database_url = os.getenv("FIREBASE_REALTIME_DATABASE")


def initialize_firebase():
    """
    Initialize Firebase Admin SDK.
    """
    try:
        # Check if Firebase app is already initialized
        if firebase_admin._apps:
            log.info("Firebase is already initialized.")
            return True

        # Initialize Firebase
        cred = credentials.Certificate(cred_path)
        firebase_admin.initialize_app(cred, {
            'databaseURL': database_url
        })
        log.info("Firebase initialized successfully")
        return True
    except Exception as e:
        log.error(f"Firebase initialization failed: {str(e)}")
        return False


def get_device_token():
    try:
        log.info("Fetching Device Token.")
        db = firestore.client()
        doc = db.collection("deviceConfig").document('139D').get()
        if doc.exists:
            return doc.to_dict().get('token')
        else:
            log.error('Device Token not Found!')

    except Exception as e:
        log.error(f"Device Token Fetch Failed : ${e}")
