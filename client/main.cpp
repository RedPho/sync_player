#include <mpv/client.h>
#include <string>
#include <iostream>
#include <asio.hpp>
#include <thread>
#include "networkData.hh"

using asio::ip::tcp;

static inline void check_error(int status)
{
    if (status < 0) {
        printf("mpv API error: %s\n", mpv_error_string(status));
        exit(1);
    }
}

void connectToServer(tcp::socket& socket, const std::string& ipString, int port, asio::io_context& io_context) {
    while (true) {
        try {
            tcp::resolver resolver(io_context);
            tcp::resolver::results_type endpoints = resolver.resolve(ipString, std::to_string(port));
            asio::connect(socket, endpoints);
            std::cout << "Connected to server\n";
            return;
        } catch (const std::exception& e) {
            std::cerr << "Failed to connect: " << e.what() << "\n";
            std::cout << "Reconnecting in 5 seconds...\n";
            std::this_thread::sleep_for(std::chrono::seconds(5));
        }
    }
}

int main(int argc, char *argv[]) {
    mpv_handle *ctx = mpv_create();
    if (!ctx) {
        std::cout << "failed creating context\n";
        return 1;
    }

    // Enable default key bindings
    check_error(mpv_set_option_string(ctx, "input-default-bindings", "yes"));
    mpv_set_option_string(ctx, "input-vo-keyboard", "yes");
    int val = 1;
    check_error(mpv_set_option(ctx, "osc", MPV_FORMAT_FLAG, &val));
    check_error(mpv_initialize(ctx));

    // Set up ASIO
    asio::io_context io_context;

    std::string ipString{"127.0.0.1"};
    std::cout << "Enter host IP:\n";
    std::cin >> ipString;
    int port{1234};
    std::cout << "Enter port:\n";
    std::cin >> port;

    tcp::socket socket(io_context);
    connectToServer(socket, ipString, port, io_context);

    std::cout << "connected\n";

    if (argc == 2) {
        const char *cmd[] = {"loadfile", argv[1], NULL};
        check_error(mpv_command(ctx, cmd));
    } else {
        const char *idle_cmd[] = {"loadfile", "no", NULL};
        check_error(mpv_command(ctx, idle_cmd));
        const char *force_window_cmd[] = {"set", "force-window", "yes", NULL};
        check_error(mpv_command(ctx, force_window_cmd));
        std::cout << "No file provided, starting with an empty mpv window.\n";
    }

    NetworkData networkData{0, 0.0};
    while (true) {
        try{
            asio::read(socket, asio::buffer(&networkData, sizeof(networkData)));
            mpv_set_property(ctx, "pause", MPV_FORMAT_FLAG, (void *) &(networkData.paused));
            mpv_set_property(ctx, "time-pos", MPV_FORMAT_DOUBLE, &(networkData.timePos));
            std::cout << "data came\n";
            std::cout << "pause:" << networkData.paused << "\n";
            std::cout << "timepos:" << networkData.timePos << "\n";

        } catch (std::exception& e) {
            std::cerr << "Connection lost: " << e.what() << "\n";
            std::cout << "Attempting to reconnect...\n";
            connectToServer(socket, ipString, port, io_context);
        }


        mpv_event *event = mpv_wait_event(ctx, 0);
        if (event->event_id == MPV_EVENT_SHUTDOWN) {
            break;
        }
    }

    mpv_terminate_destroy(ctx);
    return 0;
}
