// Licensed under the Apache License, Version 2.0.
// Author: Jin Qing (http://blog.csdn.net/jq0123)

#ifndef GRPC_CB_CORE_SERVER_WRITER_H
#define GRPC_CB_CORE_SERVER_WRITER_H

#include <cassert>
#include <string>

#include <grpc_cb_core/impl/server/server_writer_impl.h>  // for ServerWriterImpl

namespace grpc_cb_core {

// Copyable. Thread-safe.
class ServerWriter GRPC_FINAL {
 public:
  explicit ServerWriter(const CallSptr& call_sptr)
      : impl_sptr_(new ServerWriterImpl(call_sptr)) {
    assert(call_sptr);
  }

 public:
  bool Write(const std::string& response) const {
    return impl_sptr_->Write(response);
  }
  bool SyncWrite(const std::string& response) const {
    return impl_sptr_->SyncWrite(response);
  }
  void AsyncWrite(const std::string& response) const {
    impl_sptr_->AsyncWrite(response);
  }

  size_t GetQueueSize() const {
    return impl_sptr_->GetQueueSize();
  }
  size_t GetHighQueueSize() const {
    return impl_sptr_->GetHighQueueSize();
  }
  void SetHighQueueSize(size_t high_queue_size) {
    impl_sptr_->SetHighQueueSize(high_queue_size);
  }

  // Close() is optional. Dtr() will auto SyncClose().
  // Redundant close will be ignored.
  void SyncClose(const Status& status) const {
    impl_sptr_->SyncClose(status);
  }
  void AsyncClose(const Status& status) const {
    impl_sptr_->AsyncClose(status);
  }
  bool IsClosed() const { return impl_sptr_->IsClosed(); }

 private:
  const std::shared_ptr<ServerWriterImpl> impl_sptr_;
};  // class ServerWriter<>

}  // namespace grpc_cb_core

#endif  // GRPC_CB_CORE_SERVER_WRITER_H