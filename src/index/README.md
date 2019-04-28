# Index

Index for searching images by their descriptors. Implements [HNSW](https://arxiv.org/abs/1603.09320) algorithm.

### Dependencies
Application uses [cpp-httplib](https://github.com/yhirose/cpp-httplib) as dependency.

### Building
#### Docker:
```
docker build -t index .
docker run -p <port>:8000 index
```

#### Linux/MacOS (GCC):
```
g++ --std=c++11 -pthread -O2 -x c++ -I<path to httplib> main.cpp index.cpp thread_pool.cpp arguments.cpp
```

#### Windows (VS compiler):
```
cl /TP /MT /EHsc /O2 /GL /I<path to httplib> main.cpp index.cpp thread_pool.cpp arguments.cpp
```

### Arguments
\<arg\>=\<value\>  
  
`-h` `--help`  
Print info about arguments  
  
`--M`  
Max count of neighbours for nodes on non-zero layers  
Default value: 16  
  
`--M0`  
Max count of neighbours for nodes on zero layer  
Default value: 2 * M  
  
`-eC` `--efConstruction`  
Count of tracked nearest nodes during index creation  
Default value: 100  
  
`-eS` `--efSearch`  
Count of tracked nearest nodes during search  
Default value: 10  
  
`--mL`  
Prefactor for random level generation  
Default value: 1/ln(M)  
  
`-k` `--keepPrunedConnections`  
Keep constant number of nodes neighbours  
Default value: true  
  
`-dt` `--data`  
Path to file with objects for index  
Default value: "index.data"  
  
`-dm` `--dump`  
Path to file with index dump  
Default value: "index.dump"  
  
`-ds` `--dataset`  
Path to dataset directory  
Default value: "./"  
  
`-b` `--base`  
Count of object, that will be inserted sequentially  
Default value: 1000  
  
`-a`   `--address`  
Address, that web-server is hosted on  
Default value: 127.0.0.1  
  
`-p`   `--port`  
Port, that web-server listen to  
Default value: 8000  

### REST API  
`GET /health`  
Healthcheck  
Response: "I'm okay"  
Response content type: text/plain  
  
`GET /descriptor-size`  
Get descriptor size of current index  
Response: Descriptor size  
Response content type: text/plain  
  
`POST /neighbour`  
Find nearest image by provided descriptor  
Request: Image descriptor - comma-separated list of real numbers (example: 0.1,1.73,13.69)  
Request content type: text/plain  
Response: Found image (binary)  
Response content type: image/<jpeg|png|gif|bmp|tiff>; application/octet-stream in case of unknown extension  
