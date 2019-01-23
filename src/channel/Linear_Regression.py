# Gradient descent algorithm for linear regression
from numpy import *
import os
import struct
import mmap

# read from shared memory
def readFromAgent(m):
    m.seek(0)
    c = m.read(4)
    sz = struct.unpack('i', c)[0]
    coef = []
    keys = []
    for i in range(sz):
        c = m.read(4)
        coef.append(struct.unpack('f', c)[0])
    for i in range(100-sz):
        c = m.read(4)
    for i in range(sz):
        c = m.read(4)
        keys.append(struct.unpack('i', c)[0])
    return coef, keys

# write to shared memory
def transfer2Agent(values, keys, w):
    w.seek(0)
    w.write(struct.pack('i', len(values)))
    # print values
    for v in values:
        w.write(struct.pack('f', v))
    w.seek(4*(101)) #________________________________________________________
    for k in keys:
        w.write(struct.pack('i', k))

# minimize the "sum of squared errors". This is how we calculate and correct our error
def compute_error_for_line_given_points(b, m, points):
    totalError = 0      #sum of square error formula
    for i in range (0, len(points)):
        x = points[i, 0]
        y = points[i, 1]
        totalError += (y-(m*x + b)) ** 2
    return totalError/ float(len(points))

def step_gradient(read_fifo, write_fifo, m, w, b_current, m_current, points, learning_rate):
    #gradient descent
    b_gradient = 0
    m_gradient = 0
    N = float(len(points))
    for i in range(0, len(points)):
        x = points[i, 0]
        y = points[i, 1]
        b_gradient += -(2/N) * (y - (m_current * x + b_current))
        m_gradient += -(2/N) * x * (y - (m_current * x + b_current))
    # new_b = b_current - (learning_rate * b_gradient)
    # new_m = m_current - (learning_rate * m_gradient)
    transfer2Agent([b_gradient, m_gradient], [0, 1], w)
    os.write(write_fifo, struct.pack('i', 1))
    os.read(read_fifo, 4);
    coef, keys = readFromAgent(m)
    # print "python "
    # print b_gradient
    # print m_gradient
    # print coef
    # print keys
    new_b = coef[0]
    new_m = coef[1]
    return [new_b,new_m]

def gradient_descent_runner(read_fifo, write_fifo, m, w, points, starting_b, starting_m, learning_rate, num_iteartions):
    b_current = starting_b
    m_current = starting_m
    for i in range(num_iteartions):
        print i
        b_current, m_current = step_gradient(read_fifo, write_fifo, m, w, b_current, m_current, array(points), learning_rate)
    return [b_current, m_current]

def run(read_fifo, write_fifo, m, w):
    # Step 1: Collect the data
    points = genfromtxt('data.csv', delimiter=',')
    # Step 2: Define our Hyperparameters
    learning_rate = 0.0001 #how fast the data converge
    # write the key the data have
    transfer2Agent([0, 0], [0, 1], w)
    # signal to the agent
    os.write(write_fifo, struct.pack('i', 0))
    # wait agent
    os.read(read_fifo, 4)
    # read the initalize parameters from agent
    coef, keys = readFromAgent(m)
    # y=mx+b (Slope formule)
    initial_b = coef[0] # initial y-intercept guess
    initial_m = coef[1] # initial slope guess
    num_iterations = 1000
    print("Starting gradient descent at b = {0}, m = {1}, error = {2}".format(initial_b, initial_m, compute_error_for_line_given_points(initial_b, initial_m, points)))
    print("Running...")
    [b, m] = gradient_descent_runner(read_fifo, write_fifo, m, w, points, initial_b, initial_m, learning_rate, num_iterations)
    print("After {0} iterations b = {1}, m = {2}, error = {3}".format(num_iterations, b, m, compute_error_for_line_given_points(b, m, points)))
    os.write(write_fifo, struct.pack('i', 2))

# main function
if __name__ == "__main__":
    # read_fd = os.open('/dev/shm/shm_w_name', os.O_RDWR | os.O_SYNC | os.O_CREAT)
    # write_fd = os.open('/dev/shm/shm_r_name', os.O_RDWR | os.O_SYNC | os.O_CREAT)

    # read_fifo_path = '/tmp/lr_w_fifo'
    # write_fifo_path = '/tmp/lr_r_fifo'

    read_fd = os.open('/dev/shm/test_sharedMemory_sample1', os.O_RDWR | os.O_SYNC | os.O_CREAT)
    write_fd = os.open('/dev/shm/test_sharedMemory_sample2', os.O_RDWR | os.O_SYNC | os.O_CREAT)
    read_fifo_path = '/tmp/test_fifo_sample1'
    write_fifo_path = '/tmp/test_fifo_sample2'

    read_fifo = os.open(read_fifo_path, os.O_SYNC | os.O_CREAT | os.O_RDWR)
    write_fifo = os.open(write_fifo_path, os.O_SYNC | os.O_CREAT | os.O_RDWR)

    m = mmap.mmap(read_fd, 804, access=mmap.ACCESS_READ)
    w = mmap.mmap(write_fd, 804, mmap.MAP_SHARED, mmap.PROT_WRITE)

    run(read_fifo, write_fifo, m, w)
