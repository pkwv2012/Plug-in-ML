//
// Created by PeikaiZheng on 2019/4/13.
//

#include "src/util/logging.h"

namespace rpscc {
// There is no need to have a mutex,
// because std::cerr::operator<< is thread-safe
// std::mutex LogMessage::cerr_mutex;
}  // namespace rpscc

