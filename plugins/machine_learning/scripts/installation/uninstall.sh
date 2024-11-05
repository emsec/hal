#!/bin/bash

# Stop on the first sign of trouble
set -e

echo "Removing all NVIDIA drivers and CUDA packages..."

# Remove the NVIDIA driver
sudo apt-get purge -y '*nvidia*'

# Remove CUDA Toolkit and associated packages
sudo apt-get purge -y '*cuda*'
sudo apt-get purge -y '*cublas*' '*cufft*' '*curand*' '*cusolver*' '*cusparse*' '*npp*' '*nvjpeg*'

# Autoremove remaining dependencies
sudo apt-get autoremove -y

# Remove any remaining NVIDIA and CUDA directories
sudo rm -rf /usr/local/cuda*

echo "Uninstallation complete. Please reboot the system."

