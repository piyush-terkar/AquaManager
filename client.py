import sys
import socket
import json

args = sys.argv[1].strip().split(":");
host = args[0]
port = int(args[1].strip())

s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
s.connect((host, port))
try:
    while(True):
        data = s.recv(1024)
        print(json.loads(data))
except KeyboardInterrupt:
    print("Closing socket")
    s.close()