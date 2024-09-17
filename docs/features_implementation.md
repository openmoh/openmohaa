# Implementation

## Server

The server version can be built successfully. The server and the fgame components are fully implemented, the single-player campaign can be played from start to end in MOH:AA, MOH:SH and MOH:BT. However there could be some minor bugs (and rare crashes), but overall, the game should be playable.

## Client

The client version of OpenMoHAA is also fully implemented with a few bugs remaining. Credits to the SDK of **Heavy Metal: F.A.K.K. 2** both the cgame and fgame modules.

OpenMoHAA is using OpenAL for the audio.

The current operational status for each component is as follow:

| Component               | Full | Almost | Half | Early | Bad | Not working | Comment                                                             |
|-------------------------|------|--------|------|-------|-----|-------------|---------------------------------------------------------------------|
| Audio                   | x    |        |      |       |     |             | Full implementation using OpenAL                                    |
| CG Module               | x    |        |      |       |     |             | Some effects can be inaccurate compared to the original game        |
| Client                  | x    |        |      |       |     |             |                                                                     |
| Collision               | x    |        |      |       |     |             |                                                                     |
| Model/TIKI/Skeletor     | x    |        |      |       |     |             |                                                                     |
| Renderer                | x    |        |      |       |     |             |                                                                     |
| Server                  | x    |        |      |       |     |             |                                                                     |
| Server module (fgame)   | x    |        |      |       |     |             | There might still be some AI-related bugs remaining                 |
| UI                      | x    |        |      |       |     |             |                                                                     |

Full = code is fully working, however it doesn't mean it's bug-free.
