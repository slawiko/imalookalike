# Index

### Dependencies
Application uses [cpp-httplib](https://github.com/yhirose/cpp-httplib) as dependency.

### Building
On Linux/MacOS (GCC):
```
g++ --std=c++11 -pthread -O2 -x c++ -I<path to httplib> main.cpp index.cpp thread_pool.cpp arguments.cpp
```

On Windows (VS compiler):
```
cl /TP /MT /EHsc /O2 /GL /I<path to httplib> main.cpp index.cpp thread_pool.cpp arguments.cpp
```

### REST API  
```GET /health```  
**Description**: Healthcheck  
**Response content**: String "I'm okay"  
**Response content type**: text/plain  
**Response code**: 200  

```POST /neighbour```  
**Description**: Find nearest image for provided descriptor  
**Request content**: Image descriptor - comma-separated list of real numbers (example: 0.1,1.73,13.69)  
**Request content type**: text/plain  
**Response content**: Found image  
**Response content type**: image/jpeg  
**Response code**: 200  
