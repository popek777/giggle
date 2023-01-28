#include <giggle/Consumer.h>
#include <giggle/Error.h>
#include <giggle/Messages.h>

#include "librdkafka/rdkafkacpp.h"

#include <iostream>

namespace giggle {

Consumer::Consumer(const Settings &settings)
    : partition(settings.partition), timeout(settings.msg_wait_timeout_in_ms) {

  auto conf = std::unique_ptr<RdKafka::Conf>{
      RdKafka::Conf::create(RdKafka::Conf::CONF_GLOBAL)};

  std::string err;
  if (conf->set("bootstrap.servers", settings.broker, err) !=
      RdKafka::Conf::CONF_OK) {
    throw Error{"failed to set broker: " + err};
  }

  conf->set("enable.partition.eof", "true", err);

  impl.reset(RdKafka::Consumer::create(conf.get(), err));
  if (!impl) {
    throw Error{"Failed to create consumer: " + err};
  }

  auto tconf = std::unique_ptr<RdKafka::Conf>{
      RdKafka::Conf::create(RdKafka::Conf::CONF_TOPIC)};
  topic.reset(
      RdKafka::Topic::create(impl.get(), settings.topic, tconf.get(), err));
  if (!topic) {
    throw Error{"Failed to create topic: " + err};
  }

  auto res =
      impl->start(topic.get(), partition, RdKafka::Topic::OFFSET_END);
  if (res != RdKafka::ERR_NO_ERROR) {
    throw Error{"Failed to start consumer: " + err};
  }
}

Consumer::~Consumer()
{
  impl->stop(topic.get(), partition);
}

namespace {

class ConsumerCbImpl : public RdKafka::ConsumeCb {

  std::function<void(RdKafka::Message &msg)> handler;

public:
  ConsumerCbImpl(std::function<void(RdKafka::Message &msg)> h)
      : handler(std::move(h)) {}

  void consume_cb(RdKafka::Message &msg, void *) override {
    handler(msg);
  }
};

} // namespace

void Consumer::startReceiving(MsgHandler handler) {

  auto handleKafkaMsg = [&, this](RdKafka::Message &msg) {
    switch (msg.err()) {
    case RdKafka::ERR__TIMED_OUT:
      break;

    case RdKafka::ERR_NO_ERROR:
      {
      auto in = avro::memoryInputStream(static_cast<uint8_t *>(msg.payload()),
                                        msg.len());
      auto d = avro::binaryDecoder();
      d->init(*in);
      giggle::Message gmsg;
      avro::decode(*d, gmsg);

      handler(gmsg);
      break;
      }

    case RdKafka::ERR__PARTITION_EOF:
      if (stopReceivingOnLastMsg) {
        stopReceiving_ = true;
      }
      break;

    case RdKafka::ERR__UNKNOWN_TOPIC:
    case RdKafka::ERR__UNKNOWN_PARTITION:
      std::cerr << "Consume failed: " << msg.errstr() << std::endl;
      stopReceiving_ = true;
      break;

    default:
      std::cerr << "Consume failed: " << msg.errstr() << std::endl;
      stopReceiving_ = true;
    };
  };
  auto cbImpl = std::make_unique<ConsumerCbImpl>(handleKafkaMsg);

  while (!stopReceiving_) {
    impl->consume_callback(
        topic.get(), partition, timeout, cbImpl.get(), nullptr);
    impl->poll(0);
  }
}
void Consumer::stopReceiving() { stopReceiving_ = true; }

} // namespace giggle

