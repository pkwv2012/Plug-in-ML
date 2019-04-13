
add_library(zmq_communicator zmq_communicator.cc fifo_ring.cc zmq_sendrecv.cc communicator.h)
target_link_libraries(zmq_communicator pthread zmq gflags logging)