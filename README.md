# 🛡️ SafeBox

> A lightweight, highly secure Linux container runtime and sandbox built entirely from scratch in C++.

![C++17](https://img.shields.io/badge/C++-17-00599C?style=for-the-badge&logo=c%2B%2B&logoColor=white)
![Linux](https://img.shields.io/badge/Linux-FCC624?style=for-the-badge&logo=linux&logoColor=black)
![CMake](https://img.shields.io/badge/CMake-064F8C?style=for-the-badge&logo=cmake&logoColor=white)
![Bash](https://img.shields.io/badge/Bash-4EAA25?style=for-the-badge&logo=gnu-bash&logoColor=white)

## 📌 Overview
Developed as an exploration into Operating Systems architecture, SafeBox demystifies how modern container engines work under the hood. Instead of relying on external libraries like `runc` or `libcontainer`, this runtime interacts directly with the Linux Kernel via system calls to create fully isolated, sandboxed environments. 

It perfectly bridges the gap between standard dockerization and strict sandboxing, ensuring that untrusted processes can run securely without consuming excessive host resources or accessing the underlying system.

---

## ✨ Core Features Under the Hood
* **Process & File System Isolation:** Leverages Linux Namespaces (`PID`, `Mount`, `UTS`) and `pivot_root` to trap processes in an inescapable minimal Alpine Linux filesystem.
* **Resource Control (Cgroups v2):** Implements strict quotas on CPU and Memory usage, preventing containerized applications from monopolizing the host's hardware.
* **Kernel-Level Security:** Utilizes **Seccomp BPF** filters to blacklist dangerous system calls (e.g., `reboot`, `init_module`), immediately terminating any malicious breakout attempts.

---

## 🚀 Getting Started

### Prerequisites
This project requires a Linux environment with a modern kernel (v4.5+) and standard build tools:
`build-essential`, `cmake`, `libseccomp-dev`, `wget`, `tar`

### 1. Prepare the Root Filesystem
SafeBox requires a guest filesystem to act as the container's isolated root. Run the provided setup script to automatically download and extract a minimal Alpine Linux rootfs:
```bash
chmod +x setup_fs.sh
./setup_fs.sh

```

### 2. Build and Run

You can use the automated build script to compile the project via CMake and instantly launch a sandboxed shell:

```bash
chmod +x build.sh
./build.sh

```

*Alternatively, you can run SafeBox manually to pass custom resource limits:*

```bash
sudo ./build/safebox --cpu 20 --mem 50 --cmd /bin/sh

```

---

## 🔮 Future Work

* **Network Isolation:** Implementing a dedicated network namespace using Netlink sockets or `veth` pairs to fully isolate and filter container traffic from the host network.
* **Rootless Containers:** Mapping user namespaces (`CLONE_NEWUSER`) to allow the runtime to operate without requiring `sudo` privileges.
* **OverlayFS Support:** Migrating from standard directory binding to a layered filesystem for better storage efficiency.

---

*📝 Note: For an in-depth architectural breakdown, state machine flows, and security evaluation, please refer to `documentation.pdf`.*
