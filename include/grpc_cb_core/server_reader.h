// Licensed under the Apache License, Version 2.0.
// Author: Jin Qing (http://blog.csdn.net/jq0123)

#ifndef GRPC_CB_CORE_SERVER_READER_H
#define GRPC_CB_CORE_SERVER_READER_H

#include <string>

namespace grpc_cb_core {

class Status;

// ServerReader is the interface of client streaming handler.
// Thread-safe.
class ServerReader {
 public:
  ServerReader() {}
  virtual ~ServerReader() {}

 public:
  virtual void OnMsg(const std::string& msg) {}
  virtual void OnError(const Status& status) {}
  virtual void OnEnd() {}
};  // class ServerReader

}  // namespace grpc_cb_core
#endif  // GRPC_CB_CORE_SERVER_READER_H
