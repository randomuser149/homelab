# Cloudflared
This version runs Vaultwarden as the only service and it is accessible by just the domain utilizing Cloudflare Tunnel.  
Since my lovely ISP blocks me from obtaining an SSL certificate and also blocks any incoming traffic, this is the best option I've found to expose my services so as to not rely on Tailscale or any other VPNs.  
>[!NOTE]
>In the first version it was running on Debian which I switched out to Alma Linux to utilize the benefits of SELinux. Which of course comes with a bunch of new challanges but I'm putting them into the guide as well.  

## Repo structure
```
/
├── README.md
├── docker-compose.yml
├── Caddyfile
└── images/
    ├── namecheap1.png
    ├── namecheap2.png
    ├── tunnel1.png
    ├── tunnel2.png
    ├── tunnel3.png
    ├── tunnel4.png
    ├── api1.png
    ├── api2.png
    ├── api3.png
    └── end.png
```
## What you need
- a valid domain name you own
- a Cloudflare account

## Configuration
### Add your domain to your Cloudflare account
On https://dash.cloudflare.com/ navigate to Domains and either press "Buy a domain" if you don't have one and want to buy it from Cloudflare or continue with "Onboard a domain".  
Enter your apex domain and press Continue at the bottom.  
Update the nameservers at your registar  
For me, I logged in to namecheap, navigated to Account > Domains > Manage:  
![Namecheap Navigation](images/namecheap1.png)  
Then I updated the nameservers with the ones Cloudflare provided:  
![Namecheap DNS panel](images/namecheap2.png)  
  
> [!NOTE]
> Note: This section may not be fully accurate as I've written this from memory as I've not started the documentation until it worked, and this section can't be backtracked as it's GUI heavy. Refer to the official guides if my explanation misses parts https://developers.cloudflare.com/fundamentals/manage-domains/add-site/

###  Create the Cloudflare Tunnel
On https://dash.cloudflare.com/ navigate to Zero Trust > Networks > Connectors.  
Here, under Cloudflare Tunnels press Create a tunnel and choose Cloudflared then press Next.  
![Cloudflare "Create a tunnel"](images/tunnel1.png)  
Enter a recognizable name, then continue.  
Choose your environtment and follow the instructions.   
> [!TIP]
> I've included both Debian and Red Hat (or Alma) configartions, if your environment is one of them, follow the correct one, if not, follow the instructions given to you by Cloudflare when selecting your environment.   

**Alma:**  
```
curl -fsSl https://pkg.cloudflare.com/cloudflared.repo | sudo tee /etc/yum.repos.d/cloudflared.repo

sudo yum update
sudo yum install cloudflared

sudo cloudflared service install {token}
```
**Debian:**  
```
sudo mkdir -p --mode=0755 /usr/share/keyrings
curl -fsSL https://pkg.cloudflare.com/cloudflare-public-v2.gpg | sudo tee /usr/share/keyrings/cloudflare-public-v2.gpg >/dev/null

echo 'deb [signed-by=/usr/share/keyrings/cloudflare-public-v2.gpg] https://pkg.cloudflare.com/cloudflared any main' | sudo tee /etc/apt/sources.list.d/cloudflared.list

sudo apt-get update && sudo apt-get install cloudflared
sudo cloudflared service install {token}
```

After doing that, click Configure on your newly created tunnel  
![Cloudflare Configure tunnel](images/tunnel2.png)  
Click Add a published application route (you can add more later)  
![Cloudflare Add a published application route](images/tunnel3.png)  
Configure your application route  
![Cloudflare configure application route](images/tunnel4.png)  

>[!IMPORTANT]
>If you want Caddy to handle TLS and not Cloudflare, then under the 'Service' select HTTPS for 'Type' and type localhost:443 for the 'URL' (or where Caddy is listening).  
>Expand Additional application settings > TLS and toggle 'Match SNI to Host' ON.  

After finishing, you can repeat this process to add more routes. (press the three dots > Configure > Published application routes to add more routes, for example for different services under separate subdomains.)  

### Getting Podman Compose
>[!NOTE]
>If you're on Debian or any other system that has no SELinux integration, you can safely go with Docker Compose.

Since we are on Alma Linux our containers need to be able to cooperate with SELinux without any problems (and I've found Docker has issues with exactly that, and I've left out getting Docker in the previous version anyways).  
To get Podman Compose use the following commands:
```
sudo dnf install -y epel-release
sudo dnf install -y podman-compose
```
I believe that's all that is needed to be done.
To be sure it works, run ```podman compose --version```. If it prints a version number, you're good to go.

