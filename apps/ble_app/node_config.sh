#!/bin/bash

printf "\nconfig beacon/uuid 12345678901234567890123456789012\n" > /dev/ttyACM0
sleep 1
printf "config beacon/major 1\n" > /dev/ttyACM0
sleep 1
printf "config beacon/minor 2\n" > /dev/ttyACM0
sleep 1
printf "config beacon/tx_power -65\n" > /dev/ttyACM0
sleep 1
printf "config save\n" > /dev/ttyACM0
sleep 1
printf "reset\n" > /dev/ttyACM0

