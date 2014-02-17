/*
 * Copyrights (c):
 *     2002 - 2008 , Werner Freytag.
 *     2009, Haiku
 * Distributed under the terms of the MIT License.
 *
 * Original Author:
 *              Werner Freytag <freytag@gmx.de>
 */
#ifndef	_DUCK_SAVER_H
#define	_DUCK_SAVER_H


#include <ScreenSaver.h>


static const unsigned int kResourceCount = 2;


class DuckSaver : public BScreenSaver, public BHandler {
public:
							DuckSaver(BMessage* archive, image_id image);
							~DuckSaver();

	virtual	void			Draw(BView* view, int32 frame);
	virtual	void			MessageReceived(BMessage* message);
	virtual	status_t		SaveState(BMessage* into) const;
	virtual	void			StartConfig(BView* view);
	virtual	status_t		StartSaver(BView* view, bool preview);

private:
			bool			fInitOk;
			int				fDropRate;
			BBitmap*		fImage[kResourceCount];

};


#endif	_DUCK_SAVER_H

