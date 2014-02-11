/*
 * Copyright 2012, Clemens Zeidler <haiku@clemens-zeidler.de>
 * Distributed under the terms of the MIT License.
 */


#include "EditAnimation.h"

#include <Messenger.h>


using namespace BALM;


const int32 kMsgAnimate = '&Ani';


EditAnimation::area_animation::area_animation(Area* a)
	:
	area(a)
{
	startRect = area->Frame();
}


EditAnimation::EditAnimation(BALMLayout* layout, BView* view)
	:
	fLayout(layout),
	fView(view),
	fStopped(false)
{
}


void
EditAnimation::CaptureStartpoint()
{
	Cancel();

	fAnimationList.MakeEmpty();

	for (int32 i = 0; i < fLayout->CountAreas(); i++) {
		Area* area = fLayout->AreaAt(i);
		fAnimationList.AddItem(area_animation(area));
	}
}


bool
EditAnimation::Animate()
{
	if (fLayout->CountAreas() <= 1)
		return true;

	for (int32 i = 0; i < fLayout->CountAreas(); i++) {
		Area* area = fLayout->AreaAt(i);
		area_animation* animation;
		if (!_FindAnimation(area, &animation)) {
			// probably a new area just set it to its final position, this
			// speeds up the insertion operation
			_LayoutArea(area);
			continue;
		}
		
		// calculate animation
		BRect endRect = area->Frame();
		animation->translation.x = endRect.left - animation->startRect.left;
		animation->translation.y = endRect.top - animation->startRect.top;

		animation->scale.x = endRect.Width() - animation->startRect.Width();
		animation->scale.y = endRect.Height() - animation->startRect.Height();
	}

	// set areas to start point
	_SetAreas(0.0);

	fStopped = false;
	// start timer
	fDuration = 160000;
	fFramesPerSec = 25;

	fLayout->DisableLayoutInvalidation();

	thread_id id = spawn_thread(_TimerThread, "timer", B_DISPLAY_PRIORITY,
		(void*)this);
	resume_thread(id);
	
	return true;
}


void
EditAnimation::Cancel()
{
	fStopped = true;
}


void
EditAnimation::MessageReceived(BMessage* message)
{
	switch (message->what) {
	case kMsgAnimate:
	{
		float progress = message->FindFloat("progress");
		_SetAreas(progress);
		if (progress == 1) {
			fLayout->EnableLayoutInvalidation();
			fLayout->InvalidateLayout();
		}
		break;
	}

	default:
		BHandler::MessageReceived(message);
	}
}


bool
EditAnimation::_FindAnimation(Area* area, area_animation** animation)
{
	for (int32 i = 0; i < fAnimationList.CountItems(); i++) {
		area_animation& ani = fAnimationList.ItemAt(i);
		if (ani.area == area) {
			*animation = &ani;
			return true;
		}
	}
	*animation = NULL;
	return false;
}


int32
EditAnimation::_TimerThread(void* cookie)
{
	EditAnimation* that = (EditAnimation*)cookie;

	BMessenger messenger(that);
	bigtime_t snoozeTime = 1000000 / (that->fFramesPerSec - 1);
	bigtime_t elapsed = 0;
	bigtime_t endTime = that->fDuration;
	while (elapsed < endTime) {
		snooze(snoozeTime);
		elapsed += snoozeTime;

		if (that->fStopped)
			elapsed = endTime;

		BMessage message(kMsgAnimate);
		float progress = float(elapsed) / endTime;
		if (progress > 1)
			progress = 1;
		message.AddFloat("progress", progress);
		messenger.SendMessage(&message);
	}

	return 0;
}


void
EditAnimation::_SetAreas(float progress)
{
	for (int32 i = 0; i < fAnimationList.CountItems(); i++) {
		area_animation& ani = fAnimationList.ItemAt(i);
		Area* area = ani.area;
		// check if area is still there
		if (!_ValidateArea(area))
			continue;
		BRect targetRect = ani.startRect;
		targetRect.OffsetBy(ani.translation.x * progress,
			ani.translation.y * progress);
		targetRect.right += ani.scale.x * progress;
		targetRect.bottom += ani.scale.y * progress;

		area->Left()->SetValue(targetRect.left);
		area->Top()->SetValue(targetRect.top);
		area->Right()->SetValue(targetRect.right);
		area->Bottom()->SetValue(targetRect.bottom);

		_LayoutArea(area);
	}
	fView->Invalidate();
}


bool
EditAnimation::_ValidateArea(Area* area)
{
	for (int32 i = 0; i < fLayout->CountAreas(); i++) {
		Area* a = fLayout->AreaAt(i);
		if (area == a)
			return true;
	}
	return false;
}


void
EditAnimation::_LayoutArea(Area* area)
{
	BLayoutItem* item = area->Item();
	if (item->View() != NULL) {
		if (!item->IsVisible())
			item->AlignInFrame(BRect(0, 0, -1, -1));

		BRect targetRect = area->Frame();
		targetRect.left += area->LeftInset();
		targetRect.right -= area->RightInset();
		targetRect.top += area->TopInset();
		targetRect.bottom -= area->BottomInset();
		
		item->AlignInFrame(targetRect.OffsetBySelf(
			fLayout->LayoutArea().LeftTop()));
	}
}
