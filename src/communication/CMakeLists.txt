
add_library(zmq_communicator communicator.h zmq_communicator.h zmq_communicator.cc fifo_ring.cc fifo_ring.h)
target_link_libraries(zmq_communicator pthread zmq)