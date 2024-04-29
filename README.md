# Secure Password Manager

This repository contains a secure password manager application designed to help users store and manage their passwords safely. The application employs many cryptography primative to ensure the security and confidentiality of users' sensitive information.

## Features

- **Secure Storage**: Utilizes advanced encryption algorithms to securely store passwords and sensitive information in a Mariadb Database.
- **User-Friendly Interface**: Intuitive interface for easy navigation and management of passwords.
- **Cross-Platform Compatibility**: Compatible with various operating systems, providing flexibility and accessibility to users.

## Installation

1. Clone the repository:
```
git clone https://github.com/GabrielVega19/Password-Manager.git
```
2. Download the Boost asio library and the Crypto++ libraries
```
https://www.boost.org/doc/libs/1_76_0/doc/html/boost_asio.html
https://cryptopp.com/

```
3. Link the libraries in PMLibrary/CMakeLists.txt
4. Download mariadb and configure the login in PMLibrary/src/Database.cpp
5. Create A database and create two tables with these layouts. 
```
Name: users
+----------+--------------+------+-----+---------+-------+
| Field    | Type         | Null | Key | Default | Extra |
+----------+--------------+------+-----+---------+-------+
| username | varchar(100) | NO   | PRI | NULL    |       |
| password | blob         | NO   |     | NULL    |       |
| salt     | blob         | NO   |     | NULL    |       |
+----------+--------------+------+-----+---------+-------+

Name: password_storage
+----------+---------+------+-----+---------+----------------+
| Field    | Type    | Null | Key | Default | Extra          |
+----------+---------+------+-----+---------+----------------+
| entry    | int(11) | NO   | PRI | NULL    | auto_increment |
| user     | blob    | NO   |     | NULL    |                |
| website  | blob    | NO   |     | NULL    |                |
| username | blob    | NO   |     | NULL    |                |
| password | blob    | NO   |     | NULL    |                |
| iv       | blob    | NO   |     | NULL    |                |
+----------+---------+------+-----+---------+----------------+
```
6. Alter the IP address and ports for the service in PMClient/main.cpp and PMServer/main.cpp
```
I had the client connect to localhost because I only have one computer however if you have more than one computer then you can run the server on one and connect to the server from the other computer, just put the private ip address of the server computer in the client main.cpp. 
```
7. Compile the codebase with cmake 