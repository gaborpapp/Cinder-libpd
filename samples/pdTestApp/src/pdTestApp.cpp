/*
 Copyright (C) 2013 Gabor Papp

 This program is free software; you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation; either version 3 of the License, or
 (at your option) any later version.

 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with this program. If not, see <http://www.gnu.org/licenses/>.
*/

#include "cinder/Cinder.h"
#include "cinder/app/AppBasic.h"
#include "cinder/gl/gl.h"
#include "cinder/params/Params.h"

#include "PdBase.hpp"

using namespace ci;
using namespace ci::app;
using namespace std;

class PdTestApp : public AppBasic
{
	public:
		void setup();
		void shutdown();

		void keyDown( KeyEvent event );

		void update();
		void draw();

	private:
		params::InterfaceGl mParams;

		pd::PdBase mPd;
		pd::Patch mPdPatch;
};

void PdTestApp::setup()
{
	mParams = params::InterfaceGl( "Parameters", Vec2i( 200, 300 ) );

	// one input channel, two output channels, 44100 sampling rate
	if ( !mPd.init( 1, 2, 44100 ) )
	{
		app::console() << "Could not init pd" << endl;
		quit();
	}
	mPd.computeAudio( true );

	// open patch
	fs::path pdPath = getAssetPath( "test.pd" );
	mPdPatch = mPd.openPatch( pdPath.filename().string(), pdPath.parent_path().string() );

	app::console() << mPdPatch << endl;

	mParams.addButton( "bang", [&]() { mPd.sendBang( "test" ); } );
}

void PdTestApp::shutdown()
{
	mPd.computeAudio( false );

	// close patch
	mPd.closePatch( mPdPatch );
}

void PdTestApp::update()
{
	// poll pd messages
	while ( mPd.numMessages() > 0)
	{
		pd::Message &msg = mPd.nextMessage();
		switch ( msg.type )
		{
			case pd::PRINT:
				app::console() << msg.symbol << endl;
				break;
			default:
				break;
		}
	}
}

void PdTestApp::draw()
{
	gl::clear( Color::black() );

	mParams.draw();
}

void PdTestApp::keyDown( KeyEvent event )
{
	switch ( event.getCode() )
	{
		case KeyEvent::KEY_ESCAPE:
			quit();
			break;

		default:
			break;
	}
}

CINDER_APP_BASIC( PdTestApp, RendererGl )

