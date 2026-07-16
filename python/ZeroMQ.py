import zmq
import json

context = zmq.Context()

socket = context.socket(zmq.PULL)
socket.bind("tcp://*:5555")

while True:
    data = socket.recv_string()
    event = json.loads(data)

    print("[ZMQ REQUEST]")
    print(event["action"])
    print(event["filename"])
    print(event["path"])