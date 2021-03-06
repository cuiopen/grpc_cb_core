// Licensed under the Apache License, Version 2.0.
// Author: Jin Qing (http://blog.csdn.net/jq0123)

#ifndef GRPC_CB_CORE_SERVICE_SPTR_H
#define GRPC_CB_CORE_SERVICE_SPTR_H

#include <memory>  // for shared_ptr<>

namespace grpc_cb_core {

class Service;
using ServiceSptr = std::shared_ptr<Service>;

}  // namespace grpc_cb_core

#endif  // GRPC_CB_CORE_SERVICE_SPTR_H
