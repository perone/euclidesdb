import sys
import grpc

import euclidesproto_pb2_grpc as ir_grpc
import euclidesproto_pb2 as ir

def run_main():
    shutdown_type = int(sys.argv[1])

    with grpc.insecure_channel('localhost:50000') as channel:
        stub = ir_grpc.SimilarStub(channel)
        req = ir.ShutdownRequest()
        req.shutdown_type = shutdown_type
        reply = stub.Shutdown(req)
        print(reply)


if __name__ == "__main__":
    run_main()