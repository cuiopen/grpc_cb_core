// Licensed under the Apache License, Version 2.0.
// Author: Jin Qing (http://blog.csdn.net/jq0123)

#ifndef GRPC_CB_CORE_CLIENT_ASYNC_READER_WRITER_H
#define GRPC_CB_CORE_CLIENT_ASYNC_READER_WRITER_H

#include <cassert>
#include <cstdint>  // for int64_t
#include <string>

#include <grpc_cb_core/impl/client/client_async_read_handler.h>  // for ClientAsyncReadHandler
#include <grpc_cb_core/impl/client/client_async_reader_writer_impl.h>  // for ClientAsyncReaderWriterImpl<>
#include <grpc_cb_core/client/status_callback.h>  // for StatusCallback
#include <grpc_cb_core/support/config.h>   // for GRPC_FINAL

namespace grpc_cb_core {

// Copyable. Thread-safe.
class ClientAsyncReaderWriter GRPC_FINAL {
 public:
  // Todo: Move on_status to Set()
  ClientAsyncReaderWriter(const ChannelSptr& channel, const std::string& method,
                          const CompletionQueueSptr& cq_sptr,
                          int64_t timeout_ms,
                          const StatusCallback& on_status = StatusCallback())
      : impl_sptr_(new Impl(channel, method, cq_sptr, timeout_ms, on_status)) {
    assert(cq_sptr);
    assert(channel);
  }

 public:
  bool Write(const std::string& request) const {
    return impl_sptr_->Write(request);
  }

  // Optional. Writing is auto closed in dtr().
  // Redundant calls are ignored.
  void CloseWriting() {
    impl_sptr_->CloseWriting();
  }

  using OnRead = std::function<void(const std::string&)>;
  void ReadEach(const OnRead& on_read) {
    auto handler_sptr = std::make_shared<ClientAsyncReadHandler>(on_read);
    impl_sptr_->ReadEach(handler_sptr);
  }

 private:
  using Impl = ClientAsyncReaderWriterImpl;
  const std::shared_ptr<Impl> impl_sptr_;
};  // class ClientAsyncReaderWriter<>

}  // namespace grpc_cb_core

#endif  // GRPC_CB_CORE_CLIENT_ASYNC_READER_WRITER_H
