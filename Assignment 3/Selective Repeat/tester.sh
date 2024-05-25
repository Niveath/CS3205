#!/bin/bash

# Input file
input_file="loco.jpg"

# Define the output file
timing_file="timing_results.txt"

# Define the ranges for loss and delay
window_size=8
rto=200
packet_size=25600

bandwidth=800
loss_values=("0.1%" "0.5%" "1%" "1.5%" "2%" "5%")  # Example loss values in percentage (0.1% to 1%)
delay_values=("50ms" "100ms" "150ms" "200ms" "250ms" "500ms")  # Example delay values
deviation=10ms

# Loop through each combination of loss and delay
for loss in "${loss_values[@]}"; do
    for delay in "${delay_values[@]}"; do
        # Set max bandwidth
        echo "Setting max bandwidth to ${bandwidth}Kbit"
        sudo tc qdisc add dev eth0 root tbf rate ${bandwidth}Kbit

        # Set loss and delay using netem
        echo "Setting loss=$loss and delay=$delay using netem..."
        sudo tc qdisc add dev eth0 root netem loss $loss delay $delay $deviation distribution normal

        echo "Running Python code with loss=$loss and delay=$delay..."
        # Run receiver.py in the background and time its execution
        output_file="output_loss=${loss}_delay=${delay}.jpg"
        { time python3 -u receiver.py "$output_file" "$packet_size" "$window_size"; } > /dev/null 2> temp.txt &

        # Run sender.py in the background and redirect output to /dev/null
        python3 sender.py "$input_file" "$packet_size" "$window_size" "$rto" >/dev/null 2>&1 &

        echo "Waiting for the Python code to finish..."

        wait

        # Extract total time from the temporary file
        total_time=$(grep "real" temp.txt | awk '{print $2}')

        # Write loss, delay, and time taken to the output file
        echo "Loss: $loss, Delay: $delay, Time taken: $total_time" >> "$timing_file"

        # Remove netem configuration after running the code
        sudo tc qdisc del dev eth0 root netem

        # Remove the temporary file
        rm temp.txt
    done
done
