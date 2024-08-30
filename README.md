This is a program to watch local videos(or online videos if you have the link of the video) together with friends in sync.  
I created this project because screen sharing alternative doesn't always work nice(because of the internet speed or limitations of the platforms)  
Works for Linux and Windows
# Build:
Just open the command line in client or server folder and type commands:
```commandline
mkdir build
cd build
cmake ..
cmake --build . --config Release
```
# Usage:
Just enter the path of the executable in command line or double click to the exe file if on Windows.  
Open the server first. It asks for port. Type the port as you wish.  
Then client must enter the ip and port of the server.  
It connects and the server sends the time and pause data when the server user seek or pause the media.  
You can also start the server and client like that to start video immediately:
```commandline
path_to_client_executable_folder/sync_player_client path_to_video
```
or if you wish to play videos online(youtube etc.)
```commandline
path_to_client_executable_folder/sync_player_client video_link
```

# TODO
1. Android and ios support(far future)  
2. User friendly interface  
3. Separate host from server. (This way I can use a server so that users don't need to configure their own servers and also everyone can use it)