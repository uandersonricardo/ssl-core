#include "robocin/network/zmq_subscriber_socket.h"

#include "gmock/gmock.h"
#include <gtest/gtest.h>
#include <optional>
#include <span>
#include <string_view>
#include <zmq.hpp>

namespace robocin {

using ::testing::_;
using ::testing::Eq;
using ::testing::Return;
using ::testing::Throw;

constexpr int kFileDescriptor = 1;
constexpr std::string_view kAddress = "ipc:///tmp/channel.ipc";
constexpr std::string_view kDefaultTopic = "zmq_topic";
constexpr std::string_view kDefaultMessage = "zmq_message";
constexpr size_t kBytesReceived = 42;

constexpr std::span<const std::string_view> kDefaultTopics{&kDefaultTopic, 1};

class MockZmqContext {
 public:
  explicit MockZmqContext(int /*unused*/) {}
};

class MockZmqSocket {
 public:
  MockZmqSocket(MockZmqContext /*unused*/, int /*unused*/) {}

  MOCK_METHOD(void, connect, (const std::string&) );
  MOCK_METHOD(void, set, (decltype(zmq::sockopt::subscribe), std::string_view));
  MOCK_METHOD(int, get, (decltype(zmq::sockopt::fd)), (const));
  MOCK_METHOD(zmq::recv_result_t, recv, (zmq::message_t&, zmq::recv_flags));
  MOCK_METHOD(void, close, ());
};

using MockZmqSubscriberSocket = IZmqSubscriberSocket<MockZmqContext, MockZmqSocket>;

MATCHER(TrueMatcher, "") { return true; }

TEST(ZmqSubscriberSocketTest, WhenConnectIsSucceeded) {
  MockZmqSubscriberSocket socket;

  EXPECT_CALL(socket.socket_, connect(Eq(kAddress)));
  EXPECT_CALL(socket.socket_, set(TrueMatcher(), Eq(kDefaultTopic)));
  socket.connect(kAddress, kDefaultTopics);
}

TEST(ZmqSubscriberSocketTest, WhenConnectThrowsZmqErrorForAddress) {
  MockZmqSubscriberSocket socket;

  EXPECT_CALL(socket.socket_, connect(Eq(kAddress))).WillOnce(Throw(zmq::error_t{}));
  EXPECT_THROW(socket.connect(kAddress, kDefaultTopics), zmq::error_t);
}

TEST(ZmqSubscriberSocketTest, WhenConnectThrowsZmqErrorForTopic) {
  MockZmqSubscriberSocket socket;

  EXPECT_CALL(socket.socket_, connect(Eq(kAddress)));
  EXPECT_CALL(socket.socket_, set(TrueMatcher(), Eq(kDefaultTopic)))
      .WillOnce(Throw(zmq::error_t{}));
  EXPECT_THROW(socket.connect(kAddress, kDefaultTopics), zmq::error_t);
}

TEST(ZmqSubscriberSocketTest, WhenReceiveIsSucceeded) {
  MockZmqSubscriberSocket socket;

  EXPECT_CALL(socket.socket_, recv(_, zmq::recv_flags::dontwait))
      .WillOnce([](zmq::message_t& msg, zmq::recv_flags) {
        msg = zmq::message_t(kDefaultTopic);
        return kBytesReceived;
      })
      .WillOnce([](zmq::message_t& msg, zmq::recv_flags) {
        msg = zmq::message_t(kDefaultMessage);
        return kBytesReceived;
      });

  ZmqDatagram oracle{std::string{kDefaultTopic}, std::string{kDefaultMessage}};
  ASSERT_EQ(socket.receive(), oracle);
}

TEST(ZmqSubscriberSocketTest, WhenReceiveThrowsZmqErrorForTopic) {
  MockZmqSubscriberSocket socket;

  EXPECT_CALL(socket.socket_, recv(_, zmq::recv_flags::dontwait)).WillOnce(Throw(zmq::error_t{}));
  ASSERT_THROW(socket.receive(), zmq::error_t);
}

TEST(ZmqSubscriberSocketTest, WhenReceiveThrowsZmqErrorForMessage) {
  MockZmqSubscriberSocket socket;

  EXPECT_CALL(socket.socket_, recv(_, zmq::recv_flags::dontwait))
      .WillOnce([](zmq::message_t& msg, zmq::recv_flags) {
        msg = zmq::message_t(kDefaultTopic);
        return kBytesReceived;
      })
      .WillOnce(Throw(zmq::error_t{}));
  ASSERT_THROW(socket.receive(), zmq::error_t);
}

TEST(ZmqSubscriberSocketTest, WhenReceiveTopicIsNullopt) {
  MockZmqSubscriberSocket socket;

  EXPECT_CALL(socket.socket_, recv(_, zmq::recv_flags::dontwait)).WillOnce(Return(std::nullopt));
  ASSERT_EQ(socket.receive(), ZmqDatagram{});
}

TEST(ZmqSubscriberSocketTest, WhenReceiveMessageIsNullopt) {
  MockZmqSubscriberSocket socket;

  EXPECT_CALL(socket.socket_, recv(_, zmq::recv_flags::dontwait))
      .WillOnce([](zmq::message_t& msg, zmq::recv_flags) {
        msg = zmq::message_t(kDefaultTopic);
        return kBytesReceived;
      })
      .WillOnce(Return(std::nullopt));
  ASSERT_EQ(socket.receive(), ZmqDatagram{});
}

TEST(ZmqSubscriberSocketTest, WhenFileDescriptionGetterIsSucceeded) {
  MockZmqSubscriberSocket socket;

  EXPECT_CALL(socket.socket_, get(TrueMatcher())).WillOnce(Return(kFileDescriptor));
  ASSERT_EQ(socket.fd(), kFileDescriptor);
}

TEST(ZmqSubscriberSocketTest, WhenCloseIsSucceeded) {
  MockZmqSubscriberSocket socket;

  EXPECT_CALL(socket.socket_, close());
  socket.close();
}

} // namespace robocin
