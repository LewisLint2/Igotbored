#!/bin/bash
# install.sh

# 1. Create the Rat Script
cat << 'EOF' > /rat.sh
#!/bin/bash
# Get the current terminal width
WIDTH=$(tput lines)
# Clear the screen so it's JUST RAT
clear
# Print the glorious beast
jp2a --colors --width=$WIDTH /rat.jpeg
EOF

chmod +x /rat.sh

# 2. Create the Service
cat << 'EOF' > /etc/systemd/system/rat.service
[Unit]
Description=The Great Boot Rat
DefaultDependencies=no
After=local-fs.target
Before=sysinit.target
Conflicts=display-manager.service

[Service]
Type=simple
StandardInput=tty
StandardOutput=tty
TTYPath=/dev/tty1
ExecStart=/usr/local/bin/rat.sh

[Install]
WantedBy=sysinit.target
EOF

# 3. Finalize
systemctl daemon-reload
systemctl enable rat.service

echo "The Rat is prepared for the next boot."