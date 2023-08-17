import socket

# Setup packet information and confirm in console
UDP_IP = "192.168.1.200"
UDP_PORT = 1234
MESSAGE = "testMessage"

print("UDP target IP: %s" % UDP_IP)
print("UDP target port: %s" % UDP_PORT)
print("message: %s" % MESSAGE)

# Create a socket object then send formatted packet
sock = socket.socket(socket.AF_INET, # Internet
                     socket.SOCK_DGRAM) # UDP

sock.sendto(MESSAGE.encode(), (UDP_IP, UDP_PORT))
