# FreeBSD_Bluetooth_Audio_Device_Manager_v1

# project built with c++

A pleasant GUI to control Bluetooth on FreeBSD wayland xorg compatible, simple to configure, 
each step is logged (I hope). 

The was built, and compiled on FreeBSD 14.2-p1 using latest packagebase 

Upon initial usage `setup.sh`, will check for gtk3, and you must have c++ to compile this

This software is under development and as such may not work out of the box so to speak, but will if you scoll down to where the connection sequence is, alter it to match your device

 # Note
 
 Your user account needs to have hccontrol permissions for this to ever work

# Uninstall

To uninstall this project's changes to your system, simply uninstall the package setup_and_run.sh installed, and delete the directory the project is in
