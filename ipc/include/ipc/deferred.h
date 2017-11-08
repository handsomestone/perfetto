/*
 * Copyright (C) 2017 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef IPC_INCLUDE_IPC_DEFERRED_H_
#define IPC_INCLUDE_IPC_DEFERRED_H_

#include <functional>
#include <memory>
#include <utility>

#include "ipc/async_result.h"
#include "ipc/basic_types.h"

namespace perfetto {
namespace ipc {

// This class is a wrapper for a callback handling async results.
// The problem this is solving is the following: For each result argument of the
// methods generated from the .proto file:
// - The client wants to see something on which it can Bind() a callback, which
//   is invoked asynchrnously once reply is received fromt he host.
// - The host wants to expose something to embedder that implements the IPC
//   methods to allow them to provide an asynchronous reply back to the client.
//   Eventually even more than once, for the case streaming replies.
//
// In both cases we want to make sure that callbacks don't get lost along the
// way. To address this, this class will automatically reject the callbacks
// if they are not resolved at destructor time (or the object is std::move()'d).
//
// The client is supposed to use this class as follows:
//   class GreeterProxy {
//      void SayHello(const HelloRequest&, Deferred<HelloReply> reply)
//   }
//  ...
//  Deferred<HelloReply> reply;
//  reply.Bind([] (AsyncResult<HelloReply> reply) {
//    std::cout << reply.success() ? reply->message : "failure";
//  });
//  host_proxy_instance.SayHello(req, std::move(reply));
//
// The host instead is supposed to use this as follows:
//   class GreeterImpl : public Greeter {
//     void SayHello(const HelloRequest& req, Deferred<HelloReply> reply) {
//        AsyncResult<HelloReply> reply = AsyncResult<HelloReply>::New();
//        reply->set_greeting("Hello " + req.name)
//        reply.Resolve(std::move(reply));
//     }
//   }
// Or for more complex cases, the deferred object can be std::move()'d outside
// and the reply can continue asynchrnously later.

class DeferredBase {
 public:
  DeferredBase(
      std::function<void(AsyncResult<ProtoMessage>)> callback = nullptr);
  ~DeferredBase();
  DeferredBase(DeferredBase&&) noexcept;
  DeferredBase& operator=(DeferredBase&&);
  void Bind(std::function<void(AsyncResult<ProtoMessage>)> callback);
  bool IsBound() const;
  void Resolve(AsyncResult<ProtoMessage>);
  void Reject();

 protected:
  void Move(DeferredBase&);

  std::function<void(AsyncResult<ProtoMessage>)> callback_;
};

template <typename T = ProtoMessage>
class Deferred : public DeferredBase {
 public:
  Deferred(std::function<void(AsyncResult<T>)> callback = nullptr) {
    Bind(std::move(callback));
  }

  DeferredBase MoveAsBase() { return DeferredBase(std::move(callback_)); }

  void Bind(std::function<void(AsyncResult<T>)> callback) {
    // Here we need a callback adapter to downcast the callback to a generic
    // callback that takes an AsyncResult<ProtoMessage>, so that it can be
    // stored in the base class |callback_|. This is what allows MoveAsBase().
    auto callback_adapter = [callback](
                                AsyncResult<ProtoMessage> async_result_base) {
      if (!callback)
        return;
      // Upcast the async_result from <ProtoMessage> -> <T : ProtoMessage>.
      static_assert(std::is_base_of<ProtoMessage, T>::value, "T:ProtoMessage");
      AsyncResult<T> async_result(
          std::unique_ptr<T>(static_cast<T*>(async_result_base.release_msg())),
          async_result_base.has_more());
      callback(std::move(async_result));
    };
    DeferredBase::Bind(callback_adapter);
  }

  // If no more messages are expected, |callback_| is released.
  void Resolve(AsyncResult<T> async_result) {
    // Convert the |async_result| to the generic base one (T -> ProtoMessage).
    AsyncResult<ProtoMessage> async_result_base(
        std::unique_ptr<ProtoMessage>(async_result.release_msg()),
        async_result.has_more());
    DeferredBase::Resolve(std::move(async_result_base));
  }
};

}  // namespace ipc
}  // namespace perfetto

#endif  // IPC_INCLUDE_IPC_DEFERRED_H_
