#!/bin/bash

PIN=0
SPEED=$1

gpio mode $PIN

echo "Sending $SPEED pulses"
for i in $(seq 1 $SPEED);
do
   gpio $PIN 1
   sleep 0.005
   gpio $PIN 0
   sleep 0.005
done
echo "Done"