### Configure Caddy
#### Creating a Cloudflare API token
For this I followed the [original guide](https://github.com/CaddyBuilds/caddy-cloudflare?tab=readme-ov-file#configuration) step by step, but here's what I did:  
Navigate to Profile > API Tokens and click Create Token:  
![Cloudflare Profile](images/api1.png)  
![Cloudflare API Tokens](images/api2.png)  
Then click Get started and configure it the following way:  
![Cloudflare API Tokens](images/api3.png)  
Then Click Continue to summary, and create it.
>[!WARNING]
>NEVER share your API token with anyone.  
>Save your API key somewhere secure, because you'll need it in the following step and you can't open it again.
  
#### Create/update the Compose file
You can follow the [original guide](https://github.com/CaddyBuilds/caddy-cloudflare) but here's my compose file:
>[!IMPORTANT]
>Replace the CLOUDFLARE_API_TOKEN with the token you got in the previous step.

```
services:
  caddy:
    image: ghcr.io/caddybuilds/caddy-cloudflare:latest
    restart: unless-stopped
    cap_add:
      - NET_ADMIN
    ports:
      - "80:80"
      - "443:443"
      - "443:443/udp"
    volumes:
      - $PWD/Caddyfile:/etc/caddy/Caddyfile
      - $PWD/site:/srv
      - caddy_data:/data
      - caddy_config:/config
    environment:
      - CLOUDFLARE_API_TOKEN=your_cloudflare_api_token (you made this in the previous step)

volumes:
  caddy_data:
  caddy_config:
```

#### Create/update the Caddyfile
I used the Global Configuration setup and it looks like this:
```
{
	acme_dns cloudflare {env.CLOUDFLARE_API_TOKEN}
}

subdomain.example.tld {
	reverse_proxy vaultwarden:80
}
```
#### Add Vaultwarden as a service
You have to append the next section to the already created compose.yml in a way that vaultwarden is one intendation (2 spaces in my versions) inside the big "services:" block.
```
  vaultwarden:
    image: vaultwarden/server:latest
    container_name: vaultwarden
    restart: unless-stopped
    ports:
      - 8081:80
    volumes:
      - /vw-data:/data
    networks:
      default-network:
```
#### Update SELinux contexts
>[!IMPORTANT]
>You only have to do this part if you're on a system that has SELinux. So on Debian, skip this part.  

Trust me, I forgot and tried to run the containers without this part. To say the least no-one could access their directories or write files, or basically do anything. which is part of why SELinux is good.  
  
For Caddy, change to the directory where your Caddyfile is located then issue the following commands:
```
semanage fcontext -a -t svirt_sandox_file_t "$PWD/Caddyfile"
restorecon -v "$PWD/Caddyfile"
```
You should see a message saying `Relabeled /path/to/Caddyfile from unconfined_u:object_r:default_t:s0 to unconfined_u:object_r:container_file_t:s0`  
To check, you can run the `ls -Z Caddyfile` command to see if it says `unconfined_u:object_r:container_file_t:s0 Caddyfile`

For Vaultwarden issue the following commands:
>[!IMPORTANT]
>Replace `/vw-data` with the path to the dir you gave Vaultwarden to use in the compose file.

```
semanage fcontext -a -t svirt_sandox_file_t "/vw-data"
restorecon -Rv "/vw-data"
```
To check, you can run the `ls -dZ /vw-data` command to see if it says `unconfined_u:object_r:container_file_t:s0 /vw-data`
#### Firewall
>[!IMPORTANT]
>Alma uses Firewalld by default, but on other systems (Ubuntu/Debian) you might encounter other firewalls, but you have to enable port 443 over TCP in all cases.  

To allow Caddy to listen and actually send and recieve traffic on port 443 (HTTPS) use the following commands:
```
firewall-cmd --zone=public --add-port=443/tcp --permanent
firewall-cmd --reload
firewall-cdm --list-ports
```
The last command should return `443/tcp`.

#### Start the containers  
To start the container use `podman compose up -d`
#### Debug
I didn't run into any issues but if you do, please reference the original guide and also you can view logs with:  
`podman logs <container-name>`

## Results
If everything went according to plan, your domain (and/or subdomains) should be reachable from just a web browser.
Now, I can access my own Vaultwarden service just by typing in the subdomain for it.  
![Result](images/end.png)  