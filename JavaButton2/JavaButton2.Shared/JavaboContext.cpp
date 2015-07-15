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
	javaSoundIdx(0L),
	botanVel(std::complex<double>(0., 0.)),
	sensorUpdated(false)
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
	sensorUpdated = false;
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
	if (sensorUpdated) {
		Dispatcher->RunAsync(Windows::UI::Core::CoreDispatcherPriority::Normal,
			ref new Windows::UI::Core::DispatchedHandler([=]() {
			javaboMovementMutex.lock();
			auto now = std::chrono::high_resolution_clock::now();
			auto t = 0.01*static_cast<double>(std::chrono::duration_cast<std::chrono::milliseconds>(now-lastSensorUpdate).count());
			std::complex<double> acceleration(args->Reading->AccelerationX, -args->Reading->AccelerationY);
			botanVel = botanVel + t*acceleration;
			auto botanPos = std::complex<double>(botanTranslateX, botanTranslateY) + t*botanVel;
			const auto damp=0.9;
			const auto scrdim=0.4;
			const auto scrdim_guard=0.39;
			const auto scrw = m_page->ScreenWidth();
			const auto scrh = m_page->ScreenHeight();
			auto bounce=false;
			if (botanPos.real() < -scrdim*scrw) { 
				botanVel = damp*std::complex<double>(abs(botanVel.real()), botanVel.imag());
				botanPos = std::complex<double>(-scrdim_guard*scrw, botanPos.imag());
				bounce=true;
			} else if (botanPos.real() > scrdim*scrw) {
				botanVel = damp*std::complex<double>(-abs(botanVel.real()), botanVel.imag());
				botanPos = std::complex<double>(scrdim_guard*scrw, botanPos.imag());
				bounce=true;
			}
			if (botanPos.imag() < -scrdim*scrh) {
				botanVel = damp*std::complex<double>(botanVel.real(), abs(botanVel.imag()));
				botanPos = std::complex<double>(botanPos.real(), -scrdim_guard*scrh);
				bounce=true;
			} else if (botanPos.imag() > scrdim*scrh) {
				botanVel = damp*std::complex<double>(botanVel.real(), -abs(botanVel.imag()));
				botanPos = std::complex<double>(botanPos.real(), scrdim_guard*scrh);
				bounce=true;
			}
			if (bounce) { sayJava(nullptr); }
			botanTranslateX=botanPos.real();
			botanTranslateY=botanPos.imag();
			lastSensorUpdate = now;
			javaboMovementMutex.unlock();
			OnPropertyChanged("botanTranslateX");
			OnPropertyChanged("botanTranslateY");
		}));
	} else {
		javaboMovementMutex.lock();
		lastSensorUpdate = std::chrono::high_resolution_clock::now();
		sensorUpdated = true;
		javaboMovementMutex.unlock();
	}
}
