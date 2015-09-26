//
// MainPage.xaml.cpp
// Implementation of the MainPage class.
//

#include "pch.h"
#include "MainPage.xaml.h"

using namespace JavaButton2;

using namespace Platform;
using namespace Windows::Foundation;
using namespace Windows::Foundation::Collections;
using namespace Windows::UI::Xaml;
using namespace Windows::UI::Xaml::Controls;
using namespace Windows::UI::Xaml::Controls::Primitives;
using namespace Windows::UI::Xaml::Data;
using namespace Windows::UI::Xaml::Input;
using namespace Windows::UI::Xaml::Media;
using namespace Windows::UI::Xaml::Navigation;
using namespace Windows::ApplicationModel::Background;
using namespace concurrency;

// The Blank Page item template is documented at http://go.microsoft.com/fwlink/?LinkId=234238

MainPage::MainPage()
{
	InitializeComponent();
	Windows::Graphics::Display::DisplayInformation::AutoRotationPreferences=
		Windows::Graphics::Display::DisplayInformation::GetForCurrentView()->CurrentOrientation;
	auto jbc=ref new JavaboContext(this);
	DataContext = jbc;
	Loaded += ref new RoutedEventHandler([=](Object ^sender, RoutedEventArgs^ args) {
		jbc->resetBotanPosition();
	});
	SizeChanged += ref new SizeChangedEventHandler([=](Object ^sender, SizeChangedEventArgs ^e) {
		jbc->resetBotanPosition();
	});
}

/// <summary>
/// Invoked when this page is about to be displayed in a Frame.
/// </summary>
/// <param name="e">Event data that describes how this page was reached.  The Parameter
/// property is typically used to configure the page.</param>
void MainPage::OnNavigatedTo(NavigationEventArgs^ e)
{
	(void) e;	// Unused parameter

	// TODO: Prepare page for display here.

	// TODO: If your application contains multiple pages, ensure that you are
	// handling the hardware Back button by registering for the
	// Windows::Phone::UI::Input::HardwareButtons.BackPressed event.
	// If you are using the NavigationHelper provided by some templates,
	// this event is handled for you.

	auto reqAcc = create_task(BackgroundExecutionManager::RequestAccessAsync());
	reqAcc.then([=](BackgroundAccessStatus accessStatus) {
		if (accessStatus != BackgroundAccessStatus::Denied) {
			Platform::String^ taskName = L"JavaBoBgTasks";
			auto tasks=BackgroundTaskRegistration::AllTasks;
			for (auto i = tasks->First(); i->HasCurrent; i->MoveNext()) {
				if (i->Current->Value->Name == taskName) {
					i->Current->Value->Unregister(true);
				}
			}
			auto taskBuilder = ref new BackgroundTaskBuilder();
			taskBuilder->Name=taskName;
			taskBuilder->TaskEntryPoint = L"BackgroundTasks.JavaBoBgTasks";
			taskBuilder->SetTrigger(ref new TimeTrigger(720,false));
			auto registration = taskBuilder->Register();
		}
	});
}

double
MainPage::JavaboHeight() {
	return javabotan->ActualHeight;
}

double
MainPage::JavaboWidth() {
	return javabotan->ActualWidth;
}

double
MainPage::ScreenHeight() {
	return Window::Current->Bounds.Height;
}

double
MainPage::ScreenWidth() {
	return Window::Current->Bounds.Width;
}

Windows::UI::Xaml::DependencyObject^
MainPage::getJavabo() {
	return javabotan;
}
