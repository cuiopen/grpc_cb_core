// Licensed under the Apache License, Version 2.0.
// Author: Jin Qing (http://blog.csdn.net/jq0123)

#ifndef GRPC_CB_CLIENT_CLIENT_ASYNC_WRITER_SEND_MSG_CQTAG_H
#define GRPC_CB_CLIENT_CLIENT_ASYNC_WRITER_SEND_MSG_CQTAG_H

#include <functional>  // for function

#include <grpc_cb/impl/client/client_send_msg_cqtag.h>  // for ClientSendMsgCqTag
#include <grpc_cb/support/config.h>                     // for GRPC_FINAL

namespace grpc_cb {

class ClientAsyncWriterSendMsgCqTag GRPC_FINAL : public ClientSendMsgCqTag {
 public:
  explicit ClientAsyncWriterSendMsgCqTag(const CallSptr& call_sptr);

  using OnWritten = std::function<void()>;
  void SetOnWritten(const OnWritten& on_written) { on_written_ = on_written; }

  void DoComplete(bool success) GRPC_OVERRIDE;

 private:
  OnWritten on_written_;
};  // class ClientAsyncWriterSendMsgCqTag

}  // namespace grpc_cb
#endif  // GRPC_CB_CLIENT_CLIENT_ASYNC_WRITER_SEND_MSG_CQTAG_H