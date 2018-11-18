#!/bin/sh
python -m grpc_tools.protoc -I ../protos --python_out=. --grpc_python_out=. ../protos/euclidesproto.proto
