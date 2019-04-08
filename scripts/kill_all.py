#!/usr/bin/python3
"""This module is used to kill all the process invoked by the RPSCC."""
# coding: utf-8

import subprocess
import io
import os
import signal
import sys
import time


proc = subprocess.Popen(['ps', 'aux'], stdout=subprocess.PIPE)
pid_list = list()
for out in io.TextIOWrapper(proc.stdout, encoding="utf-8"):
    out = str(out)
    if ('master_main' in out \
        or 'server_main' in out \
        or 'agent_main' in out \
        or r'python3 linear' in out) \
        and 'grep' not in out:
        pid = out.split()[1]
        print(pid)
        pid_list.append(pid)

proc.stdout.close()

for pid in pid_list:
    os.kill(int(pid), signal.SIGKILL)  # or signal.SIGKILL
    # check_call will failed.
    # subprocess.check_call(['kill', '-9 %s' % pid], shell=True)

time.sleep(3)

# remove shared memory files.
try:
    # subprocess.check_call('rm /dev/shm/test_sharedMemory_sample*', shell=True)
    print('useless remove')
except subprocess.CalledProcessError:
    print('rm shared memory error')
try:
    subprocess.check_call('rm /tmp/test_fifo_sample*', shell=True)
    print('useless')
except subprocess.CalledProcessError:
    print('rm fifo error')

