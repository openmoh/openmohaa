# Creating a Docker image

OpenMoHAA dedicated server can run inside a container.

## Preparing files

In the root of the project repository, you'll find a folder named `docker`. Copy your MOH:AA directory inside `docker/server/full/game`. The directory structure should look like this:

```
[docker/server/full/game]
├── [main]
│   └── Pak*.pk3
├── [mainta]
│   └── pak*.pk3
├── [maintt]
│   └── pak*.pk3
└── [home]
    ├── [main]
    │   └── Custom PK3s go here
    ├── [mainta]
    │   └── Custom PK3s go here
    └── [maintt]
        └── Custom PK3s go here
```

You can place custom files such as the `server.cfg` file, inside `docker/server/full/game/home/main`.

### File locations inside the container

- Game files are located in `/usr/local/share/mohaa`.
- Custom content is located in `/usr/local/share/mohaa/home`.

## Building the image

To build the base and full Docker images, use the following commands:
```sh
docker build -t openmohaa/base docker/server/base
docker build -t openmohaa/full docker/server/full
```

## Running

To run the Docker container, use:
```sh
docker run -p 12203:12203 -p 12300:12300 openmohaa/full
```

💡 You can also use a Docker Compose file for easier management. Check out the `docker/server/docker-compose-example.yaml` file for an example.

## Environment variables

- `GAME_PORT`: Specifies the game port to use (default: 12203). The container and the host port must match, as Gamespy sends the game port to use.
- `GAMESPY_PORT`: Specifies the Gamespy port to use (default: 12300). The container and the host port must match because Gamespy sends this port to the master server.

## (Optional) Mounting volumes for settings

If you want to modify the `server.cfg` file without rebuilding the container, you can mount a folder for configuration files. Here's how:

1. Inside `docker/server`, create a `config/settings` folder.
2. Place your `server.cfg` file inside `config/settings`.
3. Run the container with the following command:
```sh
docker run -p 12203:12203 -p 12300:12300 -v config/settings:/usr/local/share/mohaa/home/main/settings openmohaa/full.
```

This will mount your `settings` folder directly into the container, allowing `server.cfg` to be edited without needing to rebuild the image.
