from packet import Packet
import socket
import threading
import time
import sys
import random

class Sender:
    def __init__(self, sock, client_address, input_file, packet_size, window_size, rto, drop_prob=0):
        self.debug = True
        self.drop_prob = drop_prob

        self.sock = sock
        self.client_address = client_address
        self.input_file = input_file
        self.packet_size = packet_size
        self.window_size = window_size
        self.rto = rto

        self.packet_handler = Packet(input_file, packet_size, window_size)
        self.seq_num_size = self.packet_handler.seq_num_size
        self.max_seq_num = self.packet_handler.max_seq_num
        self.num_packets = self.packet_handler.get_num_packets()
        if(self.debug):
            print("Number of packets:", self.num_packets)

        self.min_unacked = 0
        self.next_packet = 0
        self.seq_num = 0
        self.ack_num = 1
        self.send_times = [-1] * self.max_seq_num

        self.base_lock = threading.Lock()
        self.time_lock = threading.Lock()

        self.sock.settimeout(None)

        # Launch two threads. One for sending packets and the other for receiving ACKs and one for checking timeouts
        send_thread = threading.Thread(target=self.send_packets)
        recv_thread = threading.Thread(target=self.receive_acks)
        timeout_thread = threading.Thread(target=self.check_timeouts)

        send_thread.start()
        recv_thread.start()
        timeout_thread.start()

        send_thread.join()
        

    def send_packets(self):
        while(self.min_unacked < self.num_packets):
            self.base_lock.acquire()
            self.time_lock.acquire()
            while(self.next_packet < self.min_unacked + self.window_size and self.next_packet < self.num_packets):
                packet = self.packet_handler.get_file_packet(self.next_packet, self.seq_num)
                if(random.random() >= self.drop_prob):
                    self.sock.sendto(packet, self.client_address)
                else:
                    print("Simulating Packet loss, sequence number:", self.seq_num)
                self.send_times[self.next_packet % self.max_seq_num] = time.time()
                if(self.debug):
                    print("Sent packet:", self.next_packet)
                self.next_packet += 1
                self.seq_num = (self.seq_num + 1) % self.max_seq_num
            self.base_lock.release()
            self.time_lock.release()

        self.sock.close()

    def receive_acks(self):
        while(self.min_unacked < self.num_packets):
            packet, _ = self.sock.recvfrom(self.seq_num_size)
            ack_num = self.packet_handler.read_ack_packet(packet)
            if(self.debug):
                print("Ack received: ", ack_num, " Ack expected: ", ((self.min_unacked + 1) % self.max_seq_num))
            if ((ack_num >= ((self.min_unacked + 1) % self.max_seq_num))
                    or ((self.min_unacked + self.window_size) > self.max_seq_num and ack_num < ((self.min_unacked + self.window_size) % self.max_seq_num))):
                self.base_lock.acquire()
                if(ack_num >= ((self.min_unacked + 1) % self.max_seq_num)):
                    self.min_unacked += ack_num - ((self.min_unacked + 1) % self.max_seq_num) + 1
                else:
                    self.min_unacked += self.max_seq_num - ((self.min_unacked + 1) % self.max_seq_num) + ack_num + 1
                if(self.debug):
                    print("Min unacked is ", self.min_unacked)
                self.base_lock.release()

    def check_timeouts(self):
        while(self.min_unacked < self.num_packets):
            self.base_lock.acquire()
            self.time_lock.acquire()
            min_send_time = self.send_times[self.min_unacked % self.max_seq_num]
            if(min_send_time != -1 and time.time() - min_send_time >= self.rto):
                if(self.debug):
                    print("RTO triggered for ", self.min_unacked % self.max_seq_num, ". min send time: ", min_send_time, "time: ", time.time(), "rto: ", self.rto)
                self.next_packet = self.min_unacked
                self.seq_num = self.min_unacked % self.max_seq_num
                self.send_times[self.min_unacked % self.max_seq_num] = -1
                if(self.debug):
                    print("Resending packets from:", self.next_packet)
            self.base_lock.release()
            self.time_lock.release()

def main():
    input_file = sys.argv[1]
    packet_size = int(sys.argv[2])
    drop_prob = float(sys.argv[3])
    rto = float(sys.argv[4]) / 1000.0

    server_ip = '127.0.0.1'
    server_port = 8299

    client_ip = '127.0.0.1'
    client_port = 9000

    client_address = (client_ip, client_port)

    sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    sock.bind((server_ip, server_port))
    #Stop and wait is a special case of sliding window with window size 1
    sender = Sender(sock, client_address, input_file, packet_size, 1, rto,drop_prob)

if __name__ == '__main__':
    main()