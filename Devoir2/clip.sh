#!/bin/bash

ssh -e none mpiuser26@dim-openmpi0.uqac.ca "cat ~/Devoir2/testresults/rangetest.log" | xclip -selection clipboard