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
| File / Folder     | Description                          |
|-------------------|--------------------------------------|
| deprecated/       | Deprecated files                      |
| roles/            | Ansible roles                         |
| README.md         | Project overview                      |
| inventory.ini     | Inventory file                        |
| playbook.yml      | Main Ansible playbook                 |
| requirements.yml  | Ansible collections                   |
| secrets.yml       | Vault template                        |
| vars.yml          | Non‑sensitive variables template      |


## Branches
>[!IMPORTANT]
>These branches are just for now, tailscale and cloudflared will be merged back to main, as soon as I add the documentations from those branches to the Zensical site.

| Branch       | Description                                                     |
|--------------|-----------------------------------------------------------------|
| main         | Current configurations (Cloudflare Tunnel, Podman containers, Quadlets, IoT monitoring) |
| tailscale    | Older Docker Compose stack behind Tailscale                     |
| cloud        | Very simple Google Cloud config                                 |
| esp32        | ESP project files                                               |
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
