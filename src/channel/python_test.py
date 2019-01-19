import os

write_path = "/tmp/python_fifo"

wf = os.open(write_path, os.O_SYNC | os.O_CREAT | os.O_RDWR)

len_send = os.write(wf, "1")

os.close(wf)
