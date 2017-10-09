// Licensed under the Apache License, Version 2.0.
// Author: Jin Qing (http://blog.csdn.net/jq0123)

#include "client_async_reader_writer_impl.h"

#include <cassert>

#include "client_async_reader_writer_impl2.h"  // for ClientAsyncReaderWriterImpl2

namespace grpc_cb_core {

// Todo: SyncGetInitMd();

ClientAsyncReaderWriterImpl::ClientAsyncReaderWriterImpl(
    const ChannelSptr& channel, const std::string& method,
    const CompletionQueueSptr& cq_sptr, int64_t timeout_ms,
    const StatusCb& status_cb)
    : impl2_sptr_(new ClientAsyncReaderWriterImpl2(channel, method, cq_sptr,
                                                   timeout_ms, status_cb)) {
  assert(cq_sptr);
}

ClientAsyncReaderWriterImpl::~ClientAsyncReaderWriterImpl() {
  impl2_sptr_->CloseWriting();  // impl2_sptr_ will live on.
}

bool ClientAsyncReaderWriterImpl::Write(const std::string& msg) {
  return impl2_sptr_->Write(msg);
}

void ClientAsyncReaderWriterImpl::CloseWriting() {
  impl2_sptr_->CloseWriting();
}

void ClientAsyncReaderWriterImpl::ReadEach(
    const ClientAsyncReadHandlerSptr& handler_sptr) {
  impl2_sptr_->ReadEach(handler_sptr);
}

void ClientAsyncReaderWriterImpl::SetErrorStatus(const Status& error_status) {
  assert(!error_status.ok());
  impl2_sptr_->SetErrorStatus(error_status);
}

}  // namespace grpc_cb_core