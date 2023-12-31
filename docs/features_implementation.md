# Implementation

## Server

The server version can be built successfully, but some features are not functional. For example, the Actor system is almost fully implemented, but it is not yet stable. This means that Actor may not work as expected and could cause crashes.

Overall, the server and the fgame components are almost fully implemented, but they may not be completely stable. There could be some minor bugs (and rare crashes), but the game should be playable.

## Client

The client version of OpenMoHAA has undergone partial implementation, with the cgame module being nearly completed. Credits to the SDK of **Heavy Metal: F.A.K.K. 2** both the cgame and fgame modules.

The current operational status for each component is as follow:

| Component               | Full | Almost | Half | Early | Bad | Not working | Comment                                                           |
|-------------------------|------|--------|------|-------|-----|-------------|-------------------------------------------------------------------|
| Audio                   |      |        |      |       | x   |             | Very basic implementation from Quake III                          |
| CG Module               |      | x      |      |       |     |             | Missing FX, Marks and decals                                      |
| Client                  | x    |        |      |       |     |             |                                                                   |
| Collision               | x    |        |      |       |     |             |                                                                   |
| Model/TIKI/Skeletor     | x    |        |      |       |     |             |                                                                   |
| Renderer                |      | x      |      |       |     |             | Missing ghost, marks, sphere lights, sky portal, sun flare, swipe |
| Server                  |      | x      |      |       |     |             | Probably a few bugs remaining                                     |
| Server module (fgame)   |      | x      |      |       |     |             | Actor, and few gameplay bugs                                      |
| UI                      |      | x      |      |       |     |             | Dialog, field, and a few bugs to fix                              |
