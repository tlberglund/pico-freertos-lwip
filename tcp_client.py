import socket
import time
import argparse

class PicoTCPClient:
    def __init__(self, host, port=4242):
        self.host = host
        self.port = port
        self.sock = None
    
    def connect(self):
        """Establish connection to the Pico"""
        try:
            self.sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
            self.sock.connect((self.host, self.port))
            print(f"Connected to {self.host}:{self.port}")
            return True
        except ConnectionRefusedError:
            print(f"Connection refused to {self.host}:{self.port}")
            return False
        except Exception as e:
            print(f"Error connecting: {e}")
            return False
    
    def send_message(self, message):
        """Send a message and wait for response"""
        if not self.sock:
            print("Not connected!")
            return None
        
        try:
            # Send the message
            self.sock.send(message.encode('utf-8'))
            print(f"Sent: {message}")
            
            # Wait for response
            response = self.sock.recv(1024)
            print(f"Received: {response.decode('utf-8')}")
            return response
        except Exception as e:
            print(f"Error in communication: {e}")
            return None
    
    def close(self):
        """Close the connection"""
        if self.sock:
            self.sock.close()
            self.sock = None
            print("Connection closed")

def main():
    parser = argparse.ArgumentParser(description='TCP Client for Pico')
    parser.add_argument('--host', type=str, required=True, help='Pico IP address')
    parser.add_argument('--port', type=int, default=4242, help='Port number (default: 4242)')
    args = parser.parse_args()

    client = PicoTCPClient(args.host, args.port)
    
    if client.connect():
        try:
            while True:
                message = input("Enter message (or 'quit' to exit): ")
                if message.lower() == 'quit':
                    break
                    
                client.send_message(message)
                time.sleep(0.1)  # Small delay to prevent flooding
                
        except KeyboardInterrupt:
            print("\nExiting...")
        finally:
            client.close()

if __name__ == "__main__":
    main()
    