>[!IMPORTANT]
>I started a documentation page with Zensical, check it out! [https://randomuser149.github.io/homelab/](https://randomuser149.github.io/homelab/)  

# Homelab
This repo contains the files for my modular, privacy-focused homelab stack. It explores containerized services (& Quadlets), mixed types of remote access and automated setup using Ansible.  
The less that is manual and the more rebuildable the better.

## Documentation
>[!WARNING]
>It's NOT FULLY FINISHED. I just started it recently with minimal content.
>I'll try to fill it out as soon as possible.

Full documentation available at: [https://randomuser149.github.io/homelab/](https://randomuser149.github.io/homelab/)

## Services
This stack contains a variety of services from IoT to kitchen stuff.
| Service | Description | Added/Modified | Notes |
|-|-|-|-|
| Caddy | Reverse Proxy | 07/02/2026 | Needs more limitations on domain paths |
| Vaultwarden | Password manager | ?? | Needs automatic backups before can be trusted |
| Tandoor | Recipe manager & shopping list | 07/02/2026 | Needs automatic backups & additional `header_up`s in Caddy |
| Grafana | Dashboards | ?? | |
| InfluxDB | Database for IoT data | ?? | Needs automatic backups |
| Telegraf | Middleman between MQTT and DB | ?? | |
| Mosquitto | Gathers data from IoT | ?? | |
| DaviCAL | CalDAV server | 16/02/2026 | I can't really do backups (will look into alternatives) |

## Usage
```
git clone https://github.com/randomuser149/homelab.git
cd homelab
```
Create the vault and fill out the var files with the values that you'll use. Instruction (will be) available in the full documentation, then run:
```
ansible-playbook -i inventory.ini playbook.yml --ask-vault-pass
```

## Repo structure  
| File / Folder     | Description                           |
|-------------------|---------------------------------------|
| deprecated/       | Deprecated files                      |
| roles/            | Ansible roles                         |
| README.md         | Project overview                      |
| LICENCE.md        | Licence                               |
| inventory.ini     | Inventory file                        |
| playbook.yml      | Main Ansible playbook                 |
| requirements.yml  | Ansible collections                   |
| secrets.yml       | Vault template                        |
| vars.yml          | Non‑sensitive variables template      |


## Branches
>[!IMPORTANT]
>These branches are just for now, tailscale will be merged back to main, as soon as I create feature parity and add the documentations to the Zensical site.

| Branch       | Description                                                     |
|--------------|-----------------------------------------------------------------|
| main         | Current configurations (Cloudflare Tunnel, Podman containers, Quadlets, IoT monitoring) |
| tailscale    | Older Docker Compose stack behind Tailscale                     |
| cloud        | Very simple Google Cloud config                                 |
| gh-pages     | Built documentation page                                        |

## Requirements
- Ansible installed on your controller
- A system Ansible will configure (with SELinux support, or you need to manually change some labels)
- A domain you own
- A Cloudflare account
- ESP32 + Arduino IDE (if using sensors)

## Release notes
While this is not a production repo (yet lol), I find it easier to publish (which will happen soon I promise) rolling changes there as it keeps great track of time and changes to the whole setup.  
Release notes can be found here: [https://github.com/randomuser149/homelab/releases](https://github.com/randomuser149/homelab/releases)
