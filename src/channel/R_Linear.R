write_to_agent <- function(key, value, sw) {
    # init the offset
    seek(sw, 0, rw="wb")
    writeBin(as.integer("91"), sw, 4)
    for (i in 1:length(key)) {
        writeBin(as.double(value[i]), sw, 4)
    }
    seek(sw, 4 * 101, rw="wb")
    for (i in 1:length(key)) {
        writeBin(as.integer(key[i]), sw, 4)
    }
    flush(sw)
}

read_from_agent <- function(sr) {
    seek(rw, 0, rw="rb")
    size <- readBin(sr, integer(), 1, 4)
    value = readBin(sr, double(), 91, 4)
    key = readBin(sr, integer(), 91, 4)
    ret <- list(size, value, key)
    return(ret)
}

gradDescent <- function(X, Y, theta, alpha, num_iters, fw, fr, sr, sw) {
    m <- length(Y)
    key <- seq(0, 90, by=1)
    for(i in 1:num_iters) {
# X is a nrows * 91 matrix, t(X) is a 91 * nrows matrix, Y is a nrow * 1 matrix
# so it's a 91 * 1 column vector
        grad <- -alpha * (1 / m) * (t(X)%*%(X%*%theta - Y))
        write_to_agent(key, grad, sw)
        readBin(fr, integer(), 1, 4)
        theta <- theta + grad
    }
    return(theta)
}

# __ main __ 
# read data
load_data <- read.table("YPMSD_train.txt", header=FALSE, sep=",")
X <- load_data[2:91]
X <- cbind(rep(1, nrow(X)), X)  # add a col with 1s at first col, so there will be 91 cols.
Y = load_data[1]
X <- data.matrix(X)
Y <- data.matrix(Y)

# init fifo need to modify the path
# fifo_read may failed if there is not a write fifo open fd exsist.
fifo_write = fifo("/tmp/test_fifo2", "wb", blocking=TRUE)
fifo_read = fifo("/tmp/test_fifo1", "rb", blocking=TRUE)

# init shared memory need to modify the path
shm_read = file("/dev/shm/test_sharedMemory1", "rb")
shm_write = file("/dev/shm/test_sharedMemory2", "wb")


# initial setting
learning_rate <- 0.005
num_iters <- 2
theta <- rep(0, 91) 
results <- gradDescent(X, Y, theta, learning_rate, num_iters, fifo_write, fifo_read, shm_read, shm_write)

print(results)
close(fifo_write)
close(fifo_read)
close(shm_read)
close(shm_write)
