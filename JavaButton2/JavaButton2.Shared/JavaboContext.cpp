#include "pch.h"
#include "Common/DelegateCommand.h"
#include "JavaboContext.h"
#include "Sound/SoundFileReader.h"

using namespace JavaButton2;
using namespace Windows::Devices::Sensors;
using namespace Windows::UI::Xaml;
using namespace Windows::Foundation;
using namespace Windows::UI::Xaml::Media::Animation;

JavaboContext::JavaboContext(JavaboPageInterface^ page) :
	m_page(page),
	acc(Windows::Devices::Sensors::Accelerometer::GetDefault()),
	javaSoundIdx(0L),
	botanVel(std::complex<double>(0., 0.)),
	sensorUpdated(false)
{
	javaCommand = ref new DelegateCommand(ref new ExecuteDelegate(this, &JavaboContext::sayJava),
		ref new CanExecuteDelegate(this, &JavaboContext::canSayJava));
	prepareAccelerometer();
	prepareSound();
}

void
JavaboContext::resetBotanPosition() {
	botanTranslateX = .5*(m_page->ScreenWidth() - m_page->JavaboWidth());
	botanTranslateY = .7*m_page->ScreenHeight() - .5*m_page->JavaboHeight();
	OnPropertyChanged("botanTranslateX");
	OnPropertyChanged("botanTranslateY");
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
	javaboMovementMutex.lock();
	if (sensorUpdated) {
		Dispatcher->RunAsync(Windows::UI::Core::CoreDispatcherPriority::Normal,
			ref new Windows::UI::Core::DispatchedHandler([=]() {
				auto now = std::chrono::high_resolution_clock::now();
				auto t1000l=std::chrono::duration_cast<std::chrono::milliseconds>(now-lastSensorUpdate).count();
				auto t1000=static_cast<double>(t1000l);
				auto t = 0.001*t1000;
				std::complex<double> acceleration(args->Reading->AccelerationX, -args->Reading->AccelerationY);
				const auto masaz=0.99;
				botanVel = masaz*botanVel + t*980.*acceleration;
				auto botanPos = std::complex<double>(botanTranslateX, botanTranslateY) + t*botanVel;
				const auto scrw = m_page->ScreenWidth();
				const auto scrh = m_page->ScreenHeight();
				const auto botw = m_page->JavaboWidth();
				const auto both = m_page->JavaboHeight();
				const auto left=0.;
				const auto right=scrw-botw;
				const auto top=0.;
				const auto bottom=scrh-both;
				const auto damp=0.95;
				const auto guard=1.;
				auto bounce=false;
				if (botanPos.real() < left) {
					botanVel = damp*std::complex<double>(abs(botanVel.real()), botanVel.imag());
					botanPos = std::complex<double>(left+guard, botanPos.imag());
					bounce=true;
				} else if (botanPos.real() > right) {
					botanVel = damp*std::complex<double>(-abs(botanVel.real()), botanVel.imag());
					botanPos = std::complex<double>(right-guard, botanPos.imag());
					bounce=true;
				}
				if (botanPos.imag() < top) {
					botanVel = damp*std::complex<double>(botanVel.real(), abs(botanVel.imag()));
					botanPos = std::complex<double>(botanPos.real(), top+guard);
					bounce=true;
				} else if (botanPos.imag() > bottom) {
					botanVel = damp*std::complex<double>(botanVel.real(), -abs(botanVel.imag()));
					botanPos = std::complex<double>(botanPos.real(), bottom-guard);
					bounce=true;
				}
				botanGotoXY(botanPos.real(), botanPos.imag(), t1000l);
				if (bounce) { sayJava(nullptr); }
				lastSensorUpdate = now;
			}));
	} else {
		lastSensorUpdate = std::chrono::high_resolution_clock::now();
		sensorUpdated = true;
	}
	javaboMovementMutex.unlock();
}

void
JavaboContext::botanGotoXY(double X, double Y, long long timespan) {
	TimeSpan t;
	t.Duration=timespan*10000LL;
	DoubleAnimation^ animX = ref new DoubleAnimation();
	animX->Duration = Duration(t);
	//animX->From=botanTranslateX;
	animX->To=X;
	animX->EnableDependentAnimation=true;
	DoubleAnimation^ animY = ref new DoubleAnimation();
	animY->Duration=Duration(t);
	//animY->From=botanTranslateY;
	animY->To=Y;
	animY->EnableDependentAnimation=true;
	Storyboard^ sb=ref new Storyboard();
	sb->Children->Append(animX);
	sb->SetTarget(animX,m_page->getJavabo());
	sb->SetTargetProperty(animX, "(Canvas.Left)");
	sb->Children->Append(animY);
	sb->SetTarget(animY, m_page->getJavabo());
	sb->SetTargetProperty(animY, "(Canvas.Top)");
	sb->Begin();
}
