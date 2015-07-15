#include "pch.h"
#include "Common/DelegateCommand.h"
#include "JavaboContext.h"
#include "Sound/SoundFileReader.h"

using namespace JavaButton2;
using namespace Windows::Devices::Sensors;
using namespace Windows::UI::Xaml;
using namespace Windows::Foundation;

JavaboContext::JavaboContext(JavaboPageInterface^ page) :
	m_page(page),
	acc(Windows::Devices::Sensors::Accelerometer::GetDefault()),
	javaSoundIdx(0L)
{
	javaCommand = ref new DelegateCommand(ref new ExecuteDelegate(this, &JavaboContext::sayJava),
		ref new CanExecuteDelegate(this, &JavaboContext::canSayJava));
	botanTranslateX = 0.;
	botanTranslateY = 0.;

	prepareAccelerometer();
	prepareSound();
}

void
JavaboContext::prepareSound() {
	m_player = std::shared_ptr<XAudio2SoundPlayer>(new XAudio2SoundPlayer(48000));
	Application::Current->Suspending += ref new SuspendingEventHandler(this, &JavaboContext::OnSuspending);
	Application::Current->Resuming += ref new EventHandler<Platform::Object^>(this, &JavaboContext::OnResuming);

	//Application::Current->Resuming;
	SoundFileReader r(ref new Platform::String(L"Assets/java32.wav"));
	for (auto i = 0UL; i < 4UL; i++) {
		javaSndId.push_back(m_player->AddSound(r.GetSoundFormat(), r.GetSoundData()));
	}
	javaSoundIdx=0;
}

void
JavaboContext::OnSuspending(Platform::Object^ sender, Windows::ApplicationModel::SuspendingEventArgs^ e)
{
	m_player->Suspend();
	if (acc) {
		acc->ReadingChanged -= acc_token;
	}
}

void
JavaboContext::OnResuming(Object^ sender, Object^ e) {
	m_player->Resume();
	prepareAccelerometer();
}

void
JavaboContext::prepareAccelerometer() {
	// Accelerometer
	if (acc) {
		acc_token= acc->ReadingChanged += ref new TypedEventHandler<Accelerometer^, AccelerometerReadingChangedEventArgs^>(this, &JavaboContext::AccelEventHandler);
	}
}

void
JavaboContext::sayJava(Platform::Object^ parameter) {
	javaSoundIndexLock.lock();
	auto playIdx = javaSoundIdx;
	javaSoundIdx = (playIdx + 1UL) % javaSndId.size();
	javaSoundIndexLock.unlock();
	m_player->PlaySound(javaSndId[playIdx]);
}

bool
JavaboContext::canSayJava(Platform::Object^ parameter) {
	return true; // we can always say java
}

void
JavaboContext::AccelEventHandler(Accelerometer^ sensor, AccelerometerReadingChangedEventArgs^ args) {
	Dispatcher->RunAsync(Windows::UI::Core::CoreDispatcherPriority::Normal,
		ref new Windows::UI::Core::DispatchedHandler([=]() {
			botanTranslateX += 10.* args->Reading->AccelerationX;
			OnPropertyChanged("botanTranslateX");
			botanTranslateY -= 10.* args->Reading->AccelerationY;
			OnPropertyChanged("botanTranslateY");
		}));
}
