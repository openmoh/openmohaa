# Getting Started

## Installation

This guide assume you have already acquired and installed a copy of MOH:AA on your server. If not, see [Getting Started](getting_started_installation.md).

1) Download the [latest release](https://github.com/openmoh/openmohaa/releases) that matches your server's platform.
2) Extract files from the archive into your MOH:AA directory

## Preparing the server settings

1) Download [server_opm.cfg](files/server_opm.cfg) as a template for your server configuration, it contains the most relevant settings, make sure to modify them appropriately.
2) Place the `server_opm.cfg` file inside the **main** directory of your game folder.

### Configuring the server

- For a list of commonly used settings in the `server.cfg` file, see [Server configuration](configuration_server.md).
- For examples and new OpenMoHAA features (like setting up bots), see [Game configuration](configuration.md).

## Starting the server

Run the `omohaaded` executable from your MOHAA directory. Use one of the following commands, depending on the game you want to use:

- For **Allied Assault**:
  
  `./omohaaded +set com_target_game 0 +exec server_opm.cfg`
- For **Spearhead**:

  `./omohaaded +set com_target_game 1 +exec server_opm.cfg` 
- For **Breakthrough**:

  `./omohaaded +set com_target_game 2 +exec server_opm.cfg`
