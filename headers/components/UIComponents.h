/*
 * Copyright 2012, Haiku, Inc. All rights reserved.
 * Distributed under the terms of the MIT License.
 */
#ifndef _UI_COMPONENTS_H
#define _UI_COMPONENTS_H


#include <Button.h>
#include <CheckBox.h>
#include <ListItem.h>
#include <ListView.h>
#include <MenuField.h>
#include <MenuItem.h>
#include <PopUpMenu.h>
#include <RadioButton.h>
#include <ScrollView.h>
#include <SpaceLayoutItem.h>
#include <StatusBar.h>
#include <StringView.h>
#include <TextControl.h>
#include <TextView.h>

#include <CustomizableView.h>


class Button : public BButton, virtual public CustomizableView {
public:
	Button(const char* label, BMessage* message = NULL)
		:
		CustomizableView(this),
		BButton(label, message)
	{
	}

	static Customizable*	InstantiateCustomizable(const BMessage* archive);

};


class ScrollView : public BScrollView, virtual public CustomizableView {
public:
	ScrollView(BView* target, bool horizontal = false, bool vertical = false,
		border_style border = B_FANCY_BORDER)
		:
		CustomizableView(this),
		BScrollView("ScrollView", target, B_WILL_DRAW | B_FRAME_EVENTS,
			horizontal, vertical, border)
	{
		Customizable* customizable = dynamic_cast<Customizable*>(target);
		if (customizable != NULL) {
			BString name = customizable->ObjectName();
			Customizable::SetObjectName(name);
		}
	}
};


class TextControl : public BTextControl, virtual public CustomizableView {
public:
	TextControl()
		:
		CustomizableView(this),
		BTextControl("BTextControl", NULL, "Text Control", NULL)
	{
	}

	static Customizable*	InstantiateCustomizable(const BMessage* archive);

};


class TextView : public BTextView, virtual public CustomizableView {
public:
	TextView()
		:
		CustomizableView(this),
		BTextView("BTextView")
	{
	}

	static Customizable*	InstantiateCustomizable(const BMessage* archive);

};


class RadioButton : public BRadioButton, virtual public CustomizableView {
public:
	RadioButton(const char* label, BMessage* message = NULL)
		:
		CustomizableView(this),
		BRadioButton("RadioButton", label, message)
	{
	}

	static Customizable*	InstantiateCustomizable(const BMessage* archive);

};


class CheckBox : public BCheckBox, virtual public CustomizableView {
public:
	CheckBox(const char* label, BMessage* message = NULL)
		:
		CustomizableView(this),
		BCheckBox("CheckBox", label, message)
	{
	}

	static Customizable*	InstantiateCustomizable(const BMessage* archive);

};


class ComboBox : public BMenuField, virtual public CustomizableView {
public:
	ComboBox(const char* label, BMenu* menu)
		:
		CustomizableView(this),
		BMenuField("Combo Box", label, menu)
	{
	}

	static Customizable*	InstantiateCustomizable(const BMessage* archive);

};


class ProgressBar : public BStatusBar, virtual public CustomizableView {
public:
	ProgressBar(const char* label, const char* trailingLabel)
		:
		CustomizableView(this),
		BStatusBar("ProgressBar", label, trailingLabel)
	{
		Update(60);
	}

	static Customizable*	InstantiateCustomizable(const BMessage* archive);

};


class StringView : public BStringView, virtual public CustomizableView {
public:
	StringView(const char* label)
		:
		CustomizableView(this),
		BStringView("StringView", label)
	{
	}

	static Customizable*	InstantiateCustomizable(const BMessage* archive);

};


class ListView : public BListView, virtual public CustomizableView {
public:
	ListView()
		:
		CustomizableView(this),
		BListView("ListView")
	{
	}

	static Customizable*	InstantiateCustomizable(const BMessage* archive);

};


class Spacer : public BSpaceLayoutItem, virtual public CustomizableView {
public:
	Spacer()
		:
		CustomizableView(this),
		BSpaceLayoutItem(BSize(-1, -1), BSize(-1, -1), BSize(10, 10),
			BAlignment(B_ALIGN_HORIZONTAL_CENTER, B_ALIGN_VERTICAL_CENTER)),

		fMinSize(-1, -1),
		fPreferredSize(B_SIZE_UNSET, B_SIZE_UNSET),
		fMaxSize(B_SIZE_UNSET, B_SIZE_UNSET),
		fAlignment(B_ALIGN_HORIZONTAL_CENTER, B_ALIGN_VERTICAL_CENTER),
		fVisible(true),
		fStrut(B_SIZE_UNSET, B_SIZE_UNSET),
		fSpring(B_SIZE_UNSET, B_SIZE_UNSET),
		fCurrentSize(NULL)
	{
	}

	virtual ~Spacer()
	{
	}

	BSize MinSize()
	{
		return fMinSize;
	}

	BSize PreferredSize()
	{
		return fPreferredSize;
	}

	BSize MaxSize()
	{
		return fMaxSize;
	}

	BAlignment Alignment()
	{
		return fAlignment;
	}

	void SetExplicitMinSize(BSize size)
	{
		if (fCurrentSize == &fStrut)
			*fCurrentSize = size;
		fMinSize = size;
		InvalidateLayout();
	}

