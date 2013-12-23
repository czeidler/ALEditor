#include "UIComponents.h"

#include "IconData.h"


Customizable*
Button::InstantiateCustomizable(const BMessage* archive)
{
	return new Button("Button", NULL);
}


Customizable*
TextControl::InstantiateCustomizable(const BMessage* archive)
{
	return new TextControl();
}


Customizable*
TextView::InstantiateCustomizable(const BMessage* archive)
{
	TextView* view = new TextView();
	view->SetText("Haiku is a new open-source operating system that specifically targets personal computing. Inspired by the BeOS, Haiku is fast, simple to use, easy to learn and yet very powerful.");
	ScrollView* scrollView = new ScrollView(view, true, true);
	scrollView->SetExplicitPreferredSize(BSize(220, 280));
	return scrollView;
}


Customizable*
RadioButton::InstantiateCustomizable(const BMessage* archive)
{
	RadioButton* view = new RadioButton("Radio Button");
	return view;
}


Customizable*
CheckBox::InstantiateCustomizable(const BMessage* archive)
{
	CheckBox* view = new CheckBox("Check Box");
	return view;
}


Customizable*
ProgressBar::InstantiateCustomizable(const BMessage* archive)
{
	ProgressBar* progressBar = new ProgressBar("0", "100");
	progressBar->SetExplicitPreferredSize(BSize(350, B_SIZE_UNSET));
	return progressBar;
}


Customizable*
StringView::InstantiateCustomizable(const BMessage* archive)
{
	return new StringView("String View");
}


Customizable*
ComboBox::InstantiateCustomizable(const BMessage* archive)
{
	BMenu* menu = new BMenu("menu");
	menu->AddItem(new BMenuItem("item 1", NULL));
	menu->AddItem(new BMenuItem("item 2", NULL));
	
	ComboBox* view = new ComboBox("Combo Box", menu);
	return view;
}



Customizable*
ListView::InstantiateCustomizable(const BMessage* archive)
{
	ListView* view = new ListView();
	view->AddItem(new BStringItem("Item 1"));
	view->AddItem(new BStringItem("Item 2"));
	view->AddItem(new BStringItem("Item 3"));
	view->AddItem(new BStringItem("Item 4"));
	view->AddItem(new BStringItem("Item 5"));
	view->AddItem(new BStringItem("Item 6"));
	view->AddItem(new BStringItem("Item 7"));
	view->AddItem(new BStringItem("Item 8"));
	view->AddItem(new BStringItem("Item 9"));
	
	ScrollView* scrollView = new ScrollView(view, true, true);
	scrollView->SetExplicitPreferredSize(BSize(220, 280));
	return scrollView;
}


Customizable*
Spacer::InstantiateCustomizable(const BMessage* archive)
{
	Spacer* spacer = new Spacer;
	BSize minSize;
	BSize maxSize;
	BSize prefSize;
	if (archive != NULL && archive->FindSize("min", &minSize) == B_OK
		&& archive->FindSize("max", &maxSize) == B_OK
		&& archive->FindSize("pref", &prefSize) == B_OK) {
		spacer->SetExplicitMinSize(minSize);
		spacer->SetExplicitMaxSize(maxSize);
		spacer->SetExplicitPreferredSize(prefSize);
	}
	return spacer;
}


Customizable*
HorizontalStrut::InstantiateCustomizable(const BMessage* archive)
{
	Spacer* spacer = new Spacer;
	spacer->MakeHorizontalStrut(30);
	return spacer;
}


Customizable*
VerticalStrut::InstantiateCustomizable(const BMessage* archive)
{
	Spacer* spacer = new Spacer;
	spacer->MakeVerticalStrut(30);
	return spacer;
}


Customizable*
HorizontalSpring::InstantiateCustomizable(const BMessage* archive)
{
	Spacer* spacer = new Spacer;
	spacer->MakeHorizontalSpring(-1);
	return spacer;
}


Customizable*
VerticalSpring::InstantiateCustomizable(const BMessage* archive)
{
	Spacer* spacer = new Spacer;
	spacer->MakeVerticalSpring(-1);
	return spacer;
}


static CustomizableInstaller<Button> sButtonInstaller(kButtonIconBits,
	BRect(0, 0, kButtonIconWidth - 1, kButtonIconHeight - 1));
static CustomizableInstaller<RadioButton> sRadioButtonInstaller(kRadioButtonBits,
	BRect(0, 0, kRadioButtonIconWidth - 1, kRadioButtonIconHeight - 1));
static CustomizableInstaller<CheckBox> sCheckBoxInstaller(kCheckButtonBits,
	BRect(0, 0, kCheckButtonIconWidth - 1, kCheckButtonIconHeight - 1));
static CustomizableInstaller<ComboBox> sComboBoxInstaller(kComboBoxBits,
	BRect(0, 0, kComboBoxIconWidth - 1, kComboBoxIconHeight - 1));
static CustomizableInstaller<TextControl> sTextControlInstaller(kTextControlBits,
	BRect(0, 0, kTextControlIconWidth - 1, kTextControlIconHeight - 1));
static CustomizableInstaller<TextView> sTextViewInstaller(kTextViewBits,
	BRect(0, 0, kTextViewIconWidth - 1, kTextViewIconHeight - 1));
static CustomizableInstaller<ProgressBar> sProgressBarInstaller(kStatusBarBits,
	BRect(0, 0, kStatusBarIconWidth - 1, kStatusBarIconHeight - 1));
static CustomizableInstaller<StringView> sStringViewInstaller(kStringViewBits,
	BRect(0, 0, kStringViewIconWidth - 1, kStringViewIconHeight - 1));
static CustomizableInstaller<ListView> sListViewInstaller(kListViewBits,
	BRect(0, 0, kListViewIconWidth - 1, kListViewIconHeight - 1));

/*
static CustomizableInstaller<Spacer> sSpacerInstaller;
static CustomizableInstaller<HorizontalStrut> sVStrutInstaller;
static CustomizableInstaller<VerticalStrut> sHStrutInstaller;
static CustomizableInstaller<HorizontalSpring> sVSpringInstaller;
static CustomizableInstaller<VerticalSpring> sHSpringInstaller;
*/
