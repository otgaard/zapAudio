#include <vector>
#include <fstream>
#include <iostream>
#include <numeric>
#include <portaudio.h>
#include "base/wave_stream.h"

const char* filename = "/Users/otgaard/test3.wav";
std::vector<short> buffer(1024);

typedef int callback(const void* input, void* output, u_long frame_count,
                     const PaStreamCallbackTimeInfo* time_info, PaStreamCallbackFlags status_flags,
                     void* userdata);

int wave_callback(const void* input, void* output, u_long frame_count, const PaStreamCallbackTimeInfo* time_info,
                  PaStreamCallbackFlags status_flags, void* userdata) {
    wave_stream* stream = static_cast<wave_stream*>(userdata);
    short* out = static_cast<short*>(output);

    auto len = stream->read(buffer, buffer.size());
    if(len == 0) { std::cerr << "Complete" << std::endl; return paComplete; }
    for(size_t i = 0; i != len; ++i) *out++ = buffer[i];
    return paContinue;
}

int main(int argc, char*argv[]) {
    wave_stream wstream(filename, 1024, nullptr);
    wstream.start();
    if(!wstream.is_open()) {
        std::cerr << "Error opening WAVE file." << std::endl;
        return -1;
    }

    if(Pa_Initialize() != paNoError) {
        std::cerr << "Error initialising portaudio" << std::endl;
        return -1;
    }

    PaStreamParameters outputParameters;
    outputParameters.device = Pa_GetDefaultOutputDevice();
    if(outputParameters.device == paNoDevice) {
        std::cerr << "No suitable output device found by portaudio" << std::endl;
        return -1;
    }

    PaStream* stream;

    PaError err = Pa_OpenDefaultStream(
            &stream,
            0,
            wstream.get_header().num_channels,
            paInt16,
            wstream.get_header().sample_rate,
            512,
            &wave_callback,
            &wstream
    );

    if(err != paNoError) {
        std::cerr << "Pa_OpenStream failed:" << err << Pa_GetErrorText(err) << std::endl;
        if(stream) Pa_CloseStream(stream);
        return -1;
    }

    if(Pa_StartStream(stream) != paNoError) {
        std::cerr << "Pa_OpenStream failed:" << err << Pa_GetErrorText(err) << std::endl;
        if(stream) Pa_CloseStream(stream);
        return -1;
    }

    while(Pa_IsStreamActive(stream) > 0) Pa_Sleep(1000);

    if(Pa_Terminate() != paNoError) {
        std::cerr << "Pa_Terminate error:" << err << Pa_GetErrorText(err) << std::endl;
        return -1;
    }

    return 0;
}