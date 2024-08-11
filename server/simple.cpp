// Build with: gcc -o simple simple.c `pkg-config --libs --cflags mpv`

#include <cstddef>
#include <cstdlib>

#include <mpv/client.h>
#include <string>
#include <iostream>
#include <vector>
#include <SDL3_net/SDL_net.h>
#include "networkData.hh"

static inline void check_error(int status)
{
    if (status < 0) {
        printf("mpv API error: %s\n", mpv_error_string(status));
        exit(1);
    }
}

int main(int argc, char *argv[]){
    if (argc != 2) {
        std::cout << "pass a single media file as argument\n";
        return 1;
    }

    mpv_handle *ctx = mpv_create();
    if (!ctx) {
        std::cout << "failed creating context\n";
        return 1;
    }

    std::vector<SDLNet_StreamSocket*> allClients{};

    SDLNet_Init();
    SDLNet_Server* server = SDLNet_CreateServer(NULL, 1234);



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

    // Let it play, and wait until the user quits.
    const int UNPAUSED = 0;
    const int PAUSED = 1;
    int isPaused{0};
    NetworkData networkData{UNPAUSED, 0.0};
    while (true) {
        int pausedData;
        double timePosData;

        SDLNet_StreamSocket* client;
        SDLNet_AcceptClient(server, &client);

        if (client != NULL) { //new client logic
            std::cout << "tcp client connected.";
            std::cout.flush();
            allClients.push_back(client);
        }

        mpv_event *event = mpv_wait_event(ctx, 0);
        mpv_get_property(ctx, "pause", MPV_FORMAT_FLAG, &pausedData);
        mpv_get_property(ctx, "time-pos", MPV_FORMAT_DOUBLE, &timePosData);

        //std::cout << *data << "\n"; // 1 is paused, 0 is not paused.
        if ( (!isPaused && pausedData == PAUSED) || (isPaused && pausedData == UNPAUSED) || event->event_id == MPV_EVENT_SEEK) {
            if (isPaused == 0) {
                isPaused = 1;
            } else{
                isPaused = 0;
            }
            //paused and pozisyon send to clients.
            networkData.paused = isPaused;
            networkData.timePos = timePosData;
            for (SDLNet_StreamSocket* socket : allClients) {
                SDLNet_WaitUntilStreamSocketDrained(socket, -1);
                SDLNet_WriteToStreamSocket(socket, &networkData, sizeof(networkData));
                std::cout << "data sent\n";
                std::cout.flush();
            }
        }

        //std::cout << "event: " << mpv_event_name(event->event_id) << "\n";
        std::cout.flush();
        if (event->event_id == MPV_EVENT_SHUTDOWN) {
            break;
        }
    }

    mpv_terminate_destroy(ctx);
    return 0;
}
