// Build with: gcc -o simple simple.c `pkg-config --libs --cflags mpv`

#include <cstddef>
#include <cstdlib>

#include <mpv/client.h>
#include <string>
#include <iostream>
#include <SDL3_net/SDL_net.h>
#include "networkData.hh"


static inline void check_error(int status)
{
    if (status < 0) {
        printf("mpv API error: %s\n", mpv_error_string(status));
        exit(1);
    }
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        std::cout << "pass a single media file as argument\n";
        return 1;
    }

    mpv_handle *ctx = mpv_create();
    if (!ctx) {
        std::cout << "failed creating context\n";
        return 1;
    }

    // Enable default key bindings, so the user can actually interact with
    // the player (and e.g. close the window).
    check_error(mpv_set_option_string(ctx, "input-default-bindings", "yes"));
    mpv_set_option_string(ctx, "input-vo-keyboard", "yes");
    int val = 1;
    check_error(mpv_set_option(ctx, "osc", MPV_FORMAT_FLAG, &val));

    // Done setting up options.
    check_error(mpv_initialize(ctx));

    // Play this file.
    const char *cmd[] = {"loadfile", argv[1], NULL};
    check_error(mpv_command(ctx, cmd));

    SDLNet_Init();

    std::string ipString{"127.0.0.1"};
    std::cout << "Enter host IP:";
    std::cin >> ipString;
    int port{1234};
    std::cout << "Enter port:";
    std::cin >> port;
    const char *cip = ipString.c_str();

    SDLNet_Address* address = SDLNet_ResolveHostname(cip);
    SDLNet_WaitUntilResolved(address, -1);
    SDLNet_StreamSocket *socket = SDLNet_CreateClient(address, port);

    if (SDLNet_WaitUntilConnected(socket, -1) < 1) {
        std::cout << "couldn't connect" << SDLNet_GetConnectionStatus(socket);
        return 0;
    }

    // Let it play, and wait until the user quits.
    NetworkData networkData{0, 0.0};
    while (true) {
        if (SDLNet_ReadFromStreamSocket(socket, &networkData, sizeof(networkData)) > 1){
            mpv_set_property(ctx, "pause", MPV_FORMAT_FLAG, (void *) &(networkData.paused));
            mpv_set_property(ctx, "time-pos", MPV_FORMAT_DOUBLE, &(networkData.timePos));
            std::cout << "data came:" << "\n";
            std::cout << "pause:" << networkData.paused << "\n";
            std::cout << "timepos:" << networkData.timePos << "\n";
            std::cout.flush();
        }

        mpv_event *event = mpv_wait_event(ctx, 0);
 /*     std::cout << "event: " << mpv_event_name(event->event_id) << "\n";
        std::cout.flush();*/
        if (event->event_id == MPV_EVENT_SHUTDOWN) {
            break;
        }
    }

    mpv_terminate_destroy(ctx);
    return 0;
}
