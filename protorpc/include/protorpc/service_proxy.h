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

#ifndef PROTORPC_INCLUDE_PROTORPC_SERVICE_PROXY_H_
#define PROTORPC_INCLUDE_PROTORPC_SERVICE_PROXY_H_

#include "protorpc/basic_types.h"

#include <functional>
#include <map>
#include <memory>
#include <string>

#include "protorpc/method_invocation_reply.h"

namespace perfetto {
namespace protorpc {

class Client;
class ServiceDescriptor;

class ServiceProxy {
 public:
  using MethodInvocationCallbackBase =
      std::function<void(MethodInvocationReply<ProtoMessage>)>;

  ServiceProxy();
  virtual ~ServiceProxy();

  virtual void OnConnect();
  virtual void OnConnectionFailed();

  bool connected() const { return service_id_ != 0; }

  void InitializeBinding(const std::weak_ptr<ServiceProxy>&,
                         const std::weak_ptr<Client>&,
                         ServiceID,
                         std::map<std::string, MethodID>);

  // Called by the autogenerated ServiceProxy subclasses.
  template <typename T>
  void BeginInvoke(const std::string& method_name,
                   ProtoMessage* method_args,
                   std::function<void(MethodInvocationReply<T>)> callback) {
    static_assert(std::is_base_of<ProtoMessage, T>::value,
                  "T must derive ProtoMessage");
    auto callback_adapter = [callback](MethodInvocationReply<ProtoMessage> r) {
      // callback(static_cast<T>(r));
    };
    BeginInvokeGeneric(method_name, method_args, callback_adapter);
  }

  void BeginInvokeGeneric(const std::string& method_name,
                          ProtoMessage* method_args,
                          MethodInvocationCallbackBase callback);

  // Called by ClientImpl.
  // result == nullptr means request failure.
  void EndInvoke(RequestID, std::unique_ptr<ProtoMessage> result, bool eof);

  // implemented by the autogenerated class.
  virtual const ServiceDescriptor& GetDescriptor() = 0;

 private:
  // This is essentially a weak ptr factory, as weak_ptr(s) are copyable.
  std::weak_ptr<ServiceProxy> weak_ptr_self_;
  std::weak_ptr<Client> client_;
  ServiceID service_id_ = 0;
  std::map<std::string, MethodID> remote_method_ids_;
  std::map<RequestID, MethodInvocationCallbackBase> pending_callbacks_;
};

}  // namespace protorpc
}  // namespace perfetto

#endif  // PROTORPC_INCLUDE_PROTORPC_SERVICE_PROXY_H_
