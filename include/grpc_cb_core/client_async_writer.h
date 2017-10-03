// Licensed under the Apache License, Version 2.0.
// Author: Jin Qing (http://blog.csdn.net/jq0123)

#ifndef GRPC_CB_CORE_CLIENT_ASYNC_WRITER_H
#define GRPC_CB_CORE_CLIENT_ASYNC_WRITER_H

#include <cassert>  // for assert()
#include <cstdint>  // for int64_t
#include <string>

#include <grpc_cb_core/impl/channel_sptr.h>  // for ChannelSptr
#include <grpc_cb_core/impl/client/client_async_writer_close_handler.h>  // for ClientAsyncWriterCloseHandler
#include <grpc_cb_core/impl/client/client_async_writer_impl.h>  // for ClientAsyncWriterImpl
#include <grpc_cb_core/impl/completion_queue_sptr.h>  // for CompletionQueueSptr
#include <grpc_cb_core/support/config.h>              // for GRPC_OVERRIDE
#include <grpc_cb_core/support/protobuf_fwd.h>        // for Message

namespace grpc_cb_core {

class Status;

// Copyable. Thread-safe.
// Use template class instead of template member function
//    to ensure client input the correct request type.
template <class Request, class Response>
class ClientAsyncWriter GRPC_FINAL {
 public:
  inline ClientAsyncWriter(const ChannelSptr& channel,
                           const std::string& method,
                           const CompletionQueueSptr& cq_sptr,
                           int64_t timeout_ms)
      // Todo: same as ClientReader?
      : impl_sptr_(new ClientAsyncWriterImpl(channel, method, cq_sptr,
                                             timeout_ms)) {
    assert(channel);
    assert(cq_sptr);
  }

  // Todo: Get queue size()
  // Todo: SyncGetInitMd();

  bool Write(const Request& request) const {
    auto sptr = std::make_shared<Request>(request);
    return impl_sptr_->Write(sptr);
  }

  using ClosedCallback = std::function<void (const Status&, const Response&)>;
  void Close(const ClosedCallback& on_closed = ClosedCallback()) {
    auto handler = std::make_shared<CloseHandler>(on_closed);
    impl_sptr_->Close(handler);
  }  // Close()

  // Todo: Use a default CloseHandler if no Close()?

 private:
  // Use CloseHandler to make impl non-template.
  class CloseHandler GRPC_FINAL : public ClientAsyncWriterCloseHandler {
   public:
    explicit CloseHandler(const ClosedCallback& on_closed = ClosedCallback())
        : on_closed_(on_closed){};
    Message& GetMsg() GRPC_OVERRIDE { return msg_; }
    void OnClose(const Status& status) GRPC_OVERRIDE {
      if (on_closed_) on_closed_(status, msg_);
    }

   private:
    Response msg_;
    ClosedCallback on_closed_;
  };

 private:
  // Use non_template class as the implement.
  const std::shared_ptr<ClientAsyncWriterImpl> impl_sptr_;  // Easy to copy.
};  // class ClientAsyncWriter<>

}  // namespace grpc_cb_core

#endif  // GRPC_CB_CORE_CLIENT_ASYNC_WRITER_H