/*
 * Copyright 2012, Clemens Zeidler <haiku@clemens-zeidler.de>
 * Distributed under the terms of the MIT License.
 */


#include <Application.h>
#include <Window.h>

#include <ALMLayout.h>
#include <ALMLayoutBuilder.h>

#include <ALMEditor.h>
#include <PreviewWindow.h>


using namespace BALM;


const uint32 kMsgEdit = '&edi';


class EditorWindow : public BWindow {
public:
	EditorWindow(BRect frame) 
		:
		BWindow(frame, "Layout Playground", B_TITLED_WINDOW,
			B_QUIT_ON_WINDOW_CLOSE | B_AUTO_UPDATE_SIZE_LIMITS)
	{
		AddShortcut('e', B_COMMAND_KEY, new BMessage(kMsgEdit));

		fLayout = new BALMLayout(10, 10);
		fLayout->SetInsets(5);
		BALMLayoutBuilder builder(this, fLayout);

		fEditor = new BALMEditor(fLayout);
		fEditor->SetEnableCreationMode(true);

		PostMessage(kMsgEdit);
	}

	~EditorWindow()
	{
		delete fEditor;
	}

	void MessageReceived(BMessage* message)
	{
		switch (message->what) {
			case kMsgEdit:
			{
				fEditor->StartEdit();
				break;
			}

			case kMsgLayoutEdited:
			{
				BSize minSize = fLayout->MinSize();
				BSize maxSize = fLayout->MaxSize();
				SetSizeLimits(minSize.width, maxSize.width, minSize.height,
					maxSize.height);

				fPreviewWindowManager.NotifyLayoutEdited();
				break;
			}

			default:
				BWindow::MessageReceived(message);
		}
	}

private:
			BALMLayout*			fLayout;

			BALMEditor*			fEditor;

			BALM::PreviewWindowManager	fPreviewWindowManager;
};


int
main()
{
	BApplication app("application/x-vnd.haiku.ALEditor");

	BWindow* window = new EditorWindow(BRect(100, 100, 500, 400));
	window->Show();

	app.Run();
	return 0;
}

