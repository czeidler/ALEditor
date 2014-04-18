/*
 * Copyright 2012, Clemens Zeidler <haiku@clemens-zeidler.de>
 * Distributed under the terms of the MIT License.
 */


#include <Alert.h>
#include <Application.h>
#include <Button.h>
#include <File.h>
#include <Path.h>
#include <RadioButton.h>
#include <Roster.h>
#include <Window.h>

#include <ALMLayout.h>
#include <ALMLayoutBuilder.h>

#include <LayoutArchive.h>


const char* kGUIFileName = "TestLayout";


class LoadLayoutArchiveWindow : public BWindow {
public:
	LoadLayoutArchiveWindow(BRect frame) 
		:
		BWindow(frame, "Layout Loaded From Archive", B_TITLED_WINDOW, B_QUIT_ON_WINDOW_CLOSE
			| B_AUTO_UPDATE_SIZE_LIMITS)
	{
		fLayout = new BALMLayout(10, 10);
		BALM::BALMLayoutBuilder builder(this, fLayout);

		// Restore gui specifications. 
		BFile guiFile;
		_FindGUISpecifications(kGUIFileName, guiFile);

		LayoutArchive layoutArchive(fLayout);
		if (layoutArchive.RestoreFromAttribute(&guiFile, "layout") != B_OK) {
			BString text = "Can't find layout specification file: \"";
			text << kGUIFileName;
			text << "\"";
			BAlert* alert = new BAlert("Layout Specifications Not Found",
				text, "Quit");
			alert->Go();
			PostMessage(B_QUIT_REQUESTED);
		}

		// Access the views in the layout.

		BButton* button = layoutArchive.FindView<BButton>("ButtonTest");
		if (button != NULL)
			button->SetLabel("Hey");

		BRadioButton* radioButton
			= layoutArchive.FindView<BRadioButton>("RadioButtonTest");
		if (radioButton != NULL)
			radioButton->SetLabel("World");
	}

private:
	status_t _FindGUISpecifications(const char* name, BFile& file)
	{
		// First look in the directory where the app binary is
		app_info appInfo;
		be_app->GetAppInfo(&appInfo);
		BPath appPath(&appInfo.ref);
		BPath path;
		appPath.GetParent(&path);
		path.Append(name);
		status_t status= file.SetTo(path.Path(), B_READ_ONLY);
		if (status == B_OK)
			return status;
		// try the directory the app is called from
		return file.SetTo(name, B_READ_ONLY);
	}

private:
			BALMLayout*			fLayout;
};


int
main()
{
	BApplication app("application/x-vnd.haiku.ALELayoutArchive");

	BWindow* window = new LoadLayoutArchiveWindow(BRect(100, 100, 500, 350));
	window->Show();

	app.Run();
	return 0;
}

