# BeamProfiler (WIP)
A little program to monitor vehicle data in BeamNG. It's not much right now, but it currently displays:
- RPM
- Temperatures
  - Water
  - Engine Block
  - Engine Wall
  - Oil
  - Coolant
- Inputs
  - Throttle
  - Clutch
  - Brake
- Speed
  - Wheelspeed (m/s)
  - Airspeed (m/s)

The program also handles the game's bullettime (slow motion) so it matches.  
There's not many options but you can change the history (1 - 30 seconds) and you can change the FPS Limit, 15 - 240. I recommend keeping it at 30 if you want low GPU usage).

![Graphs](https://i.imgur.com/NQxKLzb.png)

## How to use
1. Download the latest release from [here](https://github.com/vulcan-dev/beamprofiler)  
2. Go into the archive and extract the files  
3. Copy "beamprofiler.zip" (from the archive, not the archive itself) to the BeamNG's mod directory  
4. Run BeamProfiler.exe

The port it uses is `4444`, the IP is localhost. You'll be able to change this in the future.

## Why did I make this?
I just enjoy watching the data move in realtime, it's nice.  
Yes, that's the only reason.

## Building
```
git clone https://github.com/vulcan-dev/beamprofiler.git --recurse-submodules
mkdir build && cdbuild
cmake .. && cmake --build .
```

### Dependencies:
- SDL2
  - Windows: (https://github.com/libsdl-org/SDL/releases/tag/release-2.28.1)
  - Linux: Use your wonderful package manager

## Notes:
I have not tested on Linux, it probably won't build but I'll look into doing that soon.  
I still have cleanup to do, but I figured I would just get it on Github before it corrupts again.