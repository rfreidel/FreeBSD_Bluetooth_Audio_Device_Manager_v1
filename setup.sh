#!/bin/sh

# Function to install necessary packages and build the application
install_and_build() {
    # Install necessary packages
    sudo pkg install -y gtk3

    # Create build directory and navigate into it
    mkdir -p build
    cd build

    # Run CMake and make to build the application
    cmake ..
    make
}

# Function to run the application
run_application() {
    # Navigate to the build directory and run the application
    cd build
    ./BluetoothManager
}

# Function to uninstall the application
uninstall_application() {
    # Navigate to the build directory and run make uninstall
    cd build
    sudo make uninstall

    # Remove the build directory
    cd ..
    rm -rf build
}

# Main script logic
case "$1" in
    install)
        install_and_build
        ;;
    run)
        run_application
        ;;
    uninstall)
        uninstall_application
        ;;
    *)
        echo "Usage: $0 {install|run|uninstall}"
        exit 1
esac