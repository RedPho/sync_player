#include <cstddef>
#include <cstdlib>
#include <iostream>
#include <vector>
#include <thread>
#include <mutex>
#include <asio.hpp>
#include <mpv/client.h>
#include <set>
#include "networkData.hh"

static inline void check_error(int status)
{
    if (status < 0) {
        printf("mpv API error: %s\n", mpv_error_string(status));
        exit(1);
    }
}


std::set<std::string> allowed_ips = {"192.168.1.1", "10.0.0.1", "127.0.0.1"};

void accept_loop(asio::ip::tcp::acceptor& acceptor, std::vector<asio::ip::tcp::socket*>& allClients, std::mutex& clientMutex, asio::io_context& io_context)
{
    while(true) {
    auto* client = new asio::ip::tcp::socket(io_context);

        try {
            // Accept the connection
            acceptor.accept(*client);

            // Retrieve the remote IP address
            std::string remote_ip = client->remote_endpoint().address().to_string();

            // Check if the IP is allowed
            if (allowed_ips.find(remote_ip) == allowed_ips.end()) {
                std::cout << "Connection from disallowed IP: " << remote_ip << "\n";
                client->close();
                delete client;
                continue;
            }

            // Lock the mutex before modifying the vector
            {
                std::lock_guard<std::mutex> lock(clientMutex);
                std::cout << "TCP client connected: " << remote_ip << "\n";
                allClients.push_back(client);
            }
        } catch (const std::exception& e) {
            std::cerr << "Exception: " << e.what() << "\n";
            delete client;
        }

    }
}


int main(int argc, char *argv[]) {
    mpv_handle *ctx = mpv_create();
    if (!ctx) {
        std::cout << "failed creating context\n";
        return 1;
    }

    std::vector<asio::ip::tcp::socket*> allClients;
    std::mutex clientMutex;

    try {

        int port = 1234;
        std::cout << "Enter port:\n";
        std::cin >> port;

        std::string whitelistIp{};
        while (true) {
            std::cout << "Enter IP address to add to whitelist or 'q' to continue:\n";
            std::cin >> whitelistIp;
            if (whitelistIp == "q") {
                break;
            }
            allowed_ips.insert(whitelistIp);
        }

        asio::io_context io_context;
        asio::ip::tcp::acceptor acceptor(io_context, asio::ip::tcp::endpoint(asio::ip::tcp::v4(), port));
        std::thread accept_thread(accept_loop, std::ref(acceptor), std::ref(allClients), std::ref(clientMutex), std::ref(io_context));


        // Enable default key bindings, so the user can actually interact with
        // the player (and e.g. close the window).
        check_error(mpv_set_option_string(ctx, "input-default-bindings", "yes"));
        mpv_set_option_string(ctx, "input-vo-keyboard", "yes");
        int val = 1;
        check_error(mpv_set_option(ctx, "osc", MPV_FORMAT_FLAG, &val));

        // Done setting up options.
        check_error(mpv_initialize(ctx));

        // Play this file.
        if (argc == 2) {
            // Play this file if provided.
            const char *cmd[] = {"loadfile", argv[1], NULL};
            check_error(mpv_command(ctx, cmd));
        } else {
            // Start mpv in idle mode to open an empty window.
            const char *idle_cmd[] = {"loadfile", "no", NULL};
            check_error(mpv_command(ctx, idle_cmd));
            const char *force_window_cmd[] = {"set", "force-window", "yes", NULL};
            check_error(mpv_command(ctx, force_window_cmd));
            std::cout << "No file provided, starting with an empty mpv window.\n";
        }

        // Let it play, and wait until the user quits.
        const int UNPAUSED = 0;
        const int PAUSED = 1;
        int isPaused{0};
        NetworkData networkData{UNPAUSED, 0.0};

        while (true) {
            mpv_event *event = mpv_wait_event(ctx, 0);
            int pausedData;
            double timePosData;

            mpv_get_property(ctx, "pause", MPV_FORMAT_FLAG, &pausedData);
            mpv_get_property(ctx, "time-pos", MPV_FORMAT_DOUBLE, &timePosData);

            if (isPaused != pausedData  || event->event_id == MPV_EVENT_SEEK) {
                isPaused = pausedData;
                networkData.paused = isPaused;
                networkData.timePos = timePosData;

                // Lock the mutex to safely access the shared client list
                try {
                    std::lock_guard<std::mutex> lock(clientMutex);
                    for (asio::ip::tcp::socket *socket: allClients) {
                        std::error_code error;
                        asio::write(*socket, asio::buffer(&networkData, sizeof(networkData)), error);
                        if (!error) {
                            std::cout << "data sent\n";

                        }else {
                            std::cout << "error:" << error.message() << "\n";
                            socket->close();
                            // Remove the socket if something wrong with it
                            auto it = std::find(allClients.begin(), allClients.end(),
                                                socket);

                            if (it != allClients.end()) {
                                allClients.erase(it);
                                std::cout << "erased\n";
                            }
                        }
                    }
                } catch (std::exception& e) {
                    std::cout << e.what();
                }
                std::cout << allClients.size() << " boyutunda allClients.\n";
            }

            if (event->event_id == MPV_EVENT_SHUTDOWN) {
                break;
            }
        }

        // Cleanup
        acceptor.close();  // Close the acceptor to stop the accept loop
        io_context.stop(); // Stop the io_context to break out of the event loop
        accept_thread.join();  // Join the thread to make sure it has finished

        for (asio::ip::tcp::socket* socket : allClients) {
            delete socket;
        }
        mpv_terminate_destroy(ctx);
    } catch (std::exception& e) {
        std::cerr << "Exception: " << e.what() << "\n";
        return 1;
    }

    return 0;
}
