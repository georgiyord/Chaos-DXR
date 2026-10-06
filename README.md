# Chaos-DXR
Ray Tracing project for the course Chaos Camp 2026

## Description

The project is aims to utilise Microsoft's DirectX 12 platform, or more specifically it's real-time API (DXR). The project is mainly developed on Linux (NixOS), so the repository contains tooling for working with Wine, a translation layer for Windows programs to run on Unix based systemd, and vkd3d-proton, a translation layer for DirectX 12 programs to run on Vulkan.

## Setup

### Nix/NixOS

If [nix](https://github.com/NixOS/nix) is available on the platform, a flake is available with a devShell to automate the setup process. All that's needed is running `nix develop`. Tests will run to confirm that the wine environment has been set up correctly.

### Linux

After cloning the repository, you have to setup Wine, MSVC and the Windows SDK by running some of the provided tools. Requirements for running them are vendor specific and should be looked at [msvc-wine](https://github.com/mstorsjo/msvc-wine).

```
git clone https://github.com/georgiyord/Chaos-DXR.git
cd Chaos-DXR
source tools/wine-msvc/wine-env.sh
./tools/wine-msvc/init.sh
```
Tests will run to confirm that the wine environment has been set up correctly.

### Windows

WIP

## 


