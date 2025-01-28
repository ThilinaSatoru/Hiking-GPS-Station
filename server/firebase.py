import firebase_admin
from firebase_admin import credentials
import logging
from rich.logging import RichHandler

# Configure logging
logging.basicConfig(
    level=logging.INFO,
    format="%(asctime)s - %(message)s",
    datefmt="%Y-%m-%d %H:%M:%S",
    handlers=[RichHandler(rich_tracebacks=False, show_path=False)]
)
log = logging.getLogger("rich")


# Path to Firebase service account key
cred_path = 'peakfind-6af89-firebase-adminsdk-fefm9-c0d373b2e0.json'
# Firebase Realtime Database URL
database_url = 'https://peakfind-6af89-default-rtdb.asia-southeast1.firebasedatabase.app'


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