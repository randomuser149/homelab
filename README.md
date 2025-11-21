# Homelab Configuration Overview
This self-hosted stack is built for privacy, modularity, and remote access, powered by Docker Compose and Tailscale with Caddy handling reverse proxies where necessary.  
Currently running in a VMware VM with plans of migrating to physical hardware after acquiring adequate parts (only a motherboard is needed as of now). Having Docker compose as a centralized "hub" to launch all services (except bind9) will make future deployment faster and more reliable.

# The "but why deal with this" question?
1. Privacy: online services increasingly turn into subscriptions collecting vast amounts of unnecessary data while legally shielding themselves with egregious TOSs.  
2. Control: self-hosting gives full control of the service chain from server to end device allowing configuration based on precise needs and usecases.  
3. Growth: as an aspiring network engineer / cloud engineer / system administrator applying my knowledge in ways giving useful, every day results further my carreer as well as my quality of life.  

# Services
Service        |  Role
---------------|----------------------------------------------------------
Nextcloud AIO  |  File sync, calendar, contacts, and Collabora
Vaultwarden    |  Self-hosted Bitwarden password manager
Immich         |  Photo backup designed to be  Google Photos replacement
Caddy          |  Reverse proxy and TLS certification handling

# Access services via:
Hostname                     |  Service
-----------------------------|-------------
nc.basil1614.duckdns.org     | Nextcloud  
photos.basil1614.duckdns.org | Immich  
homelab.boa-sailfin.ts.net   | Vaultwarden

# Notes
This setup is Tailscale only - accessible exclusively from the Tailnet.    
Security & access control: no service is exposed as the whole network is hidden behind Tailscale.  
Certification: Caddy cannot get valid certificates from any issuer necessitating self-signed TLS certificates, reserving the (single) Tailscale generated certificate for Vaultwarden to meet Bitwardens requirements blocking self-signed certificates.

# Repo Structure
homelab/  
├── README.md                       
├── caddy/  
│   └── Caddyfile                   
├── tailscale/  
│   └── dns-setup.png              
├── docker-compose/  
│   └── compose.yml  
├── bind9/  
│   ├── basil1614.duckdns.org      
│   ├── named.conf.local            
│   └── named.conf.options          

# Future plans
1. Build a physical server
2. Add a Pi device (or similar) as the only always-on machine running Vaultwarden & Caddy (these are continuous needs while the rest are unpredictable, high intensity loads), keeping the "big" server and NAS in sleep unless woken up for access of services.
3. Connect to the TrueNAS with RAID Z1 so it acts as a redundant, main storage pool for services like Immich and Nextcloud
4. Add OAuth for centralized identity management
5. Add Prometheus for monitoring
6. Migrate Plex to this server (from TrueNAS)
