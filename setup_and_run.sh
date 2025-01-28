#!/bin/sh

# Install necessary packages
sudo pkg install -y python3 py38-qt5 

# Install Python modules
pip3 install pyqt5

# Run the application
python3 bluetooth_manager.py
