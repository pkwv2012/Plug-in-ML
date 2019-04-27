"""This module is used to evaluate the system."""
# coding: utf-8

import sys
import os
import mmap
import struct
import json
import time
import argparse

from datetime import datetime
import numpy as np
from sklearn.metrics import mean_absolute_error
from sklearn.metrics import mean_squared_error

def read_from_agent(m):
    """Read from shared memory."""
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
    return np.asarray(coef), np.asarray(keys)

def transfer_to_agent(values, keys, w):
    """Write to shared memory."""
    w.seek(0)
    w.write(struct.pack('i', len(keys)))
    # print values
    for v in values:
        w.write(struct.pack('f', v))
    w.seek(4*(101)) #________________________________________________________
    for k in keys:
        w.write(struct.pack('i', k))

def load_data(file_path):
    """Loading the data from file."""
    data = list()
    with open(file_path, 'r') as fin:
        for line in fin:
            item = [float(i) for i in line.split(',')]
            data.append(item)
    data = np.asarray(data)
    return data[:, 1:], data[:, 0]

def next_batch(X, y, batch_size):
    """Yeild next batch of the training data."""
    n, cursor = len(y), 0
    permutaion = np.random.permutation(n)
    while True:
        if cursor + batch_size <= n:
            indices = permutaion[cursor: cursor + batch_size]
            cursor += batch_size
        else:
            next_cursor = batch_size - (n - cursor)
            indices = permutaion[cursor:]
            permutaion = np.random.permutation(n)
            indices = np.append(indices, permutaion[:next_cursor])
            cursor = next_cursor
        batch_X, batch_y = X[indices], y[indices]
        yield batch_X, batch_y

def calc_g(X, Y, w):
    """Calculate the gradient and Square loss."""
    alpha = 0.1
    n = len(X)
    multi_thread = False
    if multi_thread:
        y_pred = X.dot(w)
        loss = np.mean(0.5 * (y_pred - Y) ** 2) + 0.5 * alpha * w.dot(w)
        w_grad = X.T.dot(y_pred - Y) / n + alpha * w
    else:
        y_pred = np.zeros(X.shape[0])
        for i in range(X.shape[0]):
            for j in range(len(X[i])):
                y_pred[i] += X[i][j] * w[j]
        loss = 0
        for i in range(n):
            loss += 0.5 * (y_pred[i] - Y[i]) ** 2
        loss /= n
        for i in range(len(w)):
            loss += 0.5 * alpha * w[i] ** 2

        w_grad = np.zeros(len(w))
        for i in range(len(w)):
            for j in range(X.shape[0]):
                w_grad[i] += (y_pred[j] - Y[j]) * X[j][i]
            w_grad[i] /= n
        w_grad[i] += alpha * w[i]
    return loss, w_grad

def predict(X, Y, w):
    """Predict using the model."""
    y_pred = X.dot(w)
    absolute_error = mean_absolute_error(Y * (2001 - 1992) + 1992, y_pred * (2001 - 1992) + 1992)
    squared_error = mean_squared_error(Y * (2001 - 1992) + 1992, y_pred * (2001 - 1992) + 1992)
    return absolute_error, squared_error

if __name__ == '__main__':
    alone = True
    if len(sys.argv) >= 2:
        num_worker = int(sys.argv[1])
        alone = False
    else:
        num_worker = 1
    data_path = '../../YearPredictionMSD.txt'
    train_file = '{}.train'.format(data_path)
    eval_file = '{}.eval'.format(data_path)
    num_features = None
    num_iter = 50
    # num_worker = 1 if alone else 3
    batch_size = int(400000 / num_worker)
    learning_rate = 0.0000001

    if not alone:
        shm_read_file = '/dev/shm/test_sharedMemory_sample1'
        shm_write_file = '/dev/shm/test_sharedMemory_sample2'
        read_fifo_path = '/tmp/test_fifo_sample1'
        write_fifo_path = '/tmp/test_fifo_sample2'

        while not os.path.exists(read_fifo_path) or not os.path.exists(write_fifo_path) \
            or not os.path.exists(shm_read_file) or not os.path.exists(shm_write_file):
            print('waiting for FIFO')
            time.sleep(1)

        read_fd = os.open('/dev/shm/test_sharedMemory_sample1', os.O_RDWR | os.O_SYNC)
        write_fd = os.open('/dev/shm/test_sharedMemory_sample2', os.O_RDWR | os.O_SYNC)

        read_fifo = os.open(read_fifo_path, os.O_SYNC | os.O_RDWR)
        write_fifo = os.open(write_fifo_path, os.O_SYNC | os.O_RDWR)
        w = mmap.mmap(write_fd, 804, mmap.MAP_SHARED, mmap.PROT_WRITE)
        m = mmap.mmap(read_fd, 804, access=mmap.ACCESS_READ)

    train_X, train_y = load_data(train_file)
    eval_X, eval_y = load_data(eval_file)
    train_y = (train_y - 1992) / (2001 - 1992)
    eval_y = (eval_y - 1992) / (2001 - 1992)
    num_features = train_X.shape[1]
    # add one column
    train_X = np.append(train_X, np.ones(shape=[len(train_X), 1]), 1)
    eval_X = np.append(eval_X, np.ones(shape=[len(eval_X), 1]), 1)

    print('Loaded data finished, train={}, eval={}'.format(len(train_X), len(eval_X)))

    iter_batch = next_batch(train_X, train_y, batch_size)

    # w = np.zeros(num_features + 1)
    if alone:
        weights = np.zeros(num_features + 1)
    iteration, cnt = 0, 0
    min_absolute_error, min_squared_error = float('inf'), float('inf')
    start_time = datetime.now()
    status = list()
    for batch_X, batch_y in iter_batch:
        if not alone:
            weights = [0 for i in range(num_features + 1)]
            keys = [i for i in range(num_features + 1)]
            transfer_to_agent(weights, keys, w)
            os.write(write_fifo, struct.pack('i', 0))
            os.read(read_fifo, 4)
            weights, keys = read_from_agent(m)
        loss, grad = calc_g(batch_X, batch_y, weights)
        print(iteration, loss, grad[:10])
        # w -= learning_rate * grad
        if alone:
            weights -= learning_rate * grad
        else:
            with open('grad.log', 'a') as fout:
                for g in -learning_rate*grad:
                    fout.write(str(g) + ',')
                fout.write('\n')
            transfer_to_agent(-learning_rate*grad, keys, w)
            os.write(write_fifo, struct.pack('i', 1))
            weights -= learning_rate * grad

        # if not alone:
        #     time.sleep(2)
        #     weights = [0 for i in range(num_features + 1)]
        #     keys = [i for i in range(num_features + 1)]
        #     transfer_to_agent(weights, keys, w)
        #     os.write(write_fifo, struct.pack('i', 0))
        #     os.read(read_fifo, 4)
        #     weights, keys = read_from_agent(m)
        absolute_error, squared_error = predict(eval_X, eval_y, weights)
        min_absolute_error = min(min_absolute_error, absolute_error)
        min_squared_error = min(min_squared_error, squared_error)
        cur_time = datetime.now()
        status.append(((cur_time - start_time).total_seconds(),
                       min_absolute_error,
                       min_squared_error))
        print(absolute_error, squared_error)
        print(weights[:10])
        iteration += 1
        if iteration > num_iter:
            break
        cnt += 1
        if cnt >= 1000:
            cnt = 0
            learning_rate /= 2
    print(min_absolute_error, min_squared_error)
    if not alone:
        os.write(write_fifo, struct.pack('i', 2))

    with open('status', 'w') as fout:
        fout.write(json.dumps(status))
