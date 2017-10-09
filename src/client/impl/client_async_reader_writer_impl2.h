// Licensed under the Apache License, Version 2.0.
// Author: Jin Qing (http://blog.csdn.net/jq0123)

#ifndef GRPC_CB_CORE_CLIENT_CLIENT_ASYNC_READER_WRITER_IMPL2_H
#define GRPC_CB_CORE_CLIENT_CLIENT_ASYNC_READER_WRITER_IMPL2_H

#include <cassert>
#include <cstdint>  // for int64_t
#include <memory>  // for enable_shared_from_this<>
#include <mutex>
#include <string>

#include <grpc_cb_core/client/channel_sptr.h>  // for ChannelSptr
#include "client_async_read_handler_sptr.h"  // for ClientAsyncReadHandlerSptr
#include <grpc_cb_core/client/status_cb.h>  // for StatusCb
#include <grpc_cb_core/common/call_sptr.h>   // for CallSptr
#include <grpc_cb_core/common/completion_queue_sptr.h>  // for CompletionQueueSptr
#include <grpc_cb_core/common/status.h>                      // for Status
#include <grpc_cb_core/common/support/config.h>              // for GRPC_FINAL

namespace grpc_cb_core {

class ClientAsyncReaderHelper;
class ClientAsyncWriterHelper;

// Impl of impl.
// Impl1 is to make Writer copyable.
// Impl2 will live longer than the Writer.
// We need dtr() of Impl1 to close writing.
// Thread-safe.
class ClientAsyncReaderWriterImpl2 GRPC_FINAL
    : public std::enable_shared_from_this<ClientAsyncReaderWriterImpl2> {
 public:
  ClientAsyncReaderWriterImpl2(const ChannelSptr& channel,
                               const std::string& method,
                               const CompletionQueueSptr& cq_sptr,
                               int64_t timeout_ms,
                               const StatusCb& status_cb);
  ~ClientAsyncReaderWriterImpl2();

 public:
  bool Write(const std::string& msg);
  // CloseWriting() is optional. Auto closed on dtr().
  void CloseWriting();

  // Todo: Force to close reading/writing. Cancel all reading/writing.

  using ReadHandlerSptr = ClientAsyncReadHandlerSptr;
  void ReadEach(const ReadHandlerSptr& handler_sptr);

  // Set error status to break reading. Such as when parsing message failed.
  void SetErrorStatus(const Status& error_status) {
    assert(!error_status.ok());
    status_ = error_status;
  }

 private:
  // Callback of ReaderHelper.
  void OnEndOfReading();
  // Callback of WriterHelper.
  void OnEndOfWriting();

 private:
  void SendCloseIfNot();
  void SetInternalError(const std::string& sError);
  bool IsReadingEnded() const;
  bool IsWritingEnded() const;
  void CallStatusCb();

 private:
  // Callbacks will lock again.
  using Mutex = std::recursive_mutex;
  mutable Mutex mtx_;
  using Guard = std::lock_guard<Mutex>;

  const CallSptr call_sptr_;
  Status status_;

  ReadHandlerSptr read_handler_sptr_;
  StatusCb status_cb_;

  bool reading_started_ = false;  // ReadEach() to trigger reading.
  bool writing_started_ = false;  // Write() to trigger writing.

  bool has_sent_close_ = false;  // Client send close once.

  // Helper will be shared by CqTag.
  std::shared_ptr<ClientAsyncReaderHelper> reader_sptr_;
  std::shared_ptr<ClientAsyncWriterHelper> writer_sptr_;
};  // class ClientAsyncReaderWriterImpl2

// Todo: SyncGetInitMd();

}  // namespace grpc_cb_core

#endif  // GRPC_CB_CORE_CLIENT_CLIENT_ASYNC_READER_WRITER_IMPL2_H