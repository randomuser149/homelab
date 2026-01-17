# Homelab Configuration Overview
This self-hosted stack is built for privacy, modularity, and remote access, powered by Podman Compose and different ways of access (currently only Cloudflare Tunnel) with Caddy handling reverse proxies where necessary.  

> [!IMPORTANT]
> I started this repo as a complete git beginner not even aware of branching, so now I'm slowly cleaning it up and restructuring.  
> Also I've changed approaches to the whole issue many times, but I'll try my best to make it organized as soon as possible.

# The "why deal with this" question?
1. Privacy: online services increasingly turn into subscriptions collecting vast amounts of unnecessary data while legally shielding themselves with egregious TOSs.  
2. Control: self-hosting gives full control of the service chain from server to end device allowing configuration based on precise needs and use cases.  
3. Growth & knowledge: as an aspiring network engineer / cloud engineer / cloud architect applying my knowledge in ways giving useful, every day results further my career as well as my quality of life.

# Branches
This repository contains multiple deployment variants and experiments:  
- **main** – Ansible master repo, eventually access type aware making other branches redundant
- **cloudflared** – Cloudflare Tunnel version
- **tailscale** – Tailscale version
- **cloud** – Google Cloud deployment
- **esp32** - Sensor data gathering, working as an input for services

# Release notes
While this is not a production repo, I find it easier to publish rolling changes there as it keeps great track of time and changes to the whole setup.  
Release notes can be found here: [https://github.com/randomuser149/homelab/releases](https://github.com/randomuser149/homelab/releases)

# Ansible
>[!NOTE]
>I'll make a guide about how it works and why very soon, but for now this will only contain the get it up and running part (so that I myself can remember).

This is a very good way to do the setup of services automated as well. I've just started exploring it, but the current playbook can spin up containers that make Vaultwarden available over Cloudflare Tunnel.  
The Cloudflare Dashboard setup is still manual but minimal.  
Follow the next sections in the [cloudflared branch](../cloudflared) guide:
- Add your domain to your Cloudflare account
- Create the Cloudflare Tunnel (without doing the terminal commands)
- Creating a Cloudflare API token for Caddy
After that get Ansible installed on another machine with `sudo apt install ansible -y`.  
Copy the `playbook.yml` and the templates files to the machine with Ansible.
Arrange them in a folder structure just like this:
```
./ansible
├── playbook.yml
└── templates/
    ├── compose_cloudflared.yml.j2
    └── Caddyfile_cloudflared.j2
```
Then create a vault with `ansible-vault create secrets.yml` and give it a strong password.  
Fill it with the following details and replace all [parts] with your accurate details:
```
ssh_user: [username]
ssh_password: [password]

cloudflare_install_token: [the token you get after creating the tunnel (only copy the token without the install commands)]
cloudflare_api_token: [the token you generated for Caddy]

vaultwarden_domain: [the domain you have published the route for this service]
```

And run the playbook with `ansible-playbook -i inventory.ini play.yml --ask-vault-pass` (change to the ansible dir).

# Future plans
1. Finish the playbook to include the IoT monitoring.
2. Get the Ansible playbook running on an old PC.
3. Make the Ansible playbook environment aware so one can do all with different templates.
4. Make backup and restore playbooks too.
5. Get a better PC capable of running heavier services (Immich, Jellyfin, Nextcloud).
6. Move the whole stack to run on Proxmox to benefit from ZFS filesystem.
7. Add OAuth for centralized identity management
8. Add Prometheus for monitoring