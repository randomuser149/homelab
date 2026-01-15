# Tailscale
These are the Tailscale specific configurations, files and guides for the [main service stack](https://github.com/randomuser149/homelab).
>[!WARNING]
>This repo only contains the differences from the main service stack, and since it's a recent remodelling main has no version I could say this is compatible 100%.
>Also, it is a very work in progress guide, so don't follow it blindly please.

## Differences
Well, it first and foremost differs in how you access your services. This environment uses Tailscale, a mesh VPN you can use on all of your devices. The biggest pro is no exposed service, the biggest con is you have to install Tailscale on every device that leaves your LAN to access your stack.
>[!WARNING]
>This environment also includes services not present in the main stack (or other environments).
>I'll call these non-intentional differences, emerging from organic, exploration driven development.  
>The services in question are:
> - Immich
> - Nextcloud

## Configurations
>[!IMPORTANT]
>This guide assumes you already have Docker (or Podman) Compose already installed on your system.

### Installing Tailscale
*Placeholder, will gather the steps for the guide later*

### Caddy Container
Add the following snippet to your `compose.yml`.
It includes networks present for the non-intentional differences too (Nextcloud, Immich).
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
    # Map Caddyfile to its location on host
      - /docker-compose/Caddyfile:/etc/caddy/Caddyfile
      - caddy_data:/data
      - caddy_config:/config
      - /etc/caddy/certs:/etc/caddy/certs:ro
    networks:
    # Placing Caddy into both networks to reach all services
      - nextcloud-aio
      - docker-compose_default

volumes:
  caddy_data:
  caddy_config:

networks:
  nextcloud-aio:
    external: true
  docker-compose_default:
    external: true
  ```
### Caddyfile
To act as the reverse proxy for the services this environment supports currently, add the following snippet into your Caddyfile.  
It has all the reverse proxies for the non-intentional differences too (Nextcloud, Immich).
```
# Nextcloud master domain
nc.basil1614.duckdns.org {
	tls internal

  # Nextcloud AIO
	reverse_proxy nextcloud-aio-apache:11000

  # Nextcloud to Collabora
	reverse_proxy /cool/* nextcloud-aio-collabora:9980 {
		transport http {
			versions 1.1
		}
	}
  # Collabora to Nextcloud
	reverse_proxy /apps/richdocumentscode/proxy.php* nextcloud-aio-apache:11000 {
		transport http {
			versions 1.1
		}
	}
}

# Immich
photos.basil1614.duckdns.org {
	tls internal
	reverse_proxy immich_server:2283
}

# Vaultwarden
homelab.boa-sailfin.ts.net {
	reverse_proxy vaultwarden:80
  # Using certificates from "tailscale cert" command because Bitwarden rejects self signed certificates
	tls /etc/caddy/certs/homelab.boa-sailfin.ts.net.crt /etc/caddy/certs/homelab.boa-sailfin.ts.net.key
}
```
### Non-intentonal differentials
#### Nextcloud
*GUI setup guide wil be included later*
```
services:
  nextcloud:
    image: nextcloud/all-in-one:latest
    restart: always
    container_name: nextcloud-aio-mastercontainer
    volumes:
      - nextcloud_aio_mastercontainer:/mnt/docker-aio-config
      - /var/run/docker.sock:/var/run/docker.sock:ro
      - /ncdata:/mnt/data
    ports:
      - 8080:8080
    environment:
      - NEXTCLOUD_DATADIR=/mnt/data
      - APACHE_PORT=11000
      - APACHE_IP_BINDING=0.0.0.0
      - SKIP_DOMAIN_VALIDATION=true

volumes:
  nextcloud_aio_mastercontainer:
    name: nextcloud_aio_mastercontainer

networks:
  nextcloud-aio:
    external: true
```
#### Immich
*GUI setup guide wil be included later*
```
services:
  immich-server:
    container_name: immich_server
    image: ghcr.io/immich-app/immich-server:${IMMICH_VERSION:-release}
    # extends:
    #   file: hwaccel.transcoding.yml
    #   service: cpu # set to one of [nvenc, quicksync, rkmpp, vaapi, vaapi-wsl] for accelerated transcoding
    volumes:
      # Do not edit the next line. If you want to change the media storage location on your system, edit the value of UPLOAD_LOCATION in the .env file
      - ${UPLOAD_LOCATION}:/data
      - /etc/localtime:/etc/localtime:ro
    env_file:
      - .env
    ports:
      - '2283:2283'
    depends_on:
      - redis
      - database
    restart: always
    healthcheck:
      disable: false

  immich-machine-learning:
    container_name: immich_machine_learning
    # For hardware acceleration, add one of -[armnn, cuda, rocm, openvino, rknn] to the image tag.
    # Example tag: ${IMMICH_VERSION:-release}-cuda
    image: ghcr.io/immich-app/immich-machine-learning:${IMMICH_VERSION:-release}
    # extends: # uncomment this section for hardware acceleration - see https://immich.app/docs/features/ml-hardware-acceleration
    #   file: hwaccel.ml.yml
    #   service: cpu # set to one of [armnn, cuda, rocm, openvino, openvino-wsl, rknn] for accelerated inference - use the `-wsl` version for WSL2 where applicable
    volumes:
      - model-cache:/cache
    env_file:
      - .env
    restart: always
    healthcheck:
      disable: false

  redis:
    container_name: immich_redis
    image: docker.io/valkey/valkey:8-bookworm@sha256:fea8b3e67b15729d4bb70589eb03367bab9ad1ee89c876f54327fc7c6e618571
    healthcheck:
      test: redis-cli ping || exit 1
    restart: always

  database:
    container_name: immich_postgres
    image: ghcr.io/immich-app/postgres:14-vectorchord0.4.3-pgvectors0.2.0@sha256:8d292bdb796aa58bbbaa47fe971c8516f6f57d6a47e7172e62754feb6ed4e7b0
    environment:
      POSTGRES_PASSWORD: ${DB_PASSWORD}
      POSTGRES_USER: ${DB_USERNAME}
      POSTGRES_DB: ${DB_DATABASE_NAME}
      POSTGRES_INITDB_ARGS: '--data-checksums'
      # Uncomment the DB_STORAGE_TYPE: 'HDD' var if your database isn't stored on SSDs
      # DB_STORAGE_TYPE: 'HDD'
    volumes:
      # Do not edit the next line. If you want to change the database storage location on your system, edit the value of DB_DATA_LOCATION in the .env file
      - ${DB_DATA_LOCATION}:/var/lib/postgresql/data
    shm_size: 128mb
    restart: always

volumes:
  model-cache:
```