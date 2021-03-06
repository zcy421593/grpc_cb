// Licensed under the Apache License, Version 2.0.
// Author: Jin Qing (http://blog.csdn.net/jq0123)

#ifndef GRPC_CB_CLIENT_ASYNC_WRITER_IMPL2_H
#define GRPC_CB_CLIENT_ASYNC_WRITER_IMPL2_H

#include <cstdint>  // for int64_t
#include <memory>  // for enable_shared_from_this<>
#include <mutex>
#include <string>

#include <grpc_cb/impl/call_sptr.h>         // for CallSptr
#include <grpc_cb/impl/channel_sptr.h>      // for ChannelSptr
#include <grpc_cb/impl/client/client_async_writer_close_handler_sptr.h>  // for ClientAsyncWriterCloseHandlerSptr
#include <grpc_cb/impl/completion_queue_sptr.h>  // for CompletionQueueSptr
#include <grpc_cb/impl/message_sptr.h>           // for MessageSptr
#include <grpc_cb/status.h>                      // for Status
#include <grpc_cb/support/config.h>              // for GRPC_FINAL

namespace grpc_cb {

class ClientAsyncWriterHelper;
class ClientWriterCloseCqTag;

// Impl of impl.
// Impl1 is to make Writer copyable.
// Impl2 will live longer than the Writer.
// We need dtr() of Impl1 to close writing.
// Thread-safe.
class ClientAsyncWriterImpl2 GRPC_FINAL
    : public std::enable_shared_from_this<ClientAsyncWriterImpl2> {
 public:
  ClientAsyncWriterImpl2(const ChannelSptr& channel, const std::string& method,
                         const CompletionQueueSptr& cq_sptr, int64_t timeout_ms);
  ~ClientAsyncWriterImpl2();

  bool Write(const MessageSptr& request_sptr);

  using CloseHandlerSptr = ClientAsyncWriterCloseHandlerSptr;
  void Close(const CloseHandlerSptr& handler_sptr = CloseHandlerSptr());

 private:
  // for ClientWriterCloseCqTag::OnComplete()
  void OnClosed(bool success, ClientWriterCloseCqTag& tag);

  // Todo: Force to close, cancel all writing.
  // Todo: get queue size

 private:
  void SendCloseIfNot();
  void CallCloseHandler();
  void OnEndOfWriting();  // Callback from WriterHelper
  void SetInternalError(const std::string& sError);

 private:
  // The callback may lock the mutex recursively.
  using Mutex = std::recursive_mutex;
  mutable Mutex mtx_;
  using Guard = std::lock_guard<Mutex>;

  const CompletionQueueSptr cq_sptr_;
  const CallSptr call_sptr_;
  Status status_;

  bool writing_started_ = false;  // new writer_sptr once
  bool has_sent_close_ = false;  // Client send close once.

  // Close handler hides the Response and on_closed callback.
  CloseHandlerSptr close_handler_sptr_;
  bool close_handler_set_ = false;  // set handler only once

  // Will be shared by CqTag. Reset on ended.
  std::shared_ptr<ClientAsyncWriterHelper> writer_sptr_;
};  // class ClientAsyncWriterImpl2

}  // namespace grpc_cb
#endif  // GRPC_CB_CLIENT_ASYNC_WRITER_IMPL2_H
