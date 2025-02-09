#!/bin/sh

# Install necessary packages
sudo pkg install -y gtk3

# Compile the C++ application
c++ bluetooth_manager.cpp -o bluetooth_manager `pkg-config --cflags --libs gtk+-3.0`

# Run the application
./bluetooth_manager
