# Homelab Configuration
The configuration files and inner workings of my homelab setup

# Overview
This self-hosted stack is built for privacy, modularity, and remote access, powered by Docker Compose and Tailscale with Caddy handling reverse proxies where necessary.
Currently running in a VMware VM with plans of migrating to physical hardware after acquiring adequate parts. Having Docker compose as a centralized "hub" to launch all services (except bind9) will make future deployment faster and more reliable.

# The "but why deal with this" question?
Privacy: with online services increasingly turning into subscriptions collecting vast amounts of unnecessary data while legally shielding themselves with their egregious TOS.
Control: self-hosting gives full control of the service chain from server to end device allowing configuration based on precise needs and usecases.
Growth: as a future network engineer / cloud engineer applying my knowledge in ways giving useful, every day results further my carreer as well as my quality of life.

# Services
Service        |  Role
---------------|----------------------------------------------------------
Nextcloud AIO  |  File sync, calendar, contacts, and Collabora
Vaultwarden    |  Self-hosted Bitwarden password manager
Immich         |  Photo backup designed to be  Google Photos replacement
Caddy          |  Reverse proxy and TLS certification handling

# Access services via:
nc.basil1614.duckdns.org → Nextcloud  
photos.basil1614.duckdns.org → Immich
homelab.boa-sailfin.ts.net → Vaultwarden

# Notes
This setup is Tailscale only - accessible exclusively from the Tailnet.
Security & access control: no service is exposed as the whole network is hidden behind Tailscale.
Certification: Caddy cannot get valid certificates from any issuer necessitating self-signed TLS certificates, reserving the (single) Tailscale generated certificate for Vaultwarden to meet Bitwardens requirements blocking self-signed certificates.

# Repo Structure
homelab/
├── README.md                     # Project overview
├── caddy/
│   └── Caddyfile                 # Reverse proxy config with TLS
├── tailscale/
│   └── dns-setup.png             # Screenshot of split DNS config in Tailscale admin panel
├── docker-compose/
│   └── compose.yml               # Compose file handling Nextcloud, Vaultwarden, Immich, Caddy
├── bind9/
│   ├── basil1614.duckdns.org     # Zone file for internal DNS resolution
│   ├── named.conf.local          # Bind9 zone declarations
│   └── named.conf.options        # Bind9 global options

# Future plans
1. Build a physical server
2. Connect to the TrueNAS so it acts as the main storage pool for services like Immich and Nextcloud
3. Add OAuth for centralized identity management
4. Add Prometheus for monitoring
5. Migrate Plex to this server (from TrueNAS)
