# 🌐 Webserv

> A non-blocking HTTP/1.1 server written in C++98 – 42 Project

<p align="center">
  <img src="https://github.com/othorel/Webserv/blob/main/img/webserv.png" />
</p>

---

## 🧠 About

**Webserv** is a minimalist HTTP server implemented as part of the 42 cursus. Its purpose is to help students understand the internals of the HTTP protocol, how web servers work, and how to handle I/O operations efficiently using low-level system calls.

This server is entirely written in **C++98**, using **poll()** for non-blocking I/O, and is capable of serving both static and dynamic content via CGI.

---

## 🛠️ Features

### Core

- Fully **non-blocking I/O** using a single `poll()` call
- Configurable via a custom **Nginx-inspired configuration file**
- **Virtual hosting** with `server_name` and multiple ports
- Support for HTTP methods: `GET`, `POST`, and `DELETE`
- **CGI support** (e.g., Python, PHP)
- **Autoindex** (directory listing)
- File **upload** support
- **Custom error pages**
- **HTTP redirection**
- Static file serving with default index
- Proper HTTP status code handling
- Resilient to stress and malformed requests
- Works with modern web browsers

### Bonus

- Multiple CGI language support
- Cookie & session handling (basic)
- Stylized autoindex
- Upload form HTML page
- Clean shutdown and error logging

---

## ⚙️ Tech Specs

- **Language:** C++98
- **Build system:** Makefile (with rules: `all`, `clean`, `fclean`, `re`)
- **Syscalls used:** `socket`, `bind`, `listen`, `accept`, `recv`, `send`, `poll`, `fcntl`, `execve`, etc.
- **No external libraries**, no Boost, no libft

---

## 🚀 Getting Started

### 🔧 Installation

```bash
git clone https://github.com/your-username/Webserv.git
cd Webserv
make
```

---

▶️ Run the server

```bash
./webserv [path/to/config.conf]
```
If no config is provided, a default one will be used.

---

📁 Project Structure

```bash
.
├── src/              # Source code (C++)
├── include/          # Header files
├── config/           # Sample configuration files
├── www/              # Static files (HTML, CSS, JS)
  ├── cgi-bin/           # CGI scripts (Python, PHP)
  ├── index.html/        # for www
  ├── errors/            # for errors 404, 500 ...
  and other directory for www
├── Makefile
└── README.md
```
---

⚙️ Configuration
Inspired by Nginx-style blocks, each config file can define:

- Multiple server blocks
- Ports, hosts, and server names
- Custom error_page directives
- client_max_body_size
- location blocks with:
  - Allowed HTTP methods
  - Root directories
  - Index files
  - Autoindex (on/off)
  - File uploads and upload paths
  - Redirections
  - CGI by file extension

🧾 Sample config

<p align="center">
  <img src="https://github.com/othorel/Webserv/blob/main/img/config.png" />
</p>

---
