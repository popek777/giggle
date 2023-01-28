#include <giggle/Consumer.h>
#include <giggle/Error.h>
#include <giggle/Messages.h>
#include <giggle/Producer.h>

#include <atomic>
#include <csignal>
#include <cstdint>
#include <ctime>
#include <iostream>
#include <memory>
#include <thread>

namespace {

std::function<void()> handleExit = [] {};
void sigterm(int sig) { handleExit(); }

std::string topic{"quickstart"};
int32_t questionPartition = 0;
int32_t answersPartition = 1;

void runGptClientMode() {

  std::cout << "starting chat with GPT bot" << std::endl;

  std::atomic<bool> run{true};
  handleExit = [&] { run = false; };

  giggle::Producer gptQestionProducer{
      giggle::Producer::Settings{"localhost", topic, questionPartition}};

  giggle::Consumer gptAnswerConsumer{giggle::Consumer::Settings{
      "localhost", topic, answersPartition, 1000, false}};

  auto gptAnswerReceiverThr = std::thread([&] {
    gptAnswerConsumer.startReceiving([](const giggle::Message &answer) {
      std::cout << "[" << answer.senderId << ", " << answer.generationTmstmp
                << "]: " << answer.payload << std::endl;
    });
  });

  for (std::string question; run && std::getline(std::cin, question);) {
    gptQestionProducer.sendMsg(
        giggle::Message{"desperate for answers", question, std::time(nullptr)});
  }

  gptAnswerConsumer.stopReceiving();

  gptAnswerReceiverThr.join();

  std::cout << "GPT chat finished" << std::endl;
}

class AiGptEngine {
  public:
    std::string getAnswer(const std::string& question)
    {
      if (question.find("weather") != std::string::npos) {
        return "the weather is cold!!!";
      } else if (question.find("are you") != std::string::npos) {
        return "I'm John and I'm fine";
      } else if(question.find("name") != std::string::npos) {
        return "John";
      } else if(question.find("bye") != std::string::npos) {
        return "bye! nice talking to you";
      } else if (question.find("hello") != std::string::npos ||
                 question.find("hi") != std::string::npos) {
        return "hello and welcome! please ask whatever you want. I'm very smart!";
      } else {
        return "sorry! doon't know the answer to: " + question;
      }
    }

};

void runGptServerMode() {

  std::cout << "starting GPT bot" << std::endl;

    AiGptEngine gptEngine;

    giggle::Producer gptAnswerProducer{
        giggle::Producer::Settings{"localhost", topic, answersPartition}};

    giggle::Consumer gptQuestionConsumer{giggle::Consumer::Settings{
        "localhost", topic, questionPartition, 1000, false}};

    handleExit = [&] { gptQuestionConsumer.stopReceiving(); };

    auto gptQuestionReceivingThr = std::thread([&] {
      gptQuestionConsumer.startReceiving(
          [&](const giggle::Message &question) {
            auto answer = gptEngine.getAnswer(question.payload);

            std::cout << "Q [" << question.senderId << ", "
                      << question.generationTmstmp << "]: " << question.payload
                      << std::endl;

            giggle::Message answerm{"GPT bot", answer, std::time(nullptr)};
            gptAnswerProducer.sendMsg(answerm);

            std::cout << "A: " << answerm.payload << std::endl;
          });
    });

    giggle::Producer producer{giggle::Producer::Settings{"localhost"}};

    gptQuestionReceivingThr.join();

    std::cout << "GPT bot finished" << std::endl;
}
} // namespace

int main(int argc, char** argv) {

  std::string mode{argv[1]};

  std::signal(SIGINT, sigterm);
  std::signal(SIGTERM, sigterm);

  try {
    if (mode == "client") {
      runGptClientMode();
    } else if (mode == "server") {
      runGptServerMode();
    } else {
      std::cerr << "Error: invalid mode " << mode
                << ". client or server allowed" << std::endl;
      exit(1);
    }
  } catch (const giggle::Error &err) {
    std::cerr << "giggle error: " << err.what() << std::endl;
    exit(1);
  }
}
