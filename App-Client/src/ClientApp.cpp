#ifdef WL_HEADLESS
#include "Walnut/Application.h"
#include "Walnut/EntryPoint.h"
#else
#include "Walnut/ApplicationGUI.h"
#endif

#include "ClientLayer.h"

const static uint64_t s_BufferSize = 1024;
static uint8_t *s_Buffer = new uint8_t[s_BufferSize];

Walnut::Application *Walnut::CreateApplication(int argc, char **argv)
{
	Walnut::ApplicationSpecification spec;
	spec.Name = "Walnut Chat Client 1.2";
#ifndef WL_HEADLESS
	spec.IconPath = "res/Walnut-Icon.png";
	spec.CustomTitlebar = true;
	spec.CenterWindow = true;
#endif

	Walnut::Application *app = new Walnut::Application(spec);
	std::shared_ptr<ClientLayer> clientLayer = std::make_shared<ClientLayer>();
	app->PushLayer(clientLayer);
#ifndef WL_HEADLESS
	app->SetMenubarCallback([app, clientLayer]()
							{
		if (ImGui::BeginMenu("File"))
		{
			if (ImGui::MenuItem("Disconnect"))
				clientLayer->OnDisconnectButton();

			if (ImGui::MenuItem("Exit"))
				app->Close();
			ImGui::EndMenu();
		} });
#endif
	return app;
}