#!/bin/bash

# Check to make sure the script has root privileges
if [[ $EUID -ne 0 ]]; then
   echo "This script must be run as root" 
   exit 1
fi

# Remove all yosen associated files from the installed directory
sudo rm -rf /usr/local/bin/yosen*

echo "[*] Successfully uninstalled Yosen!"
