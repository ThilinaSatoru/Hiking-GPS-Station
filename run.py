import subprocess
import logging

# Configure logging
log_filename = "run_scripts.log"
logging.basicConfig(filename=log_filename, level=logging.INFO,
                    format='%(asctime)s - %(levelname)s - %(message)s')


def run_script_in_new_window(script_name):
    try:
        logging.info(f"Starting {script_name} in a new window")
        # Open a new command window and run the script
        subprocess.Popen(f'start cmd /k python {script_name}', shell=True)
        logging.info(f"{script_name} is running in a new window.")
    except Exception as e:
        logging.error(f"Error running {script_name}: {str(e)}")


# List of scripts to run
scripts = ['./server/serial_communication.py']

for script in scripts:
    # run_script(script)
    run_script_in_new_window(script)
