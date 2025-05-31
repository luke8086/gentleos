#!/bin/sh

for x in gui apps lib kernel data; do
    cproto -I include -v -e ${x}/*.c > include/tmp.h
    mv include/tmp.h include/proto_${x}.h
done
