## How to use :
Go-Back N and Selective Repeat :
    python3 receiver.py <output_file> <packet_size> <window_size>
    python3 sender.py <inputfile> <packetsize> <windowsize> <retransmissionTimeout>

    The localhost is used to set up the connection and predetermined ports are used , Make changes to the code to change them
StopandWait :
    python3 receiver.py <output_file> <packet_size> <window_size>
    python3 sender.py <input_file> <packet_size> <rto>


Netem Setup : 
    sudo tc qdisc add dev lo root netem rate ${bandwidth}mbit
    sudo tc qdisc change dev lo root netem loss $loss delay $delay $deviation distribution normal

Once done :
    sudo tc qdisc delete dev lo root netem

In every Folder, Run ./tester.sh to run the tests and statistics are collected in timing_results , Change the RTO and bandwidth according to use.
