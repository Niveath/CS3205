from packet import Packet
import socket
import sys
import random

class Receiver:
    def __init__(self, sock, output_file, packet_size, window_size):
        self.debug = True
        self.drop_prob = 0

        self.sock = sock
        self.output_file = output_file
        self.packet_size = packet_size
        self.window_size = window_size

        self.packet_handler = Packet(None, packet_size, window_size, False)
        self.seq_num_size = self.packet_handler.seq_num_size
        self.max_seq_num = self.packet_handler.max_seq_num
        
        self.next_packet = 0

        self.file = open(self.output_file, 'wb')
        self.recv_packets()

    def recv_packets(self):
        try:
            while True:
                packet, addr = self.sock.recvfrom(self.packet_size + self.seq_num_size)
                if not packet:
                    break

                seq_num, data = self.packet_handler.read_data_packet(packet)
                if(self.debug):
                    print("Received packet:", seq_num, "Expected packet: ", self.next_packet)
                if seq_num == self.next_packet:
                    self.file.write(data)
                    self.next_packet = (self.next_packet + 1) % self.max_seq_num

                    # Send an ACK for the received packet
                    ack_packet = self.packet_handler.get_ack_packet(self.next_packet)
                    if(random.random() >= self.drop_prob):
                        self.sock.sendto(ack_packet, addr)
                        if(self.debug):
                            print("Sent ACK:", self.next_packet)
                    else:
                        if(self.debug):
                            print("Simulating ACK loss, ack number:", self.next_packet)
                elif seq_num < self.next_packet:
                    print("Received duplicate packet:", seq_num)

            self.file.close()
            print("File received successfully")

        except socket.timeout:
            self.file.close()
            print("File received successfully")

def main():
    # Create a UDP socket and bind it to the client's IP and port
    output_file = sys.argv[1]
    packet_size = int(sys.argv[2])
    window_size = int(sys.argv[3])

    client_ip = '127.0.0.1'
    client_port = 8301

    sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    sock.bind((client_ip, client_port))
    sock.settimeout(10)

    # Receive packets and write to the output file
    receiver = Receiver(sock, output_file, packet_size, window_size)

if __name__ == '__main__':
    main()