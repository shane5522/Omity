import zmq
import json

class ZmqReceiver:
    context = zmq.Context()

    socket = context.socket(zmq.PULL)
    socket.bind("tcp://*:5555")

    while True:
        data = socket.recv_string()

        if(data == "shutdown"):
            print("shutdown")
            break;

        event = json.loads(data)

        print("[ZMQ REQUEST]")
        print(event["action"])
        print(event["filename"])
        print(event["path"])

    socket.close()
    context.term()

    print("ZMQ closed")