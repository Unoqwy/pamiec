# pamiec - don't lose the best moments

Pamiec is a Linux tool that runs in the background and always record the desktop audio. It stores the last moments in memory (configurable time) and let you export the audio to save it.

## Install from source

This program depends on `libpulse` and `libpulse-simple`. Almost every distro should ship with these when pulseaudio is installed. If you are using pipewire, you can checkout [pipewire-pulse](https://docs.pipewire.org/page_man_pipewire_pulse_1.html) as a drop-in compatibility layer for pulseaudio.

* Build using `make pamiec`
* Install using `sudo make install`

You can install the binary to a different location using `make DESTDIR=$HOME/.local/bin install`.

## Usage

`pamiec daemon <sink> [-n <name>] [-d <duration>]` - Starts a daemon that will record audio from PA's `sink`. It may optionally have a `name` (use this if you want to record several sinks at once, for example to record Desktop audio + your microphone). It will keep the last `duration` seconds of audio in memory at any time (defaults to 30)

`pamiec exec save <filename> [-D <daemon_name>]` - Saves the in-memory buffer to a wav file. The file path may be relative to the daemon directory or absolute. If you use named daemons, use the `-D` argument.

