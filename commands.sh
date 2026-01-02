#################################################
# Install docker (commands from docker docs)
#################################################

# Add Docker's official GPG key:
sudo apt update
sudo apt install ca-certificates curl
sudo install -m 0755 -d /etc/apt/keyrings
sudo curl -fsSL https://download.docker.com/linux/debian/gpg -o /etc/apt/keyrings/docker.asc
sudo chmod a+r /etc/apt/keyrings/docker.asc

# Add the repository to Apt sources:
sudo tee /etc/apt/sources.list.d/docker.sources <<EOF
Types: deb
URIs: https://download.docker.com/linux/debian
Suites: $(. /etc/os-release && echo "$VERSION_CODENAME")
Components: stable
Signed-By: /etc/apt/keyrings/docker.asc
EOF
sudo apt update

sudo apt install docker-ce docker-ce-cli containerd.io docker-buildx-plugin docker-compose-plugin

##################################################
# Create the dirs and copy the files
##################################################

sudo mkdir /docker-compose
sudo curl -sL https://raw.githubusercontent.com/randomuser149/homelab/main/gcloud/docker-compose/docker-compose.yml -o /docker-compose/docker-compose.yml
sudo curl -sL https://raw.githubusercontent.com/randomuser149/homelab/main/gcloud/docker-compose/Caddyfile -o /docker-compose/Caddyfile
# replace [domain] & [email] with your own
sudo vim /docker-compose/Caddyfile

##################################################
# Start the docker containers
##################################################

docker compose -f /docker-compose/docker-compose.yml up -d

##################################################
# Edit your firewalls
##################################################

sudo apt install ufw
sudo ufw enable
sudo ufw allow 80/tcp
sudo ufw allow 443/tcp
sudo ufw allow [ssh port]/tcp
sudo ufw reload

##################################################
# Move the default SSH port
#
# IMPORTANT: DO NOT CLOSE YOUR CURRENT SSH SESSION BEFORE YOU VALIDATED THE NEW ONE WORKS
#
# Note that if you follow through then you have to copy the private keys from your Cloud shell and save them to your local machine
# Cloud Shell doesn't allow custom ssh ports, so that's why you have to 'abandon' it if you change the ports
# The key is located at ~/.ssh/google_compute_engine
# Download and move this key to C:\Users\[username]\.ssh\
# Run these two commands before your first log in in PowerShell
# icacls $env:USERPROFILE\.ssh\google_compute_engine /inheritance:r
# icacls $env:USERPROFILE\.ssh\google_compute_engine /grant:r "$($env:USERNAME):(R)"
# Use the following command to log in from PowerShell
# ssh -i $env:USERPROFILE\.ssh\google_compute_engine -p [ssh port] [linux username]@[public ip]
##################################################

sudo vim /etc/ssh/sshd_config
# Uncomment the Port 22 line, and replace 22 with your own choice.
sudo systemctl restart sshd

