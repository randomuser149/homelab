# Homelab Configuration Overview
This self-hosted stack is built for privacy, modularity, and remote access, powered by Docker Compose and Tailscale with Caddy handling reverse proxies where necessary.  
It is planned to be a hybrid setup running on my own hardware as well as in a cloud instance.

# The "but why deal with this" question?
1. Privacy: online services increasingly turn into subscriptions collecting vast amounts of unnecessary data while legally shielding themselves with egregious TOSs.  
2. Control: self-hosting gives full control of the service chain from server to end device allowing configuration based on precise needs and usecases.  
3. Growth & knowledge: as an aspiring network engineer / cloud engineer / cloud architect applying my knowledge in ways giving useful, every day results further my carreer as well as my quality of life.

# Cloud?
Yes, cloud. as for the why it comes down to costs. Keeping a big server always on eats a lot of electricity. So currently (until i acquire a Pi or low power device) a cloud instance handles always on services (Vaultwarden, Caddy) for a much much lower cost (currently for free, but while Google says e2-micro is always free I have doubts) than a Ryzen 5 2600x server would.  
A big difference is that it does not need Tailscale for access as it has a public IP address, that my home server doesn't, and my lovely ISP blocks port forwardings so I cannot host any services without VPNs which make every day life a tiny bit harder even if they provide good security.  
All of the configuration files and commands are located in the [gcloud](./gcloud) directory of this repo.
For obvious security reasons, from now on I'll stop publishing the domains (or IPs) to minimize possible attack vectors.

# Release notes
While this is not a production repo, I find it easier to publish rolling changes there as it keeps great track of time and changes to the whole setup.  
Release notes can be found here: [https://github.com/randomuser149/homelab/releases](https://github.com/randomuser149/homelab/releases)

# Services
Service        |  Role | Where it is
---------------|----------------------|-----------------------------------
Nextcloud AIO  |  File sync, calendar, contacts, and Collabora | Home VM
Vaultwarden    |  Self-hosted Bitwarden password manager | Cloud
Immich         |  Photo backup designed to be  Google Photos replacement | Home VM
Caddy          |  Reverse proxy and TLS certification handling | Cloud & Home VM

# Repo Structure
homelab/  
├── README.md                       
├── caddy/  
│&emsp;&emsp;└── Caddyfile                   
├── tailscale/  
│&emsp;&emsp;└── dns-setup.png              
├── docker-compose/  
│&emsp;&emsp;└── compose.yml          
├── gcloud/  
│&emsp;&emsp;└── docker-compose/  
│&emsp;&emsp;&emsp;&emsp;&emsp;├── README.md  
│&emsp;&emsp;&emsp;&emsp;&emsp;├── commands.sh  
│&emsp;&emsp;&emsp;&emsp;&emsp;├── docker-compose.yml  
│&emsp;&emsp;&emsp;&emsp;&emsp;└── Caddyfile  
├── bind9/  
│&emsp;&emsp;├── basil1614.duckdns.org      
│&emsp;&emsp;├── named.conf.local            
│&emsp;&emsp;└── named.conf.options          

# Future plans
1. Build a physical server
2. Currently Cloud handles the following: ~~Add a Pi device (or similar)~~ to be the only always-on machine running Vaultwarden & Caddy (these are continuous needs while the rest are unpredictable, high intensity loads), keeping the "big" server and NAS in sleep unless woken up for access of services.
3. Connect to the TrueNAS with RAID Z1 so it acts as a redundant, main storage pool for services like Immich and Nextcloud
4. Add OAuth for centralized identity management
5. Add Prometheus for monitoring
6. Migrate Plex to this server (from TrueNAS)
