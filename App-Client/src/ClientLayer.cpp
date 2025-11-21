#include "ClientLayer.h"

#include "ServerPacket.h"

#include "Walnut/Application.h"
#ifndef WL_HEADLESS
#include "Walnut/UI/UI.h"
#endif
#include "Walnut/Serialization/BufferStream.h"
#include "Walnut/Networking/NetworkingUtils.h"
#include "Walnut/Utils/StringUtils.h"

#ifndef WL_HEADLESS
#include "misc/cpp/imgui_stdlib.h"
#endif

#include <yaml-cpp/yaml.h>

#include <iostream>
#include <fstream>

void ClientLayer::OnAttach()
{
	m_ScratchBuffer.Allocate(1024);

	m_Client = std::make_unique<Walnut::Client>();
	m_Client->SetServerConnectedCallback([this]() { OnConnected(); });
	m_Client->SetServerDisconnectedCallback([this]() { OnDisconnected(); });
	m_Client->SetDataReceivedCallback([this](const Walnut::Buffer data) { OnDataReceived(data); });

	m_Console.SetMessageSendCallback([this](std::string_view message) { SendChatMessage(message); });

	LoadConnectionDetails(m_ConnectionDetailsFilePath);
}

void ClientLayer::OnDetach()
{
	m_Client->Disconnect();
	// ^ currently disconnect is blocking

	m_ScratchBuffer.Release();
}

#ifdef WL_HEADLESS
void ClientLayer::OnUpdate(float ts)
{
	Headless_ConnectionModal();
}
#else
void ClientLayer::OnUIRender()
{
	UI_ConnectionModal();
	
	m_Console.OnUIRender();
	UI_ClientList();
}
#endif

bool ClientLayer::IsConnected() const
{
	return m_Client->GetConnectionStatus() == Walnut::Client::ConnectionStatus::Connected;
}

void ClientLayer::OnDisconnectButton()
{
	m_Client->Disconnect();
}

#ifndef WL_HEADLESS
void ClientLayer::UI_ConnectionModal()
{
	if (!m_ConnectionModalOpen && m_Client->GetConnectionStatus() != Walnut::Client::ConnectionStatus::Connected)
	{
		ImGui::OpenPopup("Connect to server");
	}

	m_ConnectionModalOpen = ImGui::BeginPopupModal("Connect to server", nullptr, ImGuiWindowFlags_AlwaysAutoResize);
	if (m_ConnectionModalOpen)
	{
		ImGui::Text("Your Name");
		ImGui::InputText("##username", &m_Username);

		ImGui::Text("Pick a color");
		ImGui::SameLine();
		ImGui::ColorEdit4("##color", m_ColorBuffer);

		ImGui::Text("Server Address");
		ImGui::InputText("##address", &m_ServerIP);
		ImGui::SameLine();
		if (ImGui::Button("Connect"))
		{
			m_Color = IM_COL32(m_ColorBuffer[0] * 255.0f, m_ColorBuffer[1] * 255.0f, m_ColorBuffer[2] * 255.0f, m_ColorBuffer[3] * 255.0f);

			if (Walnut::Utils::IsValidIPAddress(m_ServerIP))
			{
				m_Client->ConnectToServer(m_ServerIP);
			}
			else
			{
				// Try resolve domain name
				auto ipTokens = Walnut::Utils::SplitString(m_ServerIP, ':'); // [0] == hostname, [1] (optional) == port
				std::string serverIP = Walnut::Utils::ResolveDomainName(ipTokens[0]);
				if (ipTokens.size() != 2)
					serverIP = fmt::format("{}:{}", serverIP, 8192); // Add default port if hostname doesn't contain port
				else
					serverIP = fmt::format("{}:{}", serverIP, ipTokens[1]); // Add specified port

				m_Client->ConnectToServer(serverIP);
			}

		}

		if (Walnut::UI::ButtonCentered("Quit"))
			Walnut::Application::Get().Close();

		if (m_Client->GetConnectionStatus() == Walnut::Client::ConnectionStatus::Connected)
		{
			// Send username
			Walnut::BufferStreamWriter stream(m_ScratchBuffer);
			stream.WriteRaw<PacketType>(PacketType::ClientConnectionRequest);
			stream.WriteRaw<uint32_t>(m_Color); // Color
			stream.WriteString(m_Username); // Username

			m_Client->SendBuffer(stream.GetBuffer());

			SaveConnectionDetails(m_ConnectionDetailsFilePath);

			// Wait for response
			ImGui::CloseCurrentPopup();
		}
		else if (m_Client->GetConnectionStatus() == Walnut::Client::ConnectionStatus::FailedToConnect)
		{
			ImGui::TextColored(ImVec4(0.9f, 0.2f, 0.1f, 1.0f), "Connection failed.");
			const auto& debugMessage = m_Client->GetConnectionDebugMessage();
			if (!debugMessage.empty())
				ImGui::TextColored(ImVec4(0.9f, 0.2f, 0.1f, 1.0f), debugMessage.c_str());
		}
		else if (m_Client->GetConnectionStatus() == Walnut::Client::ConnectionStatus::Connecting)
		{
			ImGui::TextColored(ImVec4(0.8f, 0.8f, 0.8f, 1.0f), "Connecting...");
		}

		ImGui::EndPopup();
	}
}

