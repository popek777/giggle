#pragma once

#include <giggle/Messages.h>

#include <librdkafka/rdkafkacpp.h>

#include <cstdint>
#include <functional>
#include <memory>
#include <string>

namespace giggle {

class Producer final {
public:
  struct Settings {
    std::string broker;

    std::string topic;
    int32_t partition{0};
  };

  Producer(const Settings &);
  ~Producer();

  void
  sendMsg(const Message&);

private:

  bool
  sendMsgImpl(void *payload, std::size_t payloadSize);

  std::unique_ptr<RdKafka::DeliveryReportCb> callback;
  std::unique_ptr<RdKafka::Producer> impl;

  std::string topic;
  int32_t partition{0};
};

} // namespace giggle
