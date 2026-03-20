#!/bin/bash

# Copyright (c) 2025 Analog Devices Inc.
#
# This file is part of Scopy
# (see https://www.github.com/analogdevicesinc/scopy).
#
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program. If not, see <https://www.gnu.org/licenses/>.

# Install Docker Engine on Ubuntu 24.04
# Run this script with: sudo bash ci/install_docker.sh
# After installation, log out and back in for group changes to take effect.

set -euo pipefail

echo "=== Installing Docker Engine on Ubuntu 24.04 ==="

# Remove any old/conflicting packages
echo "--- Removing old Docker packages (if any) ---"
for pkg in docker.io docker-doc docker-compose docker-compose-v2 podman-docker containerd runc; do
    apt-get remove -y "$pkg" 2>/dev/null || true
done

# Install prerequisites
echo "--- Installing prerequisites ---"
apt-get update
apt-get install -y ca-certificates curl

# Add Docker's official GPG key
echo "--- Adding Docker GPG key ---"
install -m 0755 -d /etc/apt/keyrings
curl -fsSL https://download.docker.com/linux/ubuntu/gpg -o /etc/apt/keyrings/docker.asc
chmod a+r /etc/apt/keyrings/docker.asc

# Add Docker apt repository
echo "--- Adding Docker apt repository ---"
echo \
  "deb [arch=$(dpkg --print-architecture) signed-by=/etc/apt/keyrings/docker.asc] https://download.docker.com/linux/ubuntu \
  $(. /etc/os-release && echo "${VERSION_CODENAME}") stable" | \
  tee /etc/apt/sources.list.d/docker.list > /dev/null

# Install Docker Engine
echo "--- Installing Docker Engine ---"
apt-get update
apt-get install -y docker-ce docker-ce-cli containerd.io docker-buildx-plugin docker-compose-plugin

# Add current user to docker group (so they don't need sudo for docker commands)
REAL_USER="${SUDO_USER:-$USER}"
echo "--- Adding user '$REAL_USER' to docker group ---"
usermod -aG docker "$REAL_USER"

# Start and enable Docker
echo "--- Starting Docker service ---"
systemctl start docker
systemctl enable docker

# Verify installation
echo "--- Verifying installation ---"
docker --version
docker run --rm hello-world

echo ""
echo "=== Docker installed successfully ==="
echo "IMPORTANT: Log out and back in (or run 'newgrp docker') for group changes to take effect."
echo "Then you can run docker commands without sudo."