void ClientLayer::UI_ClientList()
{
	ImGui::Begin("Users Online");
	ImGui::Text("Online: %d", m_ConnectedClients.size());

	static bool selected = false;
	for (const auto& [username, clientInfo] : m_ConnectedClients)
	{
		if (username.empty())
			continue;

		ImGui::PushStyleColor(ImGuiCol_Text, ImColor(clientInfo.Color).Value);
		ImGui::Selectable(username.c_str(), &selected);
		ImGui::PopStyleColor();
	}
	ImGui::End();
}
#else
void ClientLayer::Headless_ConnectionModal()
{
	// Headless: prompt once for username/server and connect.
    // Handshake (ClientConnectionRequest) is sent once from OnConnected() when the connection is established.
    if (!m_HeadlessPrompted)
    {
        if (m_Username.empty())
            m_Username = "";
        if (m_ServerIP.empty())
            m_ServerIP = "127.0.0.1:8192";

        auto promptRequired = [](std::string_view message, std::string& out) {
            while (out.empty()) {
				std::print("{}: ", message);
                std::getline(std::cin, out);
                if (out.empty())
                    std::println("{}: ", "Input cannot be empty. Please try again.");
            }
        };
        auto promptOptional = [](std::string_view message, std::string& target) {
			std::print("{} [{}]: ", message, target);
            std::string input;
            if (std::getline(std::cin, input) && !input.empty())
                target = input;
        };

        promptRequired("Enter username", m_Username);
        promptOptional("Enter server address (host:port)", m_ServerIP);

        // Initiate connect (handle IP vs hostname)
        if (Walnut::Utils::IsValidIPAddress(m_ServerIP))
        {
            std::println("Connecting to {} ip", m_ServerIP);
            m_Client->ConnectToServer(m_ServerIP);
        }
        else
        {
            constexpr std::string_view defaultPort = "8192";
            const auto tokens = Walnut::Utils::SplitString(m_ServerIP, ':');
            const std::string hostname = tokens.empty() ? m_ServerIP : tokens[0];
            std::string_view port = (tokens.size() == 2) ? tokens[1] : defaultPort;

            std::println("Resolving hostname '{}'...", hostname);
            auto resolved = Walnut::Utils::ResolveDomainName(hostname);
            if (resolved.empty())
            {
                std::println("Failed to resolve hostname '{}'", hostname);
                // allow retry by leaving m_HeadlessPrompted == false (return early)
                return;
            }

            const std::string fullAddress = resolved + ":" + std::string(port);
            std::println("Resolved '{}' -> '{}' (port {})", hostname, resolved, port);
            std::println("Connecting to {}", fullAddress);
            m_Client->ConnectToServer(fullAddress);
        }

        m_HeadlessPrompted = true;
    }

    // Connection-state reporting. Handshake is not sent here to avoid race/duplicate sends.
    const auto status = m_Client->GetConnectionStatus();
    if (status == Walnut::Client::ConnectionStatus::Connecting)
    {
        std::println("Connecting...");
    }
    else if (status == Walnut::Client::ConnectionStatus::FailedToConnect)
    {
        std::println("Connection failed.");
        const auto &debugMessage = m_Client->GetConnectionDebugMessage();
        if (!debugMessage.empty())
            std::println("{}", debugMessage);

        // allow user to retry
        m_HeadlessPrompted = false;
        m_HeadlessHasSentHandshake = false;
    }
    else if (status == Walnut::Client::ConnectionStatus::Connected)
    {
		if (m_HeadlessPrompted && !m_HeadlessHasSentHandshake)
		{
			Walnut::BufferStreamWriter stream(m_ScratchBuffer);
			stream.WriteRaw<PacketType>(PacketType::ClientConnectionRequest);
			stream.WriteRaw<uint32_t>(m_Color); // Color
			stream.WriteString(m_Username);     // Username

			m_Client->SendBuffer(stream.GetBuffer());
			SaveConnectionDetails(m_ConnectionDetailsFilePath);
			m_HeadlessHasSentHandshake = true;
		}
    }
}
#endif

