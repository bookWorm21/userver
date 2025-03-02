#pragma once

/// @file userver/ugrpc/server/middlewares/middleware_base.hpp
/// @brief @copybrief ugrpc::server::MiddlewareBase

#include <memory>
#include <vector>

#include <userver/components/loggable_component_base.hpp>
#include <userver/utils/function_ref.hpp>

#include <userver/ugrpc/server/middlewares/fwd.hpp>
#include <userver/ugrpc/server/rpc.hpp>

USERVER_NAMESPACE_BEGIN

namespace ugrpc::server {

/// @brief Context for middleware-specific data during gRPC call
class MiddlewareCallContext final {
 public:
  /// @cond
  MiddlewareCallContext(const Middlewares& middlewares, CallAnyBase& call,
                        utils::function_ref<void()> user_call,
                        const dynamic_config::Snapshot& config,
                        ::google::protobuf::Message* request);
  /// @endcond

  /// @brief Call next plugin, or gRPC handler if none
  void Next();

  /// @brief Get original gRPC Call
  CallAnyBase& GetCall() const;

  /// @brief Get name of gRPC service
  std::string_view GetServiceName() const;

  /// @brief Get name of called gRPC method
  std::string_view GetMethodName() const;

  /// @brief Get values extracted from dynamic_config. Snapshot will be
  /// deleted when the last meddleware completes
  const dynamic_config::Snapshot& GetInitialDynamicConfig() const;

 private:
  void ClearMiddlewaresResources();

  Middlewares::const_iterator middleware_;
  Middlewares::const_iterator middleware_end_;
  utils::function_ref<void()> user_call_;

  CallAnyBase& call_;

  std::optional<dynamic_config::Snapshot> config_;
  ::google::protobuf::Message* request_;
};

/// @brief Base class for server gRPC middleware
class MiddlewareBase {
 public:
  MiddlewareBase();
  MiddlewareBase(const MiddlewareBase&) = delete;
  MiddlewareBase& operator=(const MiddlewareBase&) = delete;
  MiddlewareBase& operator=(MiddlewareBase&&) = delete;

  virtual ~MiddlewareBase();

  /// @brief Handles the gRPC request
  /// @note You should call context.Next() inside, otherwise the call will be
  /// dropped
  virtual void Handle(MiddlewareCallContext& context) const = 0;

  /// @brief Request hook. This function calls each request
  virtual void CallRequestHook(const MiddlewareCallContext& context,
                               google::protobuf::Message& request);

  /// @brief Response hook. This function calls each response
  virtual void CallResponseHook(const MiddlewareCallContext& context,
                                google::protobuf::Message& response);
};

/// @brief Base class for middleware component
class MiddlewareComponentBase : public components::LoggableComponentBase {
  using components::LoggableComponentBase::LoggableComponentBase;

 public:
  /// @brief Returns a middleware according to the component's settings
  virtual std::shared_ptr<MiddlewareBase> GetMiddleware() = 0;
};

}  // namespace ugrpc::server

USERVER_NAMESPACE_END
