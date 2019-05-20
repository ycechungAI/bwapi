#include"BWAPIProtoClient.h"

namespace BWAPI
{
    BWAPIProtoClient::BWAPIProtoClient()
    {
        connected = false;
        //tcpListener.listen(8045, "127.0.0.1");
    }

  void BWAPIProtoClient::checkForConnection(uint32_t apiVersion, std::string enginetype, std::string engineVersion)
  {
    if (isConnected())
      return;
    std::ofstream output;
    output.open("a.txt");
    auto status = tcpListener.accept(tcpSocket);
    switch (status)
    {
    case sf::Socket::Status::Disconnected:
      output << "Disconnected\n";
      break;
    case sf::Socket::Status::Done:
      output << "Done\n";
      break;
    case sf::Socket::Status::Error:
      output << "Error\n";
      break;
    case sf::Socket::Status::NotReady:
      output << "NotReady\n";
      break;
    case sf::Socket::Status::Partial:
      output << "Partial\n";
      break;
    }
    output.close();
    if (tcpSocket.getRemoteAddress() == sf::IpAddress::None)
      return;
    tcpListener.close();
    tcpSocket.setBlocking(true);
    sf::Packet packet;
  }

  void BWAPI::BWAPIProtoClient::lookForServer(int apiversion, std::string bwapiversion, bool tournament)
  {
    if (isConnected())
      return;
    std::ofstream output;
    output.open("b.txt");
    tcpSocket.connect("127.0.0.1", 8045);
    if (tcpSocket.getRemoteAddress() == sf::IpAddress::None)
      fprintf(stderr, "%s", "Connection failed.\n");
    output << "IP address: " << tcpSocket.getRemoteAddress() << std::endl;
    output.close();
  }

  void BWAPIProtoClient::transmitMessages()
  {
    //Check that we are connected to a game server.
    if (!isConnected())
        return;
    std::unique_ptr<bwapi::message::Message> currentMessage;
    sf::Packet packet;
    //loop until the message queue is empty.
    while (messageQueue.size())
    {
      packet.clear();
      currentMessage = std::move(messageQueue.front());
      messageQueue.pop_front();
      packet << currentMessage->SerializeAsString();
      if (tcpSocket.send(packet) != sf::Socket::Done)
      {
        //Error sending command, we should do something here?
        fprintf(stderr, "Failed to send a Message.\n");
      }
    }
    //Finished with queue, send the EndOfQueue message
    auto endOfQueue = std::make_unique<bwapi::game::EndOfQueue>();
    currentMessage = std::make_unique<bwapi::message::Message>();
    currentMessage->set_allocated_endofqueue(endOfQueue.release());
    packet.clear();
    packet << currentMessage->SerializeAsString();
    if (tcpSocket.send(packet) != sf::Socket::Done)
    {
      //Error sending EndofQueue
      fprintf(stderr, "Failed to send end of queue command.");
    }

  }

  void BWAPIProtoClient::receiveMessages()
  {
    //Check that we are connected to a game server or client.
    if (!isConnected())
      return;
    std::unique_ptr<bwapi::message::Message> currentMessage;
    sf::Packet packet;
    std::string packetContents;
    //loop until the end of queue message is received.
    while (true)
    {
      packet.clear();
      packetContents.clear();
      currentMessage = std::make_unique<bwapi::message::Message>();
      if (tcpSocket.receive(packet) != sf::Socket::Done)
      {
        fprintf(stderr, "Failed to receive messages.\n");
        return;
      }
      packet >> packetContents;
      currentMessage->ParseFromString(packetContents);
      if (currentMessage->has_endofqueue())
        return;
      messageQueue.push_back(std::move(currentMessage));
    }

  }

  void BWAPIProtoClient::queueMessage(std::unique_ptr<bwapi::message::Message> newMessage)
  {
    messageQueue.push_back(std::move(newMessage));
  }
    
  bool BWAPIProtoClient::isConnected() const
  {
    return tcpSocket.getRemoteAddress() != sf::IpAddress::None;
  }

  int BWAPIProtoClient::messageQueueSize() const
  {
    return messageQueue.size();
  }

  std::unique_ptr<bwapi::message::Message> BWAPIProtoClient::getNextMessage()
  {
    //if (!messageQueue.size())
    //  return std::make_unique<bwapi::message::Message>(nullptr);
    auto nextMessage = std::move(messageQueue.front());
    messageQueue.pop_front();
    return nextMessage;
  }
  void BWAPIProtoClient::disconnect()
  {
    if (!isConnected())
      return;
    tcpSocket.disconnect();
  }
  void BWAPIProtoClient::initListen()
  {
    std::ofstream output;
    output.open("d.txt");
    output << "I am going to listen now. " << std::endl;
    tcpListener.setBlocking(false);
    auto status = tcpListener.listen(8045);
    switch (status)
    {
    case sf::Socket::Status::Disconnected:
      output << "Disconnected" << std::endl;
      break;
    case sf::Socket::Status::Done:
      output << "Done" << std::endl;
      break;
    case sf::Socket::Status::Error:
      output << "Error" << std::endl;
      break;
    case sf::Socket::Status::NotReady:
      output << "NotReady" << std::endl;
      break;
    case sf::Socket::Status::Partial:
      output << "Partial" << std::endl;
      break;
    }
    output.close();
  }
  void BWAPIProtoClient::stopListen()
  {
    tcpListener.close();
  }
}