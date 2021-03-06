// Licensed under the Apache License, Version 2.0.
// Author: Jin Qing (http://blog.csdn.net/jq0123)

#include "client_call_cqtag.h"

namespace grpc_cb_core {

Status ClientCallCqTag::GetResponse(std::string& response) const {
    // Todo: Get trailing metadata.
    if (!cod_client_recv_status_.IsStatusOk())
      return cod_client_recv_status_.GetStatus();
    return cod_recv_msg_.GetResultMsg(response);
}

}  // namespace grpc_cb_core
