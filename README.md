# ft_irc

## 📚 Project Description

**ft_irc** is a simplified implementation of an IRC (Internet Relay Chat) server, developed as part of the 42 curriculum. It allows multiple clients to connect and communicate through channels, using the IRC protocol. This project is an opportunity to explore socket programming, multi-threading or multiplexing, and the design of a real-time server.

## 🛠️ Features

- IRC server compliant with RFC 1459 (partial implementation)
- Multi-client management using `poll()`
- Basic user authentication (password required on connect)
- Commands supported:
  - `/NICK`, `/USER`, `/PASS`
  - `/JOIN`, `/PART`, `/PRIVMSG`, `/NOTICE`
  - `/QUIT`, `/PING`, `/PONG`
- Channel creation and management
- Message broadcasting within channels
- Private messaging between users

## 🚀 Getting Started

### Prerequisites

- Unix-like OS (Linux, macOS)
- C++ compiler (C++98 or C++11 depending on project requirements)
- Make

### Installation

git clone https://github.com/your_username/ft_irc.git

cd ft_irc 

make

Usage:
./ircserv '<'port'>' '<'password'>'

Example:
./ircserv 6667 mySuperSecret

You can then connect using a client like nc, irssi, or WeeChat:
nc localhost 6667

## 🧠 Learning Objectives

Understanding how IRC works (architecture, protocol)
Mastering socket programming in C++
Efficiently managing multiple clients
Designing a reliable event-driven system
Parsing and handling a command protocol

## 🧪 Testing
You can test the server using multiple terminal clients or dedicated IRC clients like:

irssi
WeeChat
telnet or nc for simple raw connections

## 👨‍💻 Authors
Emilien Houot &
Mattieu Cordes

📝 License

This project is part of the 42 School curriculum and is not licensed for production use.
