#include "pch.h"
#include "JavaBoBgTasks.h"

using namespace BackgroundTasks;
using namespace Platform;
using namespace Windows::ApplicationModel::Background;
using namespace Windows::UI::Notifications;
using namespace Windows::Data::Xml::Dom;

JavaBoBgTasks::JavaBoBgTasks() { }

void 
JavaBoBgTasks::Run(IBackgroundTaskInstance^ taskInstance)
{
	auto deferral = taskInstance->GetDeferral();
	UpdateTile();
	deferral->Complete();
}

void
JavaBoBgTasks::UpdateTile() {
	static const wchar_t* javaMessages[] = {
		L"30億のデバイスで走るJava",
		L"無料Javaのダウンロード",
		L"Java Update 利用可能",
		L"あなたとJava,\n今すぐダウンロー\nド",
		L"同意して無料ダウンロードを開始",
		nullptr
	};
	Platform::String^ textTagName(L"text");
	auto updater = TileUpdateManager::CreateTileUpdaterForApplication();
	updater->EnableNotificationQueue(true);
	updater->Clear();

	for (auto msg = javaMessages; *msg; msg++) {
		auto tileXml = TileUpdateManager::GetTemplateContent(TileTemplateType::TileSquare150x150Text01);
		tileXml->GetElementsByTagName(textTagName)->GetAt(0)->InnerText = ref new Platform::String(*msg);
		auto wideTileXml=TileUpdateManager::GetTemplateContent(TileTemplateType::TileWide310x150Text03);
		wideTileXml->GetElementsByTagName(textTagName)->GetAt(0)->InnerText = ref new Platform::String(*msg);
		auto node = tileXml->ImportNode(wideTileXml->GetElementsByTagName("binding")->GetAt(0),true);
		tileXml->GetElementsByTagName("visual")->Item(0)->AppendChild(node);
		updater->Update(ref new TileNotification(tileXml));
	}
}
