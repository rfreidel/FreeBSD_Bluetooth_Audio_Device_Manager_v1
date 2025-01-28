import sys
import subprocess
import logging
import tkinter as tk
from tkinter import messagebox, Listbox, Scrollbar, Button, END

# Configure logging to user-level log file
logging.basicConfig(filename='bluetooth_manager.log', level=logging.INFO, format='%(asctime)s %(message)s')

def log_action(action):
    logging.info(action)

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
            subprocess.run(['sudo', 'service', 'bluetooth', 'start', 'ubt0'], check=True)
            result = subprocess.run(['sudo', 'hccontrol', '-n', 'ubt0hci', 'list'], capture_output=True, text=True, check=True)
            devices = result.stdout.splitlines()
            for device in devices:
                self.device_listbox.insert(END, device)
            log_action("Scanned for Bluetooth devices")
        except subprocess.CalledProcessError as e:
            messagebox.showerror("Error", f"Failed to scan devices: {e}")
            log_action(f"Failed to scan devices: {e}")

    def connect_device(self):
        selected_device = self.device_listbox.get(tk.ACTIVE)
        if selected_device:
            try:
                subprocess.run(['sudo', 'hccontrol', '-n', 'ubt0hci', 'create_connection', selected_device], check=True)
                messagebox.showinfo("Success", f"Connected to {selected_device}")
                log_action(f"Connected to {selected_device}")
            except subprocess.CalledProcessError as e:
                messagebox.showerror("Error", f"Failed to connect to device: {e}")
                log_action(f"Failed to connect to device: {e}")
        else:
            messagebox.showwarning("Warning", "No device selected")
            log_action("No device selected for connection")

if __name__ == '__main__':
    log_action("Bluetooth Manager started")
    app = BluetoothManager()
    app.mainloop()
