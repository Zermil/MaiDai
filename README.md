# MaiDai

MIDI 'controller' written in C++ (the fundamentals/basics of it) for Windows,
it connects to your MIDI device and outputs sound based on which 'note' you pressed.
Alternatively it sends signal TO your MIDI device from a midi file, essentialy 'telling' it what to play.

## Quick start

### Compile/Build:

```console
> build
```

### Run:

Play notes FROM MIDI controller:

```console
> ./maidai
```

Send notes TO MIDI controller:

```console
> ./maidai [MIDI FILE]
```