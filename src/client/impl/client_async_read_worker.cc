// Licensed under the Apache License, Version 2.0.
// Author: Jin Qing (http://blog.csdn.net/jq0123)

#include "client_async_read_worker.h"

#include <cassert>  // for assert()

#include "client_reader_read_cqtag.h"  // for ClientReaderReadCqTag

namespace grpc_cb_core {

ClientAsyncReadWorker::ClientAsyncReadWorker(CallSptr call_sptr,
    const MsgStrCb& msg_cb, const EndCb& end_cb)
    : call_sptr_(call_sptr),
      msg_cb_(msg_cb),
      end_cb_(end_cb) {
  assert(call_sptr);
}

ClientAsyncReadWorker::~ClientAsyncReadWorker() {}

// Setup to read each.
void ClientAsyncReadWorker::Start() {
  Guard g(mtx_);
  if (started_) return;
  started_ = true;
  Next();
}

void ClientAsyncReadWorker::Abort() {
  Guard g(mtx_);
  if (aborted_) return;
  aborted_ = true;
  // to stop circular sharing
  msg_cb_ = nullptr;
  end_cb_ = nullptr;
}  // Abort()

// Return copy for thread-safety.
const Status ClientAsyncReadWorker::GetStatus() const {
  Guard g(mtx_);
  return status_;
}

// Setup next async read.
void ClientAsyncReadWorker::Next() {
  Guard g(mtx_);
  assert(started_);
  if (aborted_)  // Maybe writer failed.
    return;

  auto* tag = new ClientReaderReadCqTag(call_sptr_);
  auto sptr = shared_from_this();
  tag->SetCompleteCb([sptr, tag](bool success) {
      sptr->OnRead(success, *tag);
  });
  if (tag->Start()) return;

  delete tag;
  status_.SetInternalError("Failed to async read server stream.");
  End();
}  // Next()

void ClientAsyncReadWorker::OnRead(bool success, ClientReaderReadCqTag& tag) {
  Guard g(mtx_);
  if (aborted_)  // Maybe writer failed.
    return;
  assert(status_.ok());
  if (!success) {
    status_.SetInternalError("ClientReaderReadCqTag failed.");
    End();
    return;
  }
  if (!tag.HasGotMsg()) {
    // End of read. Do not recv status in Reader. Do it after all reading and writing.
    End();
    return;
  }

  std::string sMsg;
  status_ = tag.GetResultMsg(sMsg);
  if (!status_.ok()) {
    End();
    return;
  }

  if (msg_cb_) {
    status_ = msg_cb_(sMsg);
    if (!status_.ok()) {
      End();
      return;
    }
  }

  Next();
}  // OnRead()

void ClientAsyncReadWorker::End() {
  Guard g(mtx_);
  if (end_cb_)
    end_cb_();
  if (!aborted_)
    Abort();
  assert(!end_cb_);
}

}  // namespace grpc_cb_core