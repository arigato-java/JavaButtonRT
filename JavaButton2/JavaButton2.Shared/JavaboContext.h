#pragma once
#include "pch.h"
#include "Common/BindableBase.h"
#include "JavaboPageInterface.h"
#include "Sound/XAudio2SoundPlayer.h"

namespace JavaButton2 {
	[Windows::UI::Xaml::Data::Bindable]
	[Windows::Foundation::Metadata::WebHostHidden]
	public ref class JavaboContext sealed: public Common::BindableBase {
	public:
		JavaboContext(JavaboPageInterface^ page);
		
		void sayJava(Platform::Object^ parameter);
		bool canSayJava(Platform::Object^ parameter);

		void AccelEventHandler(Windows::Devices::Sensors::Accelerometer^ sensor, Windows::Devices::Sensors::AccelerometerReadingChangedEventArgs^ args);

		property Windows::UI::Xaml::Input::ICommand^ javaCommand;
		property double botanTranslateX;
		property double botanTranslateY;
	private:
		void prepareSound();
		void prepareAccelerometer();
		void OnSuspending(Platform::Object^ sender, Windows::ApplicationModel::SuspendingEventArgs^ e);
		void OnResuming(Object^ sender, Object^ e);

		std::vector<size_t> javaSndId;
		size_t javaSoundIdx;
		JavaboPageInterface^ m_page;
		std::mutex javaSoundIndexLock;
		std::shared_ptr<XAudio2SoundPlayer> m_player;

		Windows::Devices::Sensors::Accelerometer^ acc;
		Windows::Foundation::EventRegistrationToken acc_token;
		std::mutex javaboMovementMutex;
		std::complex<double> botanVel;
		std::chrono::time_point<std::chrono::system_clock,std::chrono::system_clock::duration> lastSensorUpdate;
		bool sensorUpdated;
	};
}
