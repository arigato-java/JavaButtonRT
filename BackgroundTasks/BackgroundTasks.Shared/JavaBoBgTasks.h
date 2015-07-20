#pragma once
#include "pch.h"

namespace BackgroundTasks
{
	[Windows::Foundation::Metadata::WebHostHidden]
	public ref class JavaBoBgTasks sealed: public Windows::ApplicationModel::Background::IBackgroundTask
	{
	public:
		JavaBoBgTasks();
		virtual void Run(Windows::ApplicationModel::Background::IBackgroundTaskInstance^ taskInstance);
	private:
		void UpdateTile();
	};
}
