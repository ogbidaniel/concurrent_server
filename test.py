import socket

host = '127.0.0.1'  # Localhost
port = 9090         # Port used by the server

# Create a socket connection
with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as client:
    client.connect((host, port))
    print("Connected to server")
    
    # Send a test message
    message = "^Hello$".encode('utf-8')
    client.sendall(message)
    
    # Receive response
    response = client.recv(1024)
    print("Received:", response.decode('utf-8'))
