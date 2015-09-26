#!/usr/bin/env python3
import os
import time
from subprocess import call
from time import sleep

call("mkdir -p time_results", shell=True)
call("mkdir -p gen_results", shell=True)

call("rm -f time_results/*.txt", shell=True)
call("rm -f gen_results/*.bin", shell=True)

nodefiles = os.listdir("nodeFiles")
print(nodefiles)
threads = [3]
for file in nodefiles:
    for thread in threads:
        filename = "nodeFiles/" + file
        command = "./bin/assignment_4 -n " + filename + " -t " + str(thread)
        call(command, shell=True)

call("mv time_results.txt time_results/", shell=True)
call("mv *results.bin gen_results/", shell=True)
