This is a program to watch local videos (or online videos if you have the link) together with friends in sync.
I created this project because screen sharing alternatives don't always work well (due to internet speed or platform limitations).
It works on Linux and Windows.
# Build:
Just open the command line in the client or server folder and type the following commands:
```commandline
mkdir build
cd build
cmake ..
cmake --build . --config Release
```
# Usage:
Simply enter the path of the executable in the command line or double-click the .exe file if on Windows.  
```commandline
path_to_client_executable_folder/sync_player_client
```
```commandline
path_to_client_executable_folder/sync_player_server
```
The server and client can learn their public IPv4 easily by searching online:  
Open your favorite search engine(Google, DuckDuckGo etc) and search: "What is my ip".  
Open the server first. It will ask for a port. Type the port as you wish.  
Note: Your router need to forward this port to your local ip address(to your computer).  
If you don't know what port forwarding is, you can search it online. It is not that complex.  
Then it will ask for ip to whitelist.  
Then, the client must enter the IP and port of the server.  
It connects, and the server sends the time and pause data when the server user seeks or pauses the media.  
You can also start the server and client like this to start the video immediately:  
```commandline
path_to_client_executable_folder/sync_player_client path_to_video
```
```commandline
path_to_server_executable_folder/sync_player_server path_to_video
```
or if you wish to play videos online (YouTube, etc.):  
```commandline
path_to_client_executable_folder/sync_player_client video_link
```
```commandline
path_to_server_executable_folder/sync_player_server video_link
```

# TODO
1. Android and iOS support (far future)
2. User-friendly interface  
3. Separate host from server (This way, I can use a server so that users don't need to configure their own servers, and everyone can use it)
4. Chat, reactions
5. Pointer: A user can point to something visual in the video.
6. Whitelist save and no whitelist option.