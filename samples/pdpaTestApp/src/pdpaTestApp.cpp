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

#include "portaudio.h"
#include "PdBase.hpp"

using namespace ci;
using namespace ci::app;
using namespace std;

class PdPaTestApp : public AppBasic
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

		static int audioCallback( const void *inputBuffer, void *outputBuffer, unsigned long framesPerBuffer,
				const PaStreamCallbackTimeInfo* timeInfo, PaStreamCallbackFlags statusFlags,
				void *userData );

		float mFreq;

		static const int sSampleRate = 44100;
		static const int sNumInputs = 0;
		static const int sNumOutputs = 2;

		PaStream *mStream;
		int checkPaError( const PaError &err );
};

void PdPaTestApp::setup()
{
	PaError err = Pa_Initialize();

	PaStreamParameters outputParameters;
	outputParameters.device = Pa_GetDefaultOutputDevice();
	if ( outputParameters.device == paNoDevice )
	{
		app::console() << "Error: No default output device." << endl;
		quit();
	}
	outputParameters.channelCount = sNumOutputs;
	outputParameters.sampleFormat = paFloat32;
	outputParameters.suggestedLatency = Pa_GetDeviceInfo( outputParameters.device )->defaultLowOutputLatency;
	outputParameters.hostApiSpecificStreamInfo = NULL;

	err = Pa_OpenStream(
			&mStream,
			NULL,
			&outputParameters,
			(double)sSampleRate,
			512,
			paNoFlag,
			&PdPaTestApp::audioCallback,
			&mPd );
	if ( checkPaError( err ) )
		quit();

	err = Pa_StartStream( mStream );
	if ( checkPaError( err ) )
		quit();

	if ( !mPd.init( sNumInputs, sNumOutputs, sSampleRate ) )
	{
		app::console() << "Could not init pd" << endl;
		quit();
	}
	mPd.computeAudio( true );

	// open patch
	fs::path pdPath = getAssetPath( "test.pd" );
	mPdPatch = mPd.openPatch( pdPath.filename().string(), pdPath.parent_path().string() );

	app::console() << mPdPatch << endl;

	mParams = params::InterfaceGl( "Parameters", Vec2i( 200, 300 ) );
	mParams.addButton( "bang", [&]() { mPd.sendBang( "test" ); } );
	mFreq = 220.f;
	mParams.addParam( "freq", &mFreq, "min=0 max=880 step=1" );
}

int PdPaTestApp::audioCallback( const void *inputBuffer, void *outputBuffer,
		unsigned long framesPerBuffer,
		const PaStreamCallbackTimeInfo* timeInfo,
		PaStreamCallbackFlags statusFlags,
		void *userData )
{
	pd::PdBase *pd = reinterpret_cast< pd::PdBase * >( userData );
	int ticks = framesPerBuffer / pd::PdBase::blockSize();
	pd->processFloat( ticks, (float *)inputBuffer, (float *)outputBuffer );
	return paContinue;
}

void PdPaTestApp::shutdown()
{
	// close portaudio
	PaError err = Pa_AbortStream( mStream );
	checkPaError( err );

	// sleep until playback has finished
	while ( ( err = Pa_IsStreamActive( mStream ) ) == 1 )
		Pa_Sleep( 1000 );
	checkPaError( err );

	err = Pa_CloseStream( mStream );
	checkPaError( err );
	Pa_Terminate();

	// pd
	mPd.computeAudio( false );

	// close patch
	mPd.closePatch( mPdPatch );
}

void PdPaTestApp::update()
{
	mPd.sendFloat( "freq", mFreq );

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

void PdPaTestApp::draw()
{
	gl::clear( Color::black() );

	mParams.draw();
}

void PdPaTestApp::keyDown( KeyEvent event )
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

int PdPaTestApp::checkPaError( const PaError &err )
{
	if ( err != paNoError )
	{
		app::console() << "An error occured while using portaudio" << endl;
		if ( err == paUnanticipatedHostError )
		{
			app::console() << " unanticipated host error." << endl;
			const PaHostErrorInfo* herr = Pa_GetLastHostErrorInfo();
			if ( herr )
			{
				app::console() << " Error number: " << herr->errorCode << endl;
				if ( herr->errorText )
					app::console() << " Error text: " << herr->errorText << endl;
			}
			else
			{
				app::console() << " Pa_GetLastHostErrorInfo() failed!" << endl;
			}
		}
		else
		{
			app::console() << " Error number: " << err << endl;
			app::console() << " Error text: " << Pa_GetErrorText( err ) << endl;
		}

		return 1;
	}

	return 0;
}

CINDER_APP_BASIC( PdPaTestApp, RendererGl )

