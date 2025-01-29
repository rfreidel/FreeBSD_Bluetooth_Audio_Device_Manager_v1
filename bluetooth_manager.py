import sys
import subprocess
import logging
import tkinter as tk
from tkinter import messagebox, Listbox, Button, END

# Configure logging to user-level log file
logging.basicConfig(filename='bluetooth_manager.log', level=logging.DEBUG, format='%(asctime)s %(message)s')

def log_action(action):
    logging.info(action)

def log_error(error):
    logging.error(error)

def is_service_running(service_name):
    result = subprocess.run(['service', service_name, 'status'], capture_output=True, text=True)
    return "is running" in result.stdout

class BluetoothManager(tk.Tk):
    def __init__(self):
        super().__init__()
        self.title("Bluetooth Manager")
        self.geometry("400x300")

        self.scan_button = Button(self, text="Scan for Devices", command=self.scan_devices)
        self.scan_button.pack(pady=10)

        self.device_listbox = Listbox(self)
        self.device_listbox.pack(pady=10, fill=tk.BOTH, expand=True)

        self.connect_button = Button(self, text="Connect", command=self.connect_device)
        self.connect_button.pack(pady=10)

    def scan_devices(self):
        self.device_listbox.delete(0, END)
        try:
            # Check if hcsecd service is running
            if not is_service_running('hcsecd'):
                result = subprocess.run(['sudo', 'service', 'hcsecd', 'onestart'], capture_output=True, text=True)
                if result.returncode != 0:
                    raise subprocess.CalledProcessError(result.returncode, result.args, output=result.stdout, stderr=result.stderr)
                log_action("Started hcsecd service")
            else:
                log_action("hcsecd service already running")

            # Start Bluetooth service
            result = subprocess.run(['sudo', 'service', 'bluetooth', 'onestart', 'ubt0'], capture_output=True, text=True)
            if result.returncode != 0:
                raise subprocess.CalledProcessError(result.returncode, result.args, output=result.stdout, stderr=result.stderr)
            log_action(f"Bluetooth service output: {result.stdout}")

            # List Bluetooth devices
            result = subprocess.run(['sudo', 'hccontrol', '-n', 'ubt0hci', 'inquiry'], capture_output=True, text=True, check=True)
            devices = result.stdout.splitlines()
            for device in devices:
                self.device_listbox.insert(END, device)
            log_action("Scanned for Bluetooth devices")
        except subprocess.CalledProcessError as e:
            stderr_output = e.stderr.strip() if e.stderr else "Unknown error"
            stdout_output = e.stdout.strip() if e.stdout else "No standard output"
            error_message = f"Failed to scan devices: {stderr_output}. Stdout: {stdout_output}"
            print(error_message)  # Print error to terminal
            messagebox.showerror("Error", error_message)
            log_error(error_message)

    def connect_device(self):
        selected_device = self.device_listbox.get(tk.ACTIVE)
        if selected_device:
            try:
                subprocess.run(['sudo', 'hccontrol', '-n', 'ubt0hci', 'create_connection', selected_device], check=True, capture_output=True, text=True)
                messagebox.showinfo("Success", f"Connected to {selected_device}")
                log_action(f"Connected to {selected_device}")
            except subprocess.CalledProcessError as e:
                stderr_output = e.stderr.strip() if e.stderr else "Unknown error"
                stdout_output = e.stdout.strip() if e.stdout else "No standard output"
                error_message = f"Failed to connect to device: {stderr_output}. Stdout: {stdout_output}"
                print(error_message)  # Print error to terminal
                messagebox.showerror("Error", error_message)
                log_error(error_message)
        else:
            messagebox.showwarning("Warning", "No device selected")
            log_action("No device selected for connection")

if __name__ == '__main__':
    log_action("Bluetooth Manager started")
    app = BluetoothManager()
    app.mainloop()
