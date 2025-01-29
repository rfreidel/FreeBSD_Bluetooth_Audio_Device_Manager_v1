#!/bin/sh

# Install necessary packages
sudo pkg install -y lang/python3 devel/py-qt5 x11-toolkits/py-tkinter

# Install Python modules
pip3 install pyqt5

# Run the application
python3 bluetooth_manager.py
