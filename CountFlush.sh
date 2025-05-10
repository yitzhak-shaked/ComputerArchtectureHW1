#!/bin/bash

input_file="$1"
output_file="$2"

echo "Input file: $input_file"
echo "Output file: $output_file"
echo "Counting lines with N and T in input and output files..."

# paste "$input_file" "$output_file" | awk '{
#     in_line = $1
#     out_line = $2
#     if (((in_line ~ /N/) && (out_line !~ /T/)) || ((in_line ~ /T/) && (out_line !~ /N/))) count++
# } END { print (count ? count : 0) }'

# paste "$input_file" "$output_file" | awk '{
#     in_type = $2
#     out_type = $5
#     if (((in_type == "N") && (out_type != "T")) || ((in_type == "T") && (out_type != "N"))) count++
# } END { print (count ? count : 0) }'

paste "$input_file" "$output_file" | awk '{
    if ($2 != $5) count++
} END { print (count ? count : 0) }'
