// Licensed under the Apache License, Version 2.0.
// Author: Jin Qing (http://blog.csdn.net/jq0123)

#ifndef GRPC_CB_IMPL_MESSAGE_QUEUE_H
#define GRPC_CB_IMPL_MESSAGE_QUEUE_H

#include <queue>

#include <grpc_cb_core/impl/message_sptr.h>  // for MessageSptr

namespace grpc_cb_core {

using MessageQueue = std::queue<MessageSptr>;

}  // namespace grpc_cb_core

#endif  // GRPC_CB_IMPL_CALL_SPTR_H
