/*
 * Copyright 2012, Clemens Zeidler <haiku@clemens-zeidler.de>
 * Distributed under the terms of the MIT License.
 */
#ifndef	EDIT_ANIMATOIN_H
#define	EDIT_ANIMATOIN_H


#include <ArrayContainer.h>
#include <ALMLayout.h>
#include <Handler.h>
#include <View.h>


namespace BALM {


class EditAnimation : public BHandler {
public:
								EditAnimation(BALMLayout* layout, BView* view);

			void				CaptureStartpoint();
			bool				Animate();

			void				Cancel();

			void				MessageReceived(BMessage* message);
private:
	struct area_animation {
		area_animation(Area* area);

		Area*		area;
		BRect		startRect;
		BPoint		translation;
		BPoint		scale;		
	};

			bool				_FindAnimation(Area* area,
									area_animation** animation);

	static	int32				_TimerThread(void* cookie);

			void				_SetAreas(float progress);
			bool				_ValidateArea(Area* area);
			void				_LayoutArea(Area* area);
private:
			BALMLayout*			fLayout;
			BView*				fView;
			BArray<area_animation>	fAnimationList;

			bool				fStopped;

			bigtime_t			fDuration;
			int32				fFramesPerSec;
};


}	// namespace BALM


#endif	// EDIT_ANIMATOIN_H
