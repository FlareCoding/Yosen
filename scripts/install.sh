#!/bin/bash
cd ..

# Check to make sure the script has root privileges
if [[ $EUID -ne 0 ]]; then
   echo "This script must be run as root" 
   exit 1
fi

# Retrieve the full binary path
bin_rel_path="build/bin/$(uname)/Release/"
bin_path=$(cd "$(dirname "$bin_rel_path")"; pwd)/$(basename "$bin_rel_path")

# Get the installation path
install_path="/usr/local/bin/yosen_lang/"

if [ ! -d $install_path ]; then
  sudo mkdir -p $install_path;
fi

# Copy all the binary files and
# modules into the installation directory.
sudo cp -a "$bin_path/." $install_path

# Create a hard symlink to the installed binaries
symlink_path="/usr/local/bin/yosen"
sudo ln "$install_path/yosen_lang" $symlink_path

echo "[*] Successfully installed Yosen!"