void ClientLayer::OnConnected()
{
    m_Console.ClearLog();
}

void ClientLayer::OnDisconnected()
{
    m_Console.AddItalicMessageWithColor(0xff8a8a8a, "Lost connection to server!");

#ifdef WL_HEADLESS
    // reset handshake/prompt state so headless can retry/prompt again
    m_HeadlessHasSentHandshake = false;
    m_HeadlessPrompted = false;
#endif
}

void ClientLayer::OnDataReceived(const Walnut::Buffer buffer)
{
	Walnut::BufferStreamReader stream(buffer);

	PacketType type;
	stream.ReadRaw<PacketType>(type);

	switch (type)
	{
	case PacketType::Message:
	{
		std::string fromUsername, message;
		stream.ReadString(fromUsername);
		stream.ReadString(message);

		// Find user
		if (m_ConnectedClients.contains(fromUsername))
		{
			const auto& clientInfo = m_ConnectedClients.at(fromUsername);
			m_Console.AddTaggedMessageWithColor(clientInfo.Color, fromUsername, message);
		}
		else if (fromUsername == "SERVER") // special message from server
		{
			m_Console.AddTaggedMessage(fromUsername, message);
		}
		else
		{
			std::println("[ERROR] Message from unknown user? This shouldn't happen...");
			// display message anyway
			m_Console.AddTaggedMessage(fromUsername, message);
		}

		break;
	}
	case PacketType::ClientConnectionRequest:
	{
		bool requestStatus;
		stream.ReadRaw<bool>(requestStatus);
		if (requestStatus)
		{
			// Defer connection message to after message history is received
			m_ShowSuccessfulConnectionMessage = true;
			// m_Console.AddItalicMessageWithColor(0xff8a8a8a, "Successfully connected to {} with username {}", m_ServerIP, m_Username);
		}
		else
		{
			m_Console.AddItalicMessageWithColor(0xfffa4a4a, "Server rejected connection with username {}", m_Username);
		}
		break;
	}
	case PacketType::ConnectionStatus:
		break;
	case PacketType::ClientList:
	{
		std::vector<UserInfo> clientList;
		stream.ReadArray(clientList);

		// Update our client list
		m_ConnectedClients.clear();
		for (const auto& client : clientList)
			m_ConnectedClients[client.Username] = client;

		break;
	}
	case PacketType::ClientConnect:
	{
		UserInfo newClient;
		stream.ReadObject(newClient);

		m_ConnectedClients[newClient.Username] = newClient;
		m_Console.AddItalicMessageWithColor(newClient.Color, "Welcome {}!", newClient.Username);

		break;
	}
	case PacketType::ClientUpdate:
		break;
	case PacketType::ClientDisconnect:
	{
		UserInfo disconnectedClient;
		stream.ReadObject(disconnectedClient);

		m_ConnectedClients.erase(disconnectedClient.Username);
		m_Console.AddItalicMessageWithColor(disconnectedClient.Color, "Goodbye {}!", disconnectedClient.Username);
		break;
	}
	case PacketType::ClientUpdateResponse:
		break;
	case PacketType::MessageHistory:
	{
		std::vector<ChatMessage> messageHistory;
		stream.ReadArray(messageHistory);
		for (const auto& message : messageHistory)
		{
			// find user color if connected
			uint32_t userColor = 0xffffffff;
			if (m_ConnectedClients.contains(message.Username))
				userColor = m_ConnectedClients.at(message.Username).Color;

			m_Console.AddTaggedMessageWithColor(userColor, message.Username, message.Message);
		}

		if (m_ShowSuccessfulConnectionMessage)
		{
			m_ShowSuccessfulConnectionMessage = false;
			m_Console.AddItalicMessageWithColor(0xff8a8a8a, "Successfully connected to {} with username {}", m_ServerIP, m_Username);
		}

		break;
	}
	case PacketType::ServerShutdown:
	{
		m_Console.AddItalicMessage("Server is shutting down... goodbye!");
		m_Client->Disconnect();
		break;
	}
	case PacketType::ClientKick:
	{
		m_Console.AddItalicMessage("You have been kicked by server!");
		std::string reason;
		stream.ReadString(reason);
		if (!reason.empty())
			m_Console.AddItalicMessage("Reason: {}", reason);

		m_Client->Disconnect();
		break;
	}
	default:
		break;
	}
}

