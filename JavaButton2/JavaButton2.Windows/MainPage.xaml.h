//
// MainPage.xaml.h
// Declaration of the MainPage class.
//

#pragma once

#include "JavaboPageInterface.h"
#include "JavaboContext.h"
#include "MainPage.g.h"

namespace JavaButton2
{
	/// <summary>
	/// An empty page that can be used on its own or navigated to within a Frame.
	/// </summary>
	public ref class MainPage sealed: public JavaboPageInterface
	{
	public:
		MainPage();
		virtual double JavaboHeight();
		virtual double JavaboWidth();
		virtual double ScreenHeight();
		virtual double ScreenWidth();
	protected:
		virtual void OnNavigatedTo(Windows::UI::Xaml::Navigation::NavigationEventArgs^ e) override;

	};
}
