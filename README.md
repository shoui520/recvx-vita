# recvx-decomp

[![Build Status]][actions] [![Code Progress]][progress] [![Data Progress]][progress] 

[Build Status]: https://github.com/AshfordFamily/recvx-decomp/actions/workflows/progress.yml/badge.svg
[actions]: https://github.com/AshfordFamily/recvx-decomp/actions/workflows/progress.yml

[Code Progress]: https://decomp.dev/AshfordFamily/recvx-decomp.svg?mode=shield&label=Code&measure=fuzzy_match_percent
[Data Progress]: https://decomp.dev/AshfordFamily/recvx-decomp.svg?mode=shield&label=Data&measure=matched_data_percent
[progress]: https://decomp.dev/AshfordFamily/recvx-decomp

<img src="https://i.imgur.com/FreVpxO.png"/> 

## PS Vita port

This fork builds the decompiled game as a native PS Vita application. The game code in `src/` and `include/` is the decompilation with portability changes: inline EE assembly is translated to C (the original assembly is kept beside it as a comment) and other fixes are marked `RECVX_VITA`. Because of the assembly translation, these sources no longer produce the matching PS2 ELF; use upstream [recvx-decomp](https://github.com/AshfordFamily/recvx-decomp) for that. The PS2 hardware is replaced by a platform layer in `platform/`:

- `platform/gs`: GS and VU1 rendering on the GPU through vitaGL (VU1 microprograms become shaders)
- `platform/iop`: the IOP side, including a reimplementation of the TSNDDRV sound driver
- `platform/ps2compat`: EE kernel, DMA, CD-ROM, memory card, pad and VU0 replacements
- `platform/vita`: startup, logging and movie playback

The port is early work: it boots through the title screen into gameplay, but FMVs are not played yet, some in-game models and text are missing, and 3D scenes run slowly.

### Building the VPK

Requirements: [VitaSDK](https://vitasdk.org) with the vdpm packages `vitaGL`, `vitashark`, `SceShaccCgExt`, `mathneon` and `taihen`, and CMake 3.16 or newer. The PS2 SDK and the MWCC compiler are not needed.

```sh
git clone https://github.com/shoui520/recvx-vita
cd recvx-vita
git submodule update --init include/recvx-decomp-cri include/recvx-decomp-katana
export VITASDK=/usr/local/vitasdk   # your VitaSDK install
cmake -S . -B build-vita
cmake --build build-vita -j
```

The package is `build-vita/recvx.vpk` (title ID `RECVX0001`).

### Running

1. Install `recvx.vpk` with VitaShell.
2. Copy an image of your own US disc (SLUS-20184) to `ux0:data/recvx/recvx.iso`.
3. vitaGL compiles shaders at runtime, so `ur0:data/libshacccg.suprx` must be present (see the vitaGL documentation).

Saves go to `ux0:data/recvx/mc0`, and a log is written to `ux0:data/recvx/log.txt`. The Vita has no L2/R2/L3/R3: L2/R2 are the left/right halves of the rear touchpad, L3/R3 the left/right halves of the front screen.

For testing, files in `ux0:data/recvx` switch on debug hooks; none exist by default:

| File | Effect |
| --- | --- |
| `boot.txt` containing `newgame` | skip the memory card check and title menu and start a new game |
| `input.txt` | scripted pad input, one `FRAME BUTTON[+BUTTON] [HOLD]` per line |
| `watchdog.txt` | crash on purpose (for a core dump) when no frame is shown for 15 s |
| `dumpat.txt` | crash on purpose at the given frame number |
| `capture.txt` | frame numbers to save as `frame-N.ppm` |

## About

> [!IMPORTANT]
**AI policy**: LLMs produced negligible decompilation results in 2024 and also a good deal of 2025. Since we're trying our best not to mess up a 2-year-old project, we ask that any AI-generated submission is disclosed to us and handled responsibly. 

**recvx-decomp** is a reverse-engineering project for Resident Evil: Code Veronica X which has the goal of reconstructing the source code of the game by decompiling the MIPS in the PS2 ELF back to C. The project currently only works with the US release (**SLUS-20184**), with plans to add support for more regions in the future.

Currently, the engine and gameplay systems are decompiled, as well as the GFX code and the **CRI ADXT (Jan 26th, 2001)** lib employed by the game. Enemy AI is ~~still incomplete~~ (EDIT: done now, the game is decompiled). Testing is done by repackaging the retail disc with our own compiled ELF using a script, and trying out the results on PCSX2. 

Groundwork has been made for decompiling the Dreamcast and GameCube releases of Code Veronica; see the Resources section on this page for some links. Once the project is completed, there will be many potential uses of the code, including and beyond porting.

## Building

> [!IMPORTANT] 
You will have to provide your own files for the PS2 API, the project only works with the **2.0.0** and **3.0.3** versions of the SDK. **compile_config.json** outlines the paths where the build system expects the SCE stuff.

First clone the repository: 
```
git clone --recursive https://github.com/AshfordFamily/recvx-decomp.git
```

Next, place your copy of the `SLUS_201.84` file from inside the game disc into the `config` folder. 

For this part of the setup, you can use a dev container (or not):

### Dev Container route

If you're using an IDE that supports dev containers such as Visual Studio Code, you can simply open up the repo as a container (you'll need to have Docker or Podman installed on your machine to use this feature).

### Manual route

Install splat with the following command: 
```
pip install -r config/requirements.txt
```

---

**Follow these instructions after performing the step of one of the two routes above:**

Use this command to setup objdiff:
```
python compile.py --setup
```

Once done, you should see a newly-generated `objdiff.json` project file and a `config/asm` folder.

From now on, to build this project you just need to run the compile script each time:
```
python compile.py
```

Note: if you're using Linux, wibo is needed in order to run `mwccps2.exe`. A small prompt with install steps for it will appear if the script can't find wibo in your path.

You can repackage CVX's disc image with the compiled ELF to see the decompiled code in action. You need to put your ISO dump of the game's DVD on the `iso` folder, and extract its contents there:
```
python mkiso.py -m extract --iso iso/RE_CVX.iso
```

Then repackage the ISO:
```
python mkiso.py -m insert
```

If the process is successful, there should be a new file called `RECVX_NEW.iso` that you'll be able to use to test the project with a PS2 emulator or a modded console.  

## Resources

Related decomp projects:
- [Resident Evil - Code: Veronica X (Nintendo GameCube)](https://github.com/fmil95/recvx-gc-decomp)
- [Resident Evil - Code: Veronica (Dreamcast)](https://github.com/fmil95/recv-dc-decomp)
- [Dino Stalker](https://github.com/fmil95/dinostalkRE)
- [Fahrenheit](https://github.com/fmil95/santamonica)
- [Fatal Frame](https://github.com/Mikompilation/Himuro)
- [Legacy of Kain: Soul Reaver](https://github.com/fmil95/soul-re)

Also be sure to check out these [neat patches for PCSX2](https://github.com/fmil95/cvxpacchi). AshfordFamily's org avatar fan art was designed by [fishiiarts_](https://www.instagram.com/fishiiarts_/).

## Disclaimer

recvx-decomp is licensed under **MIT License**, which allows for commercial use of the project's code. However, for commercializing ports of the game to modern platforms we still very much recommend contacting Capcom first for a proper publishing deal. 
