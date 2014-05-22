/*
 * Copyrights (c):
 *     2002-2008, Werner Freytag.
 *     2009-2014 Haiku, Inc. All rights reserved
 * Distributed under the terms of the MIT License.
 *
 * Original Author:
 *		Werner Freytag <freytag@gmx.de>
 * Updates by:
 *		John Scipione, jscipione@gmail.com
 */


#include "DuckSaver.h"

#include <Bitmap.h>
#include <Entry.h>
#include <Handler.h>
#include <InterfaceDefs.h>
#include <LayoutBuilder.h>
#include <Mime.h>
#include <Resources.h>
#include <Screen.h>
#include <Slider.h>
#include <StringView.h>

#include <ctime>
#include <cstdlib>

#ifdef DEBUG
#include <iostream>
#include <string.h>
#endif


static const char* kAppSignature = "application/x-vnd.pecora-ducksaver";

static const int kDefaultDropRate = 30;
static const int kMaxTickSize = 10000000;

static const unsigned int kMsgUpdateTickSize = 'tick';

extern "C" _EXPORT BScreenSaver* instantiate_screen_saver(BMessage* message,
	image_id image)
{
	return new DuckSaver(message, image);
}


DuckSaver::DuckSaver(BMessage* archive, image_id image)
	:
	BScreenSaver(archive, image),
	fInitOk(false),
	fDropRate(kDefaultDropRate)
{
	srand(time(NULL));

	for (unsigned int i = 0; i < kResourceCount; ++i)
		fImage[i] = 0;

	BMimeType mime(kAppSignature);
	entry_ref app_ref;
	BResources resources;

	if (resources.SetToImage((const void*)&instantiate_screen_saver) != B_OK) {
#ifdef DEBUG
		cerr << "Unable to open resource file." << endl;
#endif
		return;
	} else {
		size_t size;
		BMessage message;
		char* buffer;

		for (unsigned int i = 0; i < kResourceCount; ++i) {
			buffer = (char *)resources.LoadResource('BBMP', i + 1, &size);
			if (buffer == NULL) {
				fImage[i] = 0;
#ifdef DEBUG
				cerr << "Resource not found: " << i + 1 << endl;
#endif
			} else {
				message.Unflatten(buffer);
				fImage[i] = new BBitmap(&message);
			}
		}
	}

	fInitOk = true;

	if (archive != NULL) {
		int32 dropRate;
		if (archive->FindInt32("drop rate", &dropRate) == B_OK)
			fDropRate = dropRate;
	}
}


DuckSaver::~DuckSaver()
{
	for (unsigned int i = 0; i < kResourceCount; ++i)
		delete fImage[i];
}


void
DuckSaver::Draw(BView* view, int32 frame)
{
	if (frame == 0)
		view->SetDrawingMode(B_OP_ALPHA);

	SetTickSize(kMaxTickSize / fDropRate);

	BBitmap* bitmap = fImage[rand() > (RAND_MAX >> 1) ? 1 : 0];
	if (bitmap != NULL) {
		BPoint point((rand() % view->Bounds().IntegerWidth())
				- (bitmap->Bounds().IntegerWidth() >> 1),
			(rand() % view->Bounds().IntegerHeight())
				- (bitmap->Bounds().IntegerHeight() >> 1));
		view->DrawBitmap(bitmap, point);
	}
}


void
DuckSaver::MessageReceived(BMessage* message)
{
	switch(message->what) {
		case kMsgUpdateTickSize:
		{
			int32 dropRate = 0;
			if (message->FindInt32("be:value", &dropRate) == B_OK)
				fDropRate = dropRate;
			break;
		}

		default:
			BHandler::MessageReceived(message);
	}
}


status_t
DuckSaver::SaveState(BMessage* into) const
{
	return into->AddInt32("drop rate", fDropRate);
}


void
DuckSaver::StartConfig(BView* view)
{
	BView* topView = NULL;
	BStringView* title = NULL;
	BSlider* dropRateSlider = new BSlider(B_EMPTY_STRING, "Drop rate:",
		new BMessage(kMsgUpdateTickSize), 2, 50, B_HORIZONTAL);
	dropRateSlider->SetValue(fDropRate);
	dropRateSlider->SetLimitLabels("Slow", "Fast");
	BWindow* window = view->Window();
	if (window != NULL) {
		window->AddHandler(this);
		dropRateSlider->SetTarget(this);
	}

	if (!fInitOk) {
		title = new BStringView(B_EMPTY_STRING,
			"Error: Can't find resources!");
		title->SetHighColor(ui_color(B_FAILURE_COLOR));
		topView = BLayoutBuilder::Group<>(B_VERTICAL, 0)
			.Add(title)
			.SetInsets(B_USE_DEFAULT_SPACING)
			.View();
	} else {
		title = new BStringView(B_EMPTY_STRING, "DuckSaver");
		title->SetFont(be_bold_font);
		topView = BLayoutBuilder::Group<>(B_VERTICAL, 0)
			.Add(title)
			.Add(new BStringView(B_EMPTY_STRING, "©2002 by Werner Freytag"))
			.AddGlue()
			.Add(dropRateSlider)
			.SetInsets(B_USE_DEFAULT_SPACING)
			.View();
	}

	topView->ResizeTo(view->Bounds().Width(), view->Bounds().Height());
	view->AddChild(topView);
}


status_t
DuckSaver::StartSaver(BView* view, bool preview)
{	
	return fInitOk ? B_OK : B_ERROR;
}
