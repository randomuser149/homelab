# Google Cloud  
This branch contains all files & commands that are and were used on the current Google Cloud VM instance.  
It only runs Caddy and Vaultwarden as of currently.  
Commands used can be found in the [commands.sh](./commands.sh)

## The file structure on the instance:  
```bash
/  
└── docker-compose/  
    ├── docker-compose.yml  
    ├── radicale
    │   ├── config
    │   └── data
    └── Caddyfile
```

# Configuration

### Docker Compose:  
To run all of the other services you have to install Docker Compose. I mean you don't have to, but I find Compose to be the easiest.  
Do the following steps to install Docker Compose:

```
sudo apt update
sudo apt install ca-certificates curl
sudo install -m 0755 -d /etc/apt/keyrings
sudo curl -fsSL https://download.docker.com/linux/debian/gpg -o /etc/apt/keyrings/docker.asc
sudo chmod a+r /etc/apt/keyrings/docker.asc

sudo tee /etc/apt/sources.list.d/docker.sources <<EOF
Types: deb
URIs: https://download.docker.com/linux/debian
Suites: $(. /etc/os-release && echo "$VERSION_CODENAME")
Components: stable
Signed-By: /etc/apt/keyrings/docker.asc
EOF
sudo apt update

sudo apt install docker-ce docker-ce-cli containerd.io docker-buildx-plugin docker-compose-plugin
```

### Starting the containers
To start the containers use the following command:

```
docker compose -f /docker-compose/docker-compose.yml up -d
```
>[!NOTE]
>You have to reissue this command either after every service added to the compose file or at the very end of the setup process.

### Caddy
Caddy is my current reverse proxy of choice. It's simple, easy to set up, and most importantly it just works.
You need to do two things to get it up and running:
- add it to the compose file
- create the Caddyfile

#### Compose file
To create the container for it, add the following chunk into the `docker-compose.yml` file:
```
services:
  caddy:
    image: caddy:latest
    container_name: caddy
    restart: unless-stopped
    ports:
      - "80:80"
      - "443:443"
    volumes:
    # map Caddyfile to its location on host -> path-on-host:path-in-container
      - /docker-compose/Caddyfile:/etc/caddy/Caddyfile
      - caddy_data:/data
      - caddy_config:/config

volumes:
  caddy_data:
  caddy_config:
```
#### Caddyfile
This file can really be placed anywhere, you just have to make sure you update the `docker-compose.yml` accordingly.  
My file is in the `/docker-compose` directory.  
This is the whole file, but relevant snippets can be found under each service.
```
# Vaultwarden
[domain] {
        reverse_proxy vaultwarden:80
        tls [email]
}

# Radicale
[domain] {
        reverse_proxy radicale:5232
        tls [email]
}
```

### Radicale
As another always on service this will run on the cloud too.  
It is an easy to set up CalDAV and CardDAV server.
To install it
- add it to the compose file
- add it to the Caddyfile
- create at least one user account

#### Compose file
To create the container for it, add the following chunk into the `docker-compose.yml` file:
```
services:
  radicale:
      image: ghcr.io/kozea/radicale:3.5.10
      container_name: radicale
      ports:
      - 5232:5232
      volumes:
        - config:/etc/radicale
        - data:/var/lib/radicale

volumes:
  config:
    name: radicale-config
    driver: local
    driver_opts:
      type: none
      o: bind
      # map dir to its location on host
      device: /docker-compose/radicale/config

  data:
    name: radicale-data
    driver: local
    driver_opts:
      type: none
      o: bind
      # map dir to its location on host
      device: /docker-compose/radicale/data
```
#### Caddyfile
To update the Caddyfile, add the following snippets to it:
```
# Radicale
[domain] {
        reverse_proxy radicale:5232
        tls [email]
}
```
#### User account
To get the utils needed, install the `apache2-utils` package with:  
`sudo apt-get install apache2-utils`  
To use Radicale you have to make at least one account with the following command:  
```sudo htpasswd -5 -c /docker-compose/radicale/config/users [username]```


### Firewall:
To allow necessary traffic, edit your firewalls.  
In this example I'm using ufw.
```
sudo apt install ufw
sudo ufw enable
sudo ufw allow 80/tcp
sudo ufw allow 443/tcp
sudo ufw allow [ssh port]/tcp
sudo ufw reload
```

### Change default SSH port
> [!WARNING]
> Do not close your session until you validated that the new one works!

> [!NOTE]
> Note that if you follow through then you have to copy the private keys from your Cloud shell and save them to your local machine

#### Download your keys
Cloud Shell doesn't allow custom ssh ports, so that's why you have to 'abandon' it if you change the ports
The key is located at `~/.ssh/google_compute_engine`
Download and move this key to `C:\Users\[username]\.ssh\`
Run these two commands before your first log in in PowerShell:
```
icacls $env:USERPROFILE\.ssh\google_compute_engine /inheritance:r
icacls $env:USERPROFILE\.ssh\google_compute_engine /grant:r "$($env:USERNAME):(R)"
```
Use the following command to log in from PowerShell:  
```ssh -i $env:USERPROFILE\.ssh\google_compute_engine -p [ssh port] [linux username]@[public ip]```

#### Change the port

Uncomment the Port 22 line, and replace 22 with your own choice, then restart the service.
```
sudo vim /etc/ssh/sshd_config
sudo systemctl restart sshd
```


# Instance configuration:  
Name:    vaultwarden  
Region:  us-central1  
OS:      debian-12-bookworm  
Machine type:  e2-micro (2 vCPUs, 1 GB memory)  
HTTP traffic:  on  
HTTPS traffic: on  
Network tags:  http-server https-server server   
Network interface name:  nic0  
Network:                 mynetwork  
Subnetwork:              subnet-us-central1  
Network tier:            Standard  
Storage: vaultwarden (Boot disk)  
Size:    10 GB  
Type:    Standard persistent disk  

## VPC network configuration:  
VPC network:  mynetwork  
Subnets:      subnet-us-central1  
Firewalls (all ingress, all allow):  
- mynetwork-allow-http - tcp:80 ; target tags: http-server 
- mynetwork-allow-https - tcp:443 ; target tags: https-server
- mynetwork-allow-custom - tcp:22,80,443,8080,[ssh port] icmp ; target: Apply to all
  
