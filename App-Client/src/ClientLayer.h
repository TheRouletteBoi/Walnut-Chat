#pragma once

#include "Walnut/Layer.h"
#include "Walnut/Networking/Client.h"

#ifdef WL_HEADLESS
#include "HeadlessConsole.h"
#else
#include "Walnut/UI/Console.h"
#endif

#include "UserInfo.h"

#include <set>
#include <filesystem>

class ClientLayer : public Walnut::Layer
{
public:
	virtual void OnAttach() override;
	virtual void OnDetach() override;
	virtual void OnUIRender() override;
#ifdef WL_HEADLESS
	virtual void OnUpdate(float ts) override;
#endif

	bool IsConnected() const;
	void OnDisconnectButton();

private:
	// UI
	void UI_ConnectionModal();
	void UI_ClientList();

	// Server event callbacks
	void TempConnectionSetup();
	void OnConnected();
	void OnDisconnected();
	void OnDataReceived(const Walnut::Buffer buffer);

	void SendChatMessage(std::string_view message);

private:
	void SaveConnectionDetails(const std::filesystem::path &filepath);
	bool LoadConnectionDetails(const std::filesystem::path &filepath);

private:
	std::unique_ptr<Walnut::Client> m_Client;

#ifdef WL_HEADLESS
	HeadlessConsole m_Console{"Chat"};
#else
	Walnut::UI::Console m_Console{"Chat"};
#endif

	std::string m_ServerIP;
	std::filesystem::path m_ConnectionDetailsFilePath = "ConnectionDetails.yaml";

	Walnut::Buffer m_ScratchBuffer;

	float m_ColorBuffer[4] = {1.0f, 1.0f, 1.0f, 1.0f};

	std::string m_Username;
	uint32_t m_Color = 0xffffffff;

	std::map<std::string, UserInfo> m_ConnectedClients;
	bool m_ConnectionModalOpen = false;
	bool m_ShowSuccessfulConnectionMessage = false;
};