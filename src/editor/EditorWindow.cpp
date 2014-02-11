/*
 * Copyright 2007-2008, Christof Lutteroth, lutteroth@cs.auckland.ac.nz
 * Copyright 2007-2008, James Kim, jkim202@ec.auckland.ac.nz
 * Copyright 2011, Clemens Zeidler <haiku@clemens-zeidler.de>
 * Distributed under the terms of the MIT License.
 */


#include "EditorWindow.h"

#include <math.h>
#include <stdio.h>
#include <stdlib.h>

#include <Autolock.h>
#include <Box.h>
#include <ControlLook.h>
#include <Menu.h>
#include <Message.h>
#include <Rect.h>
#include <Screen.h>
#include <Slider.h>

#include <ALMEditor.h>
#include <ALMLayout.h>
#include <ALMLayoutBuilder.h>

#include "Variable.h"

#include "components/UIComponents.h"
#include "CustomizableRosterView.h"
#include "CustomizableNodeFactory.h"
#include "CustomizableNodeView.h"
#include "LayoutArchive.h"
#include "LayoutEditView.h"
#include "PreviewWindow.h"
#include "TrashView.h"


const uint32 kMsgDecreaseWindowPreview = '&DeW';
const uint32 kMsgEnlargeWindowPreview = '&PrW';


using BALM::LayoutEditView;


const int kSliderSteps = 1000;


template<class SizePolicy, class DirectionPolicy>
class SizeValueConfigView : public BView {
public:
	SizeValueConfigView(BLayoutItem* item, LayoutEditView* editView)
		:
		BView("SizeValueConfig", 0),
		fItem(item),
		fEditView(editView),
		fSliderMax(1000)
	{
		SetExplicitPreferredSize(BSize(-1, B_SIZE_UNSET));

		fTextControl = new BTextControl("", "", "0",
			new BMessage(kMsgTextChanged));
		fSlider = new BSlider("", "", NULL, 0,
			kSliderSteps * log10(fSliderMax), B_HORIZONTAL);
		fSlider->SetModificationMessage(new BMessage(kMsgSliderChanged));
		fSlider->SetExplicitPreferredSize(BSize(-1, B_SIZE_UNSET));

		SetViewColor(ui_color(B_PANEL_BACKGROUND_COLOR));

		float spacing = be_control_look->DefaultItemSpacing();
		BALMLayout* layout = new BALMLayout(spacing, spacing);
		SetLayout(layout);
		
		BALM::BALMLayoutBuilder(layout)
			.Add(fSlider, layout->Left(), layout->Top(), NULL,
				layout->Bottom())
			.AddToRight(fTextControl, layout->Right());
	}

	void AttachedToWindow()
	{
		fTextControl->SetTarget(this);
		fSlider->SetTarget(this);

		_LoadValues();
	}

	void MessageReceived(BMessage* message)
	{
		switch (message->what) {
			case kMsgTextChanged:
			{
				DirectionPolicy directionPolicy;
				SizePolicy sizePolicy;
				BSize size = sizePolicy.Get(fItem);
				int value;
				if (!_GetValue(fTextControl, value))
					_SetValue(fTextControl, directionPolicy.Get(size));
				else {
					// TODO check if value is bigger than the slider max
					_SetValue(fSlider, value);
					_SetItemValue(value);
				}
				break;
			}

			case kMsgSliderChanged:
			{
				float value = _GetValue(fSlider);
				if (value >= fSliderMax)
					value = -1;
				_SetValue(fTextControl, (int)value);
				_SetItemValue(value);
				break;
			}

			default:
				BView::MessageReceived(message);
		}
	}

	void SetEnabled(bool enabled)
	{
		fSlider->SetEnabled(enabled);
		fTextControl->SetEnabled(enabled);
	}

private:
	enum {
		kMsgTextChanged = '&txc',
		kMsgSliderChanged	
	};


	void _LoadValues()
	{
		DirectionPolicy directionPolicy;
		SizePolicy sizePolicy;
		BSize size = sizePolicy.Get(fItem);
		_SetValue(fTextControl, (int)directionPolicy.Get(size));
		_SetValue(fSlider, directionPolicy.Get(size));
	}

	void _SetValue(BTextControl* control, int value)
	{
		BString numberString;
		numberString.SetToFormat("%i", value);
		control->SetText(numberString);
	}

	bool _GetValue(BTextControl* control, int& value)
	{
		BString numberString = control->Text();
		value = atoi(numberString.String());
		return true;
	}

	void _SetValue(BSlider* control, float value)
	{
		if (value == -1)
			value = fSliderMax;
		value = log10(value) * kSliderSteps;
		control->SetValue((int32)value);
	}

	float _GetValue(BSlider* control)
	{
		float value = control->Value();
		value /= kSliderSteps;
		value = pow(10, value);
		return value;
	}

	void _SetItemValue(float value)
	{
		BAutolock _(fEditView->Window());

		DirectionPolicy directionPolicy;
		SizePolicy sizePolicy;
		BSize size = sizePolicy.Get(fItem);
		directionPolicy.Set(size, value);
		sizePolicy.Set(fItem, size);
		fEditView->Invalidate();
	}

private:
			BLayoutItem*		fItem;
			LayoutEditView*		fEditView;

			BSlider*			fSlider;
			BTextControl*		fTextControl;

			int					fSliderMax;
};


class WidthDirectionPolicy {
public:
	float Get(BSize& size)
	{
		return size.width;
	}

	void Set(BSize& size, float value)
	{
		size.width = value;
	}
};


class HeightDirectionPolicy {
public:
	float Get(BSize& size)
	{
		return size.height;
	}

	void Set(BSize& size, float value)
	{
		size.height = value;
	}
};


template<class SizePolicy>
class SizeConfigView : public BView {
public:
	SizeConfigView(BLayoutItem* item, LayoutEditView* editView)
		:
		BView("SizeConfig", 0),
		fItem(item),
		fEditView(editView)
	{
	}

	void AttachedToWindow()
	{
		SetViewColor(ui_color(B_PANEL_BACKGROUND_COLOR));

		float spacing = be_control_look->DefaultItemSpacing();
		BALMLayout* layout = new BALMLayout(spacing, spacing);
		SetLayout(layout);

		BStringView* widthLabel = new BStringView("", "Width:");
		BStringView* heightLabel = new BStringView("", "Height:");

		SizeValueConfigView<SizePolicy, WidthDirectionPolicy>* widthView
			= new SizeValueConfigView<SizePolicy, WidthDirectionPolicy>(fItem,
				fEditView);
		SizeValueConfigView<SizePolicy, HeightDirectionPolicy>* heightView
			= new SizeValueConfigView<SizePolicy, HeightDirectionPolicy>(fItem,
				fEditView);
				
		BALM::BALMLayoutBuilder(layout)
			.Add(widthLabel, layout->Left(), layout->Top())
			.AddToRight(widthView, layout->Right())
			.StartingAt(widthLabel)
				.AddBelow(heightLabel, layout->Bottom())
				.AddToRight(heightView, layout->Right());
	}

private:
			BLayoutItem*		fItem;
			LayoutEditView*		fEditView;
};


class PrefSizePolicy {
public:
	BSize Get(BLayoutItem* item)
	{
		return item->PreferredSize();
	}

	void Set(BLayoutItem* item, BSize size)
	{
		item->SetExplicitPreferredSize(size);
	}
};


class HMinMaxSizePolicy {
public:
	BSize Get(BLayoutItem* item)
	{
		return item->MinSize();
	}

	void Set(BLayoutItem* item, BSize size)
	{
		BSize minSize = item->MinSize();
		minSize.width = size.width;
		item->SetExplicitMinSize(minSize);
		BSize maxSize = item->MaxSize();
		maxSize.width = size.width;
		item->SetExplicitMaxSize(maxSize);
	}
};


class VMinMaxSizePolicy {
public:
	BSize Get(BLayoutItem* item)
	{
		return item->MinSize();
	}

	void Set(BLayoutItem* item, BSize size)
	{
		BSize minSize = item->MinSize();
		minSize.height = size.height;
		item->SetExplicitMinSize(minSize);
		BSize maxSize = item->MaxSize();
		maxSize.height = size.height;
		item->SetExplicitMaxSize(maxSize);
	}
};


class LayoutItemConfigView : public BView {
public:
	LayoutItemConfigView(BLayoutItem* item, LayoutEditView* editView)
		:
		BView("LayoutEditView", 0),
		fItem(item),
		fEditView(editView)
	{
	}

	void AttachedToWindow()
	{
		SetViewColor(ui_color(B_PANEL_BACKGROUND_COLOR));

		float spacing = be_control_look->DefaultItemSpacing();
		BALMLayout* layout = new BALMLayout(spacing, spacing);
		SetLayout(layout);

		fPrefSizeBox = new BBox("PrefSizeBox");
		fPrefSizeBox->SetLabel("Preferred Size");
		BALMLayout* prefLayout = new BALMLayout(spacing, spacing);
		fPrefSizeBox->SetLayout(prefLayout);
		
		BRect innerFrame = fPrefSizeBox->InnerFrame();
		BRect frame = fPrefSizeBox->Frame();
		prefLayout->SetInsets(innerFrame.left - frame.left + spacing,
			2 * fPrefSizeBox->TopBorderOffset() + spacing,
			frame.right - innerFrame.right + spacing,
			frame.bottom - innerFrame.bottom + spacing);

		BView* prefSizeConfig = new SizeConfigView<PrefSizePolicy>(fItem,
			fEditView);
		prefLayout->AddView(prefSizeConfig, prefLayout->Left(),
			prefLayout->Top(), prefLayout->Right(), prefLayout->Bottom());

		layout->AddView(fPrefSizeBox, layout->Left(), layout->Top(),
			layout->Right(), layout->Bottom());
	}

private:
			BLayoutItem*		fItem;
			LayoutEditView*		fEditView;
			BBox*				fPrefSizeBox;
};


template<class StrutSizePolicy, class SpringSizePolicy, class DirectionPolicy>
class StrutSpringSwitcherBox : public BBox {
public:
	StrutSpringSwitcherBox(Spacer* item, LayoutEditView* editView,
		const char* label)
		:
		BBox(""),
		fItem(item),
		fEditView(editView),
		fLabel(label),
		fValueConfigView(NULL)
	{
		fStrutConfigView
			= new SizeValueConfigView<StrutSizePolicy, DirectionPolicy>(
				fItem, fEditView);
		fSpringConfigView
			= new SizeValueConfigView<SpringSizePolicy, DirectionPolicy>(
				fItem, fEditView);
	}

	~StrutSpringSwitcherBox()
	{
		fStrutConfigView->RemoveSelf();
		delete fStrutConfigView;
		fSpringConfigView->RemoveSelf();
		delete fSpringConfigView;
	}

	void AttachedToWindow()
	{
		BPopUpMenu* menu = new BPopUpMenu("Select");
		fStrutMenuItem = new BMenuItem("Strut", new BMessage(kMsgStrutMenu));
		fSpringMenuItem = new BMenuItem("Spring", new BMessage(kMsgSpringMenu));
		fNeutralMenuItem = new BMenuItem("Neutral",
			new BMessage(kMsgNeutralMenu));
		menu->AddItem(fStrutMenuItem);
		menu->AddItem(fSpringMenuItem);
		menu->AddItem(fNeutralMenuItem);
		menu->SetTargetForItems(this);
		BMenuField* menuField = new BMenuField("", fLabel, menu);
		SetLabel(menuField);

		float spacing = be_control_look->DefaultItemSpacing();
		fLayout = new BALMLayout(spacing, spacing);
		SetLayout(fLayout);

		BRect innerFrame = InnerFrame();
		BRect frame = Frame();
		fLayout->SetInsets(innerFrame.left - frame.left + spacing,
			2 * TopBorderOffset() + spacing,
			frame.right - innerFrame.right + spacing,
			frame.bottom - innerFrame.bottom + spacing);

		fLayout->AddView(fStrutConfigView, fLayout->Left(),
			fLayout->Top(), fLayout->Right(), fLayout->Bottom());
		_LoadValues();
	}

	void MessageReceived(BMessage* message)
	{
		switch (message->what) {
		case kMsgStrutMenu:
		{
			BSize strut = fItem->GetStrut();
			DirectionPolicy directionPolicy;
			if (directionPolicy.Get(strut) < 0)
				directionPolicy.Set(strut, 30);
			fItem->MakeStrut(strut);
			_LoadValues();
			InvalidateLayout();
			break;
		}

		case kMsgSpringMenu:
		{
			BSize spring = fItem->GetSpring();
			DirectionPolicy directionPolicy;
			if (directionPolicy.Get(spring) == B_SIZE_UNSET)
				directionPolicy.Set(spring, -1);
			fItem->MakeSpring(spring);
			_LoadValues();
			InvalidateLayout();
			break;
		}

		case kMsgNeutralMenu:
			fItem->MakeNeutral();
			_LoadValues();
			InvalidateLayout();
			break;

		default:
			BView::MessageReceived(message);			
		}
	}

private:
	void _LoadValues()
	{
		DirectionPolicy directionPolicy;
		BSize size = fItem->PreferredSize();
		float preferredSize = directionPolicy.Get(size);
		if (preferredSize != B_SIZE_UNSET) {
			// spring
			RemoveChild(fValueConfigView);
			fSpringMenuItem->SetMarked(true);
			fValueConfigView = fSpringConfigView;
			fSpringConfigView->SetEnabled(true);
			fStrutConfigView->SetEnabled(false);
			fLayout->AddView(fValueConfigView, fLayout->Left(),
				fLayout->Top(), fLayout->Right(), fLayout->Bottom());
		} else {
			size = fItem->MinSize();
			float minSize = directionPolicy.Get(size);
			size = fItem->MaxSize();
			float maxSize = directionPolicy.Get(size);
			if (minSize != maxSize) {
				// neutral
				fNeutralMenuItem->SetMarked(true);
				fValueConfigView = NULL;
				fSpringConfigView->SetEnabled(false);
				fStrutConfigView->SetEnabled(false);
			} else {
				// strut
				RemoveChild(fValueConfigView);
				fStrutMenuItem->SetMarked(true);
				fValueConfigView = fStrutConfigView;
				fSpringConfigView->SetEnabled(false);
				fStrutConfigView->SetEnabled(true);
				fLayout->AddView(fValueConfigView, fLayout->Left(),
					fLayout->Top(), fLayout->Right(), fLayout->Bottom());
			}
		}
		if (fEditView->LockLooper()) {
			fEditView->Invalidate();
			fEditView->UnlockLooper();
		}
	}

private:
	enum {
		kMsgStrutMenu = '&stm',
		kMsgSpringMenu,
		kMsgNeutralMenu
	};

			Spacer*				fItem;
			LayoutEditView*		fEditView;
			BString				fLabel;
			BALMLayout*			fLayout;

			BMenuItem*			fStrutMenuItem;
			BMenuItem*			fSpringMenuItem;
			BMenuItem*			fNeutralMenuItem;
			BView*				fValueConfigView;

			SizeValueConfigView<StrutSizePolicy, DirectionPolicy>*
				fStrutConfigView;
			SizeValueConfigView<SpringSizePolicy, DirectionPolicy>*
				fSpringConfigView;
};


class StrutSpringConfigView : public BView {
public:
	StrutSpringConfigView(Spacer* item, LayoutEditView* editView)
		:
		BView("LayoutEditView", 0),
		fItem(item),
		fEditView(editView)
	{
	}

	void AttachedToWindow()
	{
		SetViewColor(ui_color(B_PANEL_BACKGROUND_COLOR));

		float spacing = be_control_look->DefaultItemSpacing();
		BALMLayout* layout = new BALMLayout(spacing, spacing);
		SetLayout(layout);

		StrutSpringSwitcherBox<HMinMaxSizePolicy, PrefSizePolicy,
			WidthDirectionPolicy>* widthBox
			= new StrutSpringSwitcherBox<HMinMaxSizePolicy, PrefSizePolicy,
				WidthDirectionPolicy>(fItem, fEditView, "Width:");
		StrutSpringSwitcherBox<VMinMaxSizePolicy, PrefSizePolicy,
			HeightDirectionPolicy>* heightBox
			= new StrutSpringSwitcherBox<VMinMaxSizePolicy, PrefSizePolicy,
				HeightDirectionPolicy>(fItem, fEditView, "Heigth:");

		BALM::BALMLayoutBuilder(layout)
			.Add(widthBox, layout->Left(), layout->Top(), layout->Right())
			.AddBelow(heightBox);
	}

private:
			Spacer*				fItem;
			LayoutEditView*		fEditView;
};


class CustomizableConfigView : public BView {
public:
	CustomizableConfigView(IViewContainer* customizable,
		LayoutEditView* editView)
		:
		BView("CustomizableConfigView", 0),
		fCustomizable(customizable),
		fEditView(editView)
	{
	}

	void AttachedToWindow()
	{
		SetViewColor(ui_color(B_PANEL_BACKGROUND_COLOR));

		float spacing = be_control_look->DefaultItemSpacing();
		BALMLayout* layout = new BALMLayout(spacing, spacing);
		SetLayout(layout);

		fTextControl = new BTextControl("identifier", "Identifier:",
			fCustomizable->Identifier(), NULL);
		fTextControl->SetModificationMessage(new BMessage(
			kMsgIdentifierChanged));
		fTextControl->SetTarget(this);
		layout->AddView(fTextControl, layout->Left(), layout->Top(),
			layout->Right(), layout->Bottom());
	}

	void MessageReceived(BMessage* message)
	{
		switch (message->what) {
		case kMsgIdentifierChanged:
		{
			BAutolock _(fEditView->Window());
			fCustomizable->SetIdentifier(fTextControl->Text());
			break;
		}

		default:		
			BView::MessageReceived(message);
		}
	}

private:
	enum {
		kMsgIdentifierChanged = '&ICh'
	};

			IViewContainer*		fCustomizable;
			LayoutEditView*		fEditView;

			BTextControl*		fTextControl;
};


class AreaConfigView : public BView {
public:
	AreaConfigView(Area* area, LayoutEditView* editView)
		:
		BView("AreaConfigView", 0),
		fArea(area),
		fEditView(editView)
	{
	}

	void AttachedToWindow()
	{
		SetViewColor(ui_color(B_PANEL_BACKGROUND_COLOR));

		float spacing = be_control_look->DefaultItemSpacing();
		BALMLayout* layout = new BALMLayout(spacing, spacing);
		layout->SetInsets(spacing, spacing);
		SetLayout(layout);
		YTab* top = layout->Top();

		IViewContainer* customizable = _ToViewContainer(fArea);
		if (customizable) {
			CustomizableConfigView* customizableConfigView
				= new CustomizableConfigView(customizable, fEditView);
			Area* area = layout->AddView(customizableConfigView, layout->Left(), 
				top, layout->Right());
			top = area->Bottom();
		}

		BLayoutItem* layoutItem = fArea->Item();
		if (layoutItem != NULL) {
			BView* layoutItemView = NULL;
			Spacer* spacer = dynamic_cast<Spacer*>(layoutItem);
			if (spacer != NULL)
				layoutItemView = new StrutSpringConfigView(spacer, fEditView);
			else {
				layoutItemView = new LayoutItemConfigView(layoutItem,
					fEditView);
			}
			layout->AddView(layoutItemView, layout->Left(), top,
				layout->Right(), layout->Bottom());
		}
	}

private:
	IViewContainer* _ToViewContainer(Area* area)
	{
		BLayoutItem* layoutItem = fArea->Item();
		IViewContainer* customizable = dynamic_cast<IViewContainer*>(layoutItem);
		if (customizable == NULL)
			customizable = dynamic_cast<IViewContainer*>(layoutItem->View());
		return customizable;
	}

