#pragma once

#include <giggle/Messages.h>

#include <librdkafka/rdkafkacpp.h>

#include <memory>
#include <functional>
#include <atomic>

namespace giggle {

class Consumer final {
public:
  struct Settings {
    std::string broker;
    std::string topic;
    int32_t partition{0};

    int msg_wait_timeout_in_ms{1000};
    bool stopReceivingOnLastMsg{false};
  };

  using MsgHandler = std::function<void(const Message &)>;

  Consumer(const Settings&);
  ~Consumer();

  void startReceiving(MsgHandler);

  void stopReceiving();

private:
  std::unique_ptr<RdKafka::Consumer> impl;

  std::unique_ptr<RdKafka::Topic> topic;

  int32_t partition{0};
  int timeout{1000};
  bool stopReceivingOnLastMsg{false};

  std::atomic<bool> stopReceiving_{false};
};

} // namespace giggle
