//  Agora RTC/MEDIA SDK
//
//  Created by Pengfei Han in 2020-03.
//  Copyright (c) 2020 Agora.io. All rights reserved.
//

#include <thread>

#include "gtest/gtest.h"

#include "wrapper/connection_wrapper.h"
#include "wrapper/local_user_wrapper.h"
#include "wrapper/media_packet_receiver.h"
#include "wrapper/media_packet_sender.h"
#include "wrapper/utils.h"

class AudioTest : public testing::Test {
 public:
  void SetUp() override {}

  void TearDown() override {}
};

TEST_F(AudioTest, audio_packet_send_and_receive) {
  auto service = createAndInitAgoraService(false, true, true);

  // Create receive connection
  ConnectionConfig recv_config;
  recv_config.clientRoleType = agora::rtc::CLIENT_ROLE_BROADCASTER;
  recv_config.subscribeAllAudio = true;

  recv_config.subscribeAllVideo = true;
  recv_config.encodedFrameOnly = true;
  recv_config.recv_type = agora::rtc::RECV_PACKET_ONLY;
  auto conn_recv = ConnectionWrapper::CreateConnection(service, recv_config);

  // Join channel
  conn_recv->Connect(API_CALL_APPID, CONNECTION_TEST_DEFAULT_CNAME, "3");
  ASSERT_TRUE(conn_recv->WaitForConnected(3000));

  // Create send connection and start sending.
  ConnectionConfig config;
  config.clientRoleType = agora::rtc::CLIENT_ROLE_BROADCASTER;
  config.channelProfile = agora::CHANNEL_PROFILE_LIVE_BROADCASTING;
  config.clientRoleType = agora::rtc::CLIENT_ROLE_BROADCASTER;

  auto conn_send = ConnectionWrapper::CreateConnection(service, config);
  conn_send->Connect(API_CALL_APPID, CONNECTION_TEST_DEFAULT_CNAME, "1");
  auto result = conn_send->WaitForConnected(3000);
  ASSERT_TRUE(result);

  auto factory = service->createMediaNodeFactory();

  SendConfig args = {500000, 350, 7, true};

  std::unique_ptr<MediaPacketSender> packet_sender(new MediaPacketSender(args));
  packet_sender->initialize(service, factory, conn_send);

  auto thread = std::make_shared<std::thread>(
      std::bind(&MediaPacketSender::sendPackets, packet_sender.get()));

  // To receive media packet
  auto media_packet_receiver = std::make_shared<MediaPacketReceiver>();

  auto remote_audio_track = conn_recv->GetLocalUser()->GetRemoteAudioTrack(3000);
  ASSERT_TRUE(remote_audio_track);
  remote_audio_track->registerMediaPacketReceiver(media_packet_receiver.get());

  thread->join();

  auto media_packet_bytes = media_packet_receiver->GetReceivedMediaPacketBytes();
  AGO_LOG("Receive media packet bytes %zu\n", media_packet_bytes);
}