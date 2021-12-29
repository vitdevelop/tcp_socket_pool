#!/bin/bash

mkdir -p lib
git clone https://github.com/vitdevelop/pthread_pool.git lib/pthread_pool
make -C lib/pthread_pool
