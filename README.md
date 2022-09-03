# pamiec - don't lose the best moments

Pamiec is a Linux tool that runs in the background and always record the desktop audio. It stores the last moments in memory (configurable time) and let you export the audio to save it.

## Install from source

This program depends on `libpulse` and `libpulse-simple`. Almost every distro should ship with these when pulseaudio is installed. If you are using pipewire, you can checkout [pipewire-pulse](https://docs.pipewire.org/page_man_pipewire_pulse_1.html) as a drop-in compatibility layer for pulseaudio.

* Build using `make pamiec`
* Install using `make install`

You can install the binary to a different location using `make DESTDIR=$HOME/.local/bin install`.

