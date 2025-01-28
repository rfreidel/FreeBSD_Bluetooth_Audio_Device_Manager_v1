#!/bin/sh

# Install necessary packages
sudo pkg install -y python3 py39-qt5 py39-tkinter

# Install Python modules
pip3 install pyqt5

# Run the application
python3 bluetooth_manager.py