			Area*				fArea;
			LayoutEditView*		fEditView;
};


EditWindow::EditWindow(BALMEditor* editor, LayoutEditView* editView) 
	:
	BWindow(BRect(25, 50, 450, 550), "Properties", B_TITLED_WINDOW, 0),

	fEditor(editor),
	fEditView(editView),
	fALMEngine(editor->Layout())
{	
	_InitializeComponent();

	BWindow* parent = fEditView->Window();
	if (parent) {
		BPoint position = parent->Frame().RightTop();
		position.x += 10;
		if (BScreen(this).Frame().Contains(position) == false)
			position.x -= Frame().Width() + parent->Frame().Width() + 20;
		if (BScreen(this).Frame().Contains(position) == true)
			MoveTo(position);
	}

	// set size limits
	BSize min = GetLayout()->MinSize();
	BSize max = GetLayout()->MaxSize();
	SetSizeLimits(min.Width(), max.Width(), min.Height(), max.Height());
}


EditWindow::~EditWindow()
{
	delete fOpenPanel;
	delete fSavePanel;
}


void
EditWindow::_InitializeComponent()
{	
	// Initialize all controls
	BMessenger target(this);
	fSavePanel = new BFilePanel(B_SAVE_PANEL, &target, NULL, 
		B_FILE_NODE, false, new BMessage(kMsgSaveLayout));
	fOpenPanel = new BFilePanel(B_OPEN_PANEL, &target, NULL, 
		B_FILE_NODE, false, new BMessage(kMsgLoadLayout));

	fMainMenu = new BMenuBar("MainMenu");
	
	BMenu* fileMenu = new BMenu("File");
	fileMenu->AddItem(new BMenuItem("Clear", new BMessage(kMsgClearLayout)));
	fileMenu->AddSeparatorItem();
	fileMenu->AddItem(new BMenuItem("Load", new BMessage(kMsgLoadDialog)));
	fileMenu->AddItem(new BMenuItem("Save", new BMessage(kMsgSaveDialog)));
	fileMenu->AddSeparatorItem();
	fileMenu->AddItem(new BMenuItem("Decrease Preview",
		new BMessage(kMsgDecreaseWindowPreview)));
	fileMenu->AddItem(new BMenuItem("Enlarge Preview",
		new BMessage(kMsgEnlargeWindowPreview)));
	fileMenu->AddSeparatorItem();
	fileMenu->AddItem(new BMenuItem("Exit", new BMessage(B_QUIT_REQUESTED)));
	
	fMainMenu->AddItem(fileMenu);
	fMainMenu->SetExplicitAlignment(BAlignment(B_ALIGN_LEFT,
		B_ALIGN_USE_FULL_HEIGHT));

	fShowXTabBox = new BCheckBox("ShowXTabBox", "Show X Tabs",
		new BMessage(kMsgShowXTabBox));
	fShowYTabBox = new BCheckBox("ShowYTabBox", "Show Y Tabs",
		new BMessage(kMsgShowYTabBox));
	fFreePlacementBox = new BCheckBox("FreePlacementBox", "Free Placement",
		new BMessage(kMsgFreePlacementBox));

	fShowXTabBox->SetValue(fEditor->ShowXTabs());
	fShowYTabBox->SetValue(fEditor->ShowYTabs());
	fFreePlacementBox->SetValue(fEditor->FreePlacement());
	
	// Tabs
	fEditWindowTabContent = new BTabView("EditWindowTabContent",
		B_WIDTH_FROM_LABEL);
	fEditWindowTabContent->SetExplicitPreferredSize(BSize(B_SIZE_UNSET, 150));
	fEditWindowTabContent->SetViewColor(ui_color(B_PANEL_BACKGROUND_COLOR));

	// Area tab
	fAreaPage = new BTab();
	fAreaView = new BView("", 0);
	fAreaView->SetLayout(new BALMLayout);
	fEditWindowTabContent->AddTab(fAreaView, fAreaPage);
	fEditWindowTabContent->SetExplicitMaxSize(BSize(B_SIZE_UNLIMITED,
		B_SIZE_UNLIMITED));
	fAreaPage->SetLabel("Layout Item");
	_SetTabAreaContent(_CreateNoItemSelectedView());

	// Trash tab
	BTab* trashTab = new BTab();
	fEditWindowTabContent->AddTab(new TrashView(fEditor), trashTab);
	if (fEditor->IsCreationMode())
		trashTab->SetLabel("Trash");
	else
		trashTab->SetLabel("Components");

	if (!fEditor->IsCreationMode())
		fEditWindowTabContent->Select(1);

	// Factory View
	fFactoryDragListView = new FactoryDragListView(
		CustomizableRoster::DefaultRoster());
	BScrollView* factoryScrollView = new BScrollView("ScrollView",
		fFactoryDragListView, B_WILL_DRAW | B_FRAME_EVENTS, true, true,
		B_FANCY_BORDER);
	factoryScrollView->SetExplicitPreferredSize(BSize(-1, -1));

	BALMLayout* menuLayout = new BALMLayout();
	SetLayout(menuLayout);

	BView* background = new BView("background", B_WILL_DRAW);
	background->SetViewColor(ui_color(B_PANEL_BACKGROUND_COLOR));
	BALMLayoutBuilder(menuLayout)
		.Add(fMainMenu, menuLayout->Left(), menuLayout->Top(),
			menuLayout->Right())
		.StartingAt(fMainMenu)
			.AddBelow(background, menuLayout->Bottom());

	BStringView* factoryLabel = new BStringView("Factory",
		"Component Factory:");
	factoryLabel->SetExplicitAlignment(
		BAlignment(B_ALIGN_LEFT, B_ALIGN_USE_FULL_HEIGHT));

	float spacing = be_control_look->DefaultItemSpacing() / 2;
	BALMLayout* layout = new BALMLayout(spacing, spacing);
	layout->SetInsets(spacing, spacing);
	if (fEditor->IsCreationMode()) {
		BALMLayoutBuilder(background, layout)
			.Add(factoryLabel, layout->Left(), layout->Top())
			.AddBelow(factoryScrollView, NULL, layout->Left(), layout->Right())
			.AddBelow(fEditWindowTabContent)
			.AddBelow(fShowXTabBox, layout->Bottom(), NULL, layout->AddXTab())
			.AddToRight(fShowYTabBox)
			.AddToRight(fFreePlacementBox, layout->Right());
	} else {
		BALMLayoutBuilder(background, layout)
			.Add(fEditWindowTabContent, layout->Left(), layout->Top(),
				layout->Right())
			.AddBelow(fShowXTabBox, layout->Bottom(), NULL, layout->AddXTab())
			.AddToRight(fShowYTabBox)
			.AddToRight(fFreePlacementBox, layout->Right());
	}
}


BView*
EditWindow::_CreateAreaPropertyView(Area* area)
{
	if (area == NULL)
		return _CreateNoItemSelectedView();

	return new AreaConfigView(area, fEditView);
}


BView*
EditWindow::_CreateNoItemSelectedView()
{
	BView* view = new BView("", 0);
	view->SetViewColor(ui_color(B_PANEL_BACKGROUND_COLOR));
	BALMLayout* layout = new BALMLayout();
	view->SetLayout(layout);
	layout->AddView(new BStringView("", "No item selected."), layout->Left(),
		layout->Top(), layout->Right(), layout->Bottom());
	return view;
}


void
EditWindow::_SetTabAreaContent(BView* view)
{
	BALMLayout* layout = (BALMLayout*)fAreaView->GetLayout();
	BLayoutItem* item = layout->ItemAt(0);
	if (item != NULL) {
		layout->RemoveItem(item);
		delete item;
	}
	if (view != NULL) {
		layout->AddView(view, layout->Left(), layout->Top(), layout->Right(),
			layout->Bottom());
	}
}


void
EditWindow::MessageReceived(BMessage* message)
{
	switch (message->what) {
		case kMsgShowXTabBox:
			fEditor->SetShowXTabs(fShowXTabBox->Value() == B_CONTROL_ON);
			break;

		case kMsgShowYTabBox:
			fEditor->SetShowYTabs(fShowYTabBox->Value() == B_CONTROL_ON);
			break;

		case kMsgFreePlacementBox:
			fEditor->SetFreePlacement(
				fFreePlacementBox->Value() == B_CONTROL_ON);
			break;

		case kMsgLoadDialog:
			fOpenPanel->Show();
			break;

		case kMsgSaveDialog:
			fSavePanel->Show();
			break;

		case kMsgDecreaseWindowPreview:
		{
			BWindow* window = fALMEngine->Owner()->Window();
			window->LockLooper();
			BSize size = fALMEngine->MinSize();
			window->ResizeTo(size.width, size.height);
			window->UnlockLooper();
			break;	
		}

		case kMsgEnlargeWindowPreview:
		{
			BWindow* window = fALMEngine->Owner()->Window();
			window->LockLooper();
			BSize size = fALMEngine->PreferredSize();
			window->ResizeTo(size.width, size.height);
			window->UnlockLooper();
			break;	
		}

		case kMsgClearLayout:
		{
			BWindow* parent = fEditView->Window();
			if (parent != NULL && parent->Lock()) {
				fEditor->ClearLayout();

				UpdateEditWindow();
				parent->Unlock();
			}
		}

		case kMsgLoadLayout:
		{
			entry_ref ref;
			message->FindRef("refs", &ref);
			BEntry entry(&ref, true);

			BFile file(&entry, B_READ_ONLY);

			if (fEditView->LockLooper()) {
				fEditor->RestoreFromAttribute(&file, "layout");

				UpdateEditWindow();
				fEditView->UnlockLooper();
			}
			break;
		}

		case kMsgSaveLayout:
		{
			entry_ref dirRef;
			message->FindRef("directory", &dirRef);
			BDirectory dir(&dirRef);

			const char* name;
			message->FindString("name", &name);

			BEntry entry(&dir, name);
			BFile file(&entry, B_READ_WRITE | B_CREATE_FILE);

			if (fEditView->LockLooper()) {
				BMessage archive;
				LayoutArchive archiver(fALMEngine);
				archiver.SaveLayout(&archive, true);
				archiver.SaveToAttribute(&file, "layout", &archive);
				fEditView->InvalidateLayout();
				fEditView->Invalidate();

				UpdateEditWindow();
				fEditView->UnlockLooper();
			}
			break;
		}
	}
}


bool
EditWindow::QuitRequested()
{
	fEditor->StopEdit();
	return true;
}


void
EditWindow::UpdateEditWindow()
{
	BAutolock _(this);

	_SetTabAreaContent(_CreateAreaPropertyView(fEditView->SelectedArea()));

	// set size limits
	BSize min = GetLayout()->MinSize();
	BSize max = GetLayout()->MaxSize();
	SetSizeLimits(min.Width(), max.Width(), min.Height(), max.Height());
}
