// Licensed under the Apache License, Version 2.0.
// Author: Jin Qing (http://blog.csdn.net/jq0123)

#ifndef GRPC_CB_CORE_CLIENT_SYNC_READER_H
#define GRPC_CB_CORE_CLIENT_SYNC_READER_H

#include <cassert>  // for assert()
#include <cstdint>  // for int64_t
#include <string>

#include <grpc_cb_core/channel.h>  // for MakeSharedCall()
#include <grpc_cb_core/impl/client/client_sync_reader_data.h>  // for ClientSyncReaderDataSptr
#include <grpc_cb_core/impl/client/client_reader_init_cqtag.h>  // for ClientReaderInitCqTag
#include <grpc_cb_core/impl/client/client_sync_reader_helper.h>  // for ClientSyncReaderHelper
#include <grpc_cb_core/common/status.h>  // for Status

namespace grpc_cb_core {

// Copyable. Client sync reader.
class ClientSyncReader GRPC_FINAL {
 public:
  inline ClientSyncReader(const ChannelSptr& channel, const std::string& method,
                          const std::string& request, int64_t timeout_ms);

 public:
  // Return false if error or end of stream.
  inline bool ReadOne(std::string* response) const {
    assert(response);
    Data& d = *data_sptr_;
    return ClientSyncReaderHelper::SyncReadOne(d.call_sptr, d.cq4p_sptr,
                                               *response, d.status);
  }

  inline Status RecvStatus() const {
    const Data& d = *data_sptr_;
    if (!d.status.ok()) return d.status;
    return ClientSyncReaderHelper::SyncRecvStatus(d.call_sptr, d.cq4p_sptr);
  }

 private:
  // Wrap all data in shared struct pointer to make copy quick.
  using Data = ClientSyncReaderData;
  using DataSptr = ClientSyncReaderDataSptr;
  DataSptr data_sptr_;
};  // class ClientSyncReader<>

ClientSyncReader::ClientSyncReader(
    const ChannelSptr& channel, const std::string& method,
    const std::string& request, int64_t timeout_ms)
    : data_sptr_(new Data) {
  assert(channel);
  CQueueForPluckSptr cq4p_sptr(new CQueueForPluck);
  CallSptr call_sptr = channel->MakeSharedCall(method, *cq4p_sptr, timeout_ms);
  data_sptr_->cq4p_sptr = cq4p_sptr;
  data_sptr_->call_sptr = call_sptr;

  ClientReaderInitCqTag tag(call_sptr);
  if (tag.Start(request)) {
    cq4p_sptr->Pluck(&tag);
    return;
  }

  data_sptr_->status.SetInternalError(
      "Failed to start client sync reader stream.");
}  // ClientSyncReader()

}  // namespace grpc_cb_core

#endif  // GRPC_CB_CORE_CLIENT_SYNC_READER_H
