import socket
import threading
import time

# SERVER_IP = '127.0.0.1'
SERVER_IP = '127.0.0.1'

SERVER_PORT = 8080
NUM_CLIENTS = 10

def client_task(id):
    try:
        sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        sock.connect((SERVER_IP, SERVER_PORT))
        message = f"Message du client {id}"
        sock.sendall(message.encode())
        data = sock.recv(1024)
        print(f"Client {id} reçu: {data.decode()}")
        sock.close()
    except Exception as e:
        print(f"Client {id} erreur: {e}")

threads = []

for i in range(NUM_CLIENTS):
    t = threading.Thread(target=client_task, args=(i,))
    t.start()
    threads.append(t)
    time.sleep(0.1)  # décaler un peu le démarrage pour simuler un trafic léger

for t in threads:
    t.join()

print("Test terminé.")
