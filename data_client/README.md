# CPP Maicon Cesar - Data_client

## How to build from scratch (Ubuntu like):

### Installing dependencies
- sudo apt-get update
- sudo apt-get install git
- sudo apt-get install gcc g++
- sudo apt-get install cmake
- sudo apt-get install python3-pip
- pip install conan

### Clone repository
- git clone url

### Building application for first time
- conan profile new default --detect 
- conan profile update settings.compiler.libcxx=libstdc++11 default
- mkdir build && cd build
- conan install .. --build=missing
- cmake .. -G "Unix Makefiles" -DCMAKE_BUILD_TYPE=Release
- cmake --build .

### Execution
- Input file must be in the same directory 


```
sudo ./data_client <host> <port> <file_name>
```
