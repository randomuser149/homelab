# Google Cloud  
This branch contains all files & commands that are and were used on the current Google Cloud VM instance.  
It only runs Caddy and Vaultwarden as of currently.  
Commands used can be found in the [commands.sh](./commands.sh)

## The file structure on the instance:  
```bash
/  
└── docker-compose/  
    ├── compose.yml  
    └── Caddyfile
```

## Instance configuration:  
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
  