void ClientLayer::SendChatMessage(std::string_view message)
{
	if (IsValidMessage(message))
	{
		std::string messageToSend = TrimMessage(message);
		Walnut::BufferStreamWriter stream(m_ScratchBuffer);
		stream.WriteRaw<PacketType>(PacketType::Message);
		stream.WriteString(messageToSend);
		m_Client->SendBuffer(stream.GetBuffer());

		// echo in own console
		m_Console.AddTaggedMessageWithColor(m_Color | 0xff000000, m_Username, messageToSend);
	}
}

void ClientLayer::SaveConnectionDetails(const std::filesystem::path& filepath)
{
	YAML::Emitter out;
	{
		out << YAML::BeginMap; // Root
		out << YAML::Key << "ConnectionDetails" << YAML::Value;

		out << YAML::BeginMap;
		out << YAML::Key << "Username" << YAML::Value << m_Username;
		out << YAML::Key << "Color" << YAML::Value << m_Color;
		out << YAML::Key << "ServerIP" << YAML::Value << m_ServerIP;
		out << YAML::EndMap;

		out << YAML::EndMap; // Root
	}

	std::ofstream fout(filepath);
	fout << out.c_str();
}

bool ClientLayer::LoadConnectionDetails(const std::filesystem::path& filepath)
{
	if (!std::filesystem::exists(filepath))
		return false;

	YAML::Node data;
	try
	{
		data = YAML::LoadFile(filepath.string());
	}
	catch (YAML::ParserException e)
	{
		std::print("[ERROR] Failed to load message history {}\n {}\n", filepath.string(), e.what());
		return false;
	}

	auto rootNode = data["ConnectionDetails"];
	if (!rootNode)
		return false;

	m_Username = rootNode["Username"].as<std::string>();

#ifndef WL_HEADLESS
	m_Color = rootNode["Color"].as<uint32_t>();
	ImVec4 color = ImColor(m_Color).Value;
	m_ColorBuffer[0] = color.x;
	m_ColorBuffer[1] = color.y;
	m_ColorBuffer[2] = color.z;
	m_ColorBuffer[3] = color.w;
#endif

	m_ServerIP = rootNode["ServerIP"].as<std::string>();

	return true;
}
