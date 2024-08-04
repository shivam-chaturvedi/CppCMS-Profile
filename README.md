# CppCMS Profile App

## Compiling the Application

1. Update the package list and upgrade installed packages:

    ```bash
    sudo apt update
    sudo apt upgrade
    ```

2. Install required development packages:

    ```bash
    sudo apt install build-essential libboost-all-dev libpcap-dev libmysqlclient-dev cmake make gcc g++
    ```

3. Download CppCMS:

    ```bash
    wget https://sourceforge.net/projects/cppcms/files/cppcms/1.2.1/cppcms-1.2.1.tar.bz2/download -O cppcms-1.2.1.tar.bz2
    ```

4. Extract the CppCMS archive:

    ```bash
    tar -xjf cppcms-1.2.1.tar.bz2
    cd cppcms-1.2.1
    ```

5. Build and install CppCMS:

    ```bash
    ./configure
    make
    sudo make install
    ```

## Installing MySQL Connector

1. Install the MySQL client library:

    ```bash
    sudo apt install libmysqlclient-dev
    ```

2. Install the MySQL Connector/C++:

    ```bash
    sudo apt install libmysqlcppconn-dev
    ```

   For Windows, download the MySQL Connector/C++ from: [MySQL Connector/C++](https://dev.mysql.com/downloads/connector/cpp/)

## Clone the Repository

```bash
git clone https://github.com/shivam-chaturvedi/CppCMS-Profile
cd CppCMS-Profile
```

## Build the Application

Compile the application with:

```bash
g++ app.cpp -o server -lcppcms -lmysqlcppconn
```

## Run the Server

Start the server with:

```bash
./server -c config.json
```

