import os
import time
from subprocess import call
from time import sleep


call("mkdir time_results",shell=True)
call("mkdir gen_results",shell=True)


call("rm time_results/*.txt",shell=True)
call("rm gen_results/*.txt",shell=True)


nodefiles = os.listdir("nodeFiles")
print (nodefiles)
threads = [3];
for file in nodefiles:
  for thread in threads:
        filename = "nodeFiles/"+file
        command = "./bin/assignment_4 -n "+filename +" -t "+str(thread)
        call(command,shell=True)

call("mv time_results.txt time_results/",shell=True)
call("mv *results.txt gen_results/",shell=True)
