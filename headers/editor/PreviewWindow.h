/*
 * Copyright 2012, Clemens Zeidler <haiku@clemens-zeidler.de>
 * Distributed under the terms of the MIT License.
 */
#ifndef	PREVIEW_WINDOW_H
#define	PREVIEW_WINDOW_H

#include <map>

#include <Window.h>

#include <ArrayContainer.h>
#include <ALMLayout.h>
#include <CustomizableView.h>


namespace BALM {


class ScaleLayout {
public:
	static void					Scale(BWindow* window,
									BALMLayout* layout, float scaleFactor);		
};


class PreviewWindowManager;


class PreviewWindow : public BWindow {
public:
	enum prev_size {
		kMinSize,
		kCustomSize,
		kEnlargedSize
	};

								PreviewWindow(BALMLayout* layout,
									prev_size size,
									PreviewWindowManager* manager);
								~PreviewWindow();

			bool				QuitRequested();

			void				MessageReceived(BMessage* message);
private:
			void				SetContainerLayout(BLayout* layout);

			void				_SetSize(prev_size size);
			void				_AdjustSize();
			void				_AdjustEnlargedSize();

			void				_InitLayout();

			BALMLayout*			fLayout;
			BALMLayout*			fOwnLayout;
			prev_size			fPrevSize;
			PreviewWindowManager*	fManager;
			BArray<BReference<CustomizableView> >	fItems;

			BView*				fContainerView;
};


class PreviewWindowManager {
public:
			void				NotifyLayoutEdited();
	
			PreviewWindow*		AddPreviewWindow(BALMLayout* layout,
									PreviewWindow::prev_size size);
			void				RemovePreviewWindow(BWindow* window);
private:
	friend class PreviewWindow;
			void				_AddPreviewWindow(BWindow* window);

			BObjectList<BWindow>	fWindowList;
			BLocker				fListLocker;
};


}	// namespace BALM


#endif	// PREVIEW_WINDOW_H
