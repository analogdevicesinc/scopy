#!/bin/bash
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
