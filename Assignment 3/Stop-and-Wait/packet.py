import math

class Packet:
    def __init__(self, input_file, packet_size, window_size, sender=True):
        self.input_file = input_file
        self.packet_size = packet_size
        self.window_size = window_size
        self.sender = sender

        self.seq_num_size = math.ceil(math.log2(self.window_size + 1) / 8)
        self.max_seq_num = 2 ** (self.seq_num_size * 8)
        self.packets = []
        if(self.sender):
            self.split_file()

    def split_file(self):
        # Read the input file packet_size number of bytes at a time and generate packets
        with open(self.input_file, 'rb') as file:
            while True:
                data = file.read(self.packet_size)
                if not data:
                    break
                self.packets.append(data)
    
    def get_file_packet(self, index, seq_num):
        # Create a packet with the given data and sequence number
        return seq_num.to_bytes(self.seq_num_size, 'big') + self.packets[index]
    
    def get_ack_packet(self, seq_num):
        # Create a packet with the given sequence number
        return seq_num.to_bytes(self.seq_num_size, 'big')

    def read_data_packet(self, packet):
        # Read the sequence number and data from the packet
        seq_num = int.from_bytes(packet[:self.seq_num_size], 'big')
        data = packet[self.seq_num_size:]
        return seq_num, data
    
    def read_ack_packet(self, packet):
        # Read the sequence number from the packet
        return int.from_bytes(packet, 'big')
    
    def get_num_packets(self):
        if(self.sender):
            return len(self.packets)