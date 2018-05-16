# EmotionTagging
Speech recognition and rudimentary emotion detection.

## Installation
Requires cPortAudio. This can be downloaded through standard Linux package manager.
Otherwise, the openEAR-x.x.x directory contains an executable version of PortAudio
available in the `thirdparty` directory.
After extraction, PortAudio can be installed via `./configure && make` and then
`sudo make install`.

In the higher level directory, simply run `make` to create the exectuable for the
abstract program.


## Usage
The main program can be run traditionally i.e., `./main`