	void SetExplicitPreferredSize(BSize size)
	{
		if (fCurrentSize == &fSpring)
			*fCurrentSize = size;
		fPreferredSize = size;
		InvalidateLayout();
	}

	void SetExplicitMaxSize(BSize size)
	{
		fMaxSize = size;
		InvalidateLayout();
	}

	void SetExplicitAlignment(BAlignment alignment)
	{
		fAlignment = alignment;
		InvalidateLayout();
	}

	bool IsVisible()
	{
		return fVisible;
	}

	void SetVisible(bool visible)
	{
		fVisible = visible;
	}

	BRect Frame()
	{
		return fFrame;
	}

	void SetFrame(BRect frame)
	{
		fFrame = frame;
	}

	void MakeNeutral()
	{
		fCurrentSize = NULL;

		SetExplicitMinSize(BSize(-1, -1));
		SetExplicitMaxSize(BSize(B_SIZE_UNLIMITED, B_SIZE_UNLIMITED));
		SetExplicitPreferredSize(BSize(B_SIZE_UNSET, B_SIZE_UNSET));

		SetExplicitAlignment(BAlignment(
			B_ALIGN_HORIZONTAL_CENTER, B_ALIGN_VERTICAL_CENTER));
	}

	void MakeSpring(BSize spring)
	{
		MakeNeutral();

		fCurrentSize = &fSpring;
		fSpring = spring;
		SetExplicitPreferredSize(fSpring);
	}

	BSize GetSpring()
	{
		return fSpring;
	}

	void MakeHorizontalSpring(float strength)
	{
		MakeSpring(BSize(strength, B_SIZE_UNSET));
	}

	void MakeVerticalSpring(float strength)
	{
		MakeSpring(BSize(B_SIZE_UNSET, strength));
	}

	void MakeStrut(BSize strut)
	{
		fCurrentSize = &fStrut;

		fStrut = strut;
		if (strut.width < 0)
			strut.width = -1;
		if (strut.height < 0)
			strut.height = -1;
		SetExplicitMinSize(fStrut);
		if (strut.width < 0)
			strut.width = B_SIZE_UNLIMITED;
		if (strut.height < 0)
			strut.height = B_SIZE_UNLIMITED;
		SetExplicitMaxSize(strut);
		SetExplicitPreferredSize(BSize(B_SIZE_UNSET, B_SIZE_UNSET));
		SetExplicitAlignment(BAlignment(B_ALIGN_USE_FULL_WIDTH,
			B_ALIGN_USE_FULL_HEIGHT));
	}

	BSize GetStrut()
	{
		return fStrut;
	}

	void MakeHorizontalStrut(float strength)
	{
		MakeStrut(BSize(strength, B_SIZE_UNSET));
	}

	void MakeVerticalStrut(float strength)
	{
		MakeStrut(BSize(B_SIZE_UNSET, strength));
	}

/*
	bool IsHorizontalSpring()
	{
		BSize minSize = MinSize();
		BSize maxSize = MaxSize();
		BSize prefSize = BSpaceLayoutItem::PreferredSize();
		if (minSize.width == -1 && maxSize.width == B_SIZE_UNLIMITED
			&& prefSize.width >= 0)
			return true;
		return false;
	}

	bool IsVerticalSpring()
	{
		BSize minSize = MinSize();
		BSize maxSize = MaxSize();
		BSize prefSize = BSpaceLayoutItem::PreferredSize();
		if (minSize.height == -1 && maxSize.height == B_SIZE_UNLIMITED
			&& prefSize.height >= 0)
			return true;
		return false;
	}

	bool IsHorizontalStrut()
	{
		BSize minSize = MinSize();
		BSize maxSize = MaxSize();
		BSize prefSize = BSpaceLayoutItem::PreferredSize();
		if (minSize.width >= 0 && minSize.width == maxSize.width
			&& prefSize.width == B_SIZE_UNSET)
			return true;
		return false;
	}

	bool IsVerticalStrut()
	{
		BSize minSize = MinSize();
		BSize maxSize = MaxSize();
		BSize prefSize = BSpaceLayoutItem::PreferredSize();
		if (minSize.height >= 0 && minSize.height == maxSize.height
			&& prefSize.height == B_SIZE_UNSET)
			return true;
		return false;
	}
*/
	static Customizable*	InstantiateCustomizable(const BMessage* archive);

private:
			BSize				fMinSize;
			BSize				fPreferredSize;
			BSize				fMaxSize;
			BAlignment			fAlignment;
			bool				fVisible;
			BRect				fFrame;

			BSize				fStrut;
			BSize				fSpring;
			BSize*				fCurrentSize;
};


class HorizontalStrut : public Spacer {
public:
	static Customizable*	InstantiateCustomizable(const BMessage* archive);
};


class VerticalStrut : public Spacer {
public:
	static Customizable*	InstantiateCustomizable(const BMessage* archive);
};


class HorizontalSpring : public Spacer {
public:
	static Customizable*	InstantiateCustomizable(const BMessage* archive);
};


class VerticalSpring : public Spacer {
public:
	static Customizable*	InstantiateCustomizable(const BMessage* archive);
};


#endif // _UI_COMPONENTS_H
