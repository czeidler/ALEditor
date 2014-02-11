/*
 * Copyright 2012, Clemens Zeidler <haiku@clemens-zeidler.de>
 * Distributed under the terms of the MIT License.
 */
#ifndef	TRASH_VIEW_H
#define	TRASH_VIEW_H


#include <ListView.h>

#include <ALMEditor.h>
#include <CustomizableRoster.h>


class TrashView : public BListView {
public:
								TrashView(BALMEditor* editor);

	virtual bool				InitiateDrag(BPoint point, int32 index,
									bool wasSelected);

	virtual void				AttachedToWindow();

	virtual void				MessageReceived(BMessage* message);

private:
			void				_LoadTrash();

			BALMEditor*			fEditor;
			BArray<BWeakReference<Customizable> > fTrashList;
};


#endif // CUSTOMIZABLE_NODE_FACTORY_H
