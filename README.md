# FreeBSD_Bluetooth_Audio_Device_Manager_v1

# Use only -patch-1

A pleasant GUI to control Bluetooth on FreeBSD wayland xorg compatible, simple to configure, 
each step is logged (I hope).

Upon initial usage `setup_and_run.sh`, will install several modules, you may not need these, if you currently work in python venv 
you should be able to use this script, if however modules were installed, re-start the script, it should then function
## Installation
If you currently have python3 installed and can configure python venv to run it in, I would recommend this. All you need to do is download 
`bluetooth_manager.py` and if you have at some point in time prior to executing `python3 bluetooth_manager.py` had connected to a device 
via Bluetooth, you will be presented with a GUI showing available UBT devices
# Subnote
However, you may encounter an error, the project has been redesiged to log each step, notify user of where error happened

This software is under development and as such may not work out of the box so to speak, but will if you scoll down to where the connection sequence is, and recall how you set things up manually to connect to your bluetooth device the last time, this is python and you must use python script language, if you have difficulty, contact me via the reddit post. This software will enable you to maintain eternal youth as I have.
# Note

Your user account must be configured with sudo hccontrol permissions for this to ever work

# Uninstall
To uninstall this project's changes to your system, simply uninstall the package setup_and_run.sh installed, and delete the directory the project is in
