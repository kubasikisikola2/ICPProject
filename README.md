# ICPProject

## Controls
Mouse - move camera
W, S, A, D - forward, backward, left, right
Space - jump (up in free-fly mode)
Left shift - down in free-fly mode
Left click - shoot
Right click - release cursor
Escape - quit game
V - toggle VSync
Tab - toggle ImGUI
P - toggle pause
M - toggle all sounds and music
N - toggle music
K - toggle player mode between first-person and free-fly
F11 - toggle fullscreen
F10 - take a screenshot

## Webcam input
The game senses if a face is present and draws a tracking cross at the center of the face. If no or more than one face is present, the game is paused.

## Microphone input
The game senses loudness of microphone input and displays it in ImGUI. If the loudness is above 0.2, a small box in the corner of the room will rotate towards the camera.

## Gameplay
The player is in a shooting range with targets and a gun. If the player successfully shoots a target, a sound is played and the target respawns elsewhere and the game tracks the number of all shots fired and player's shooting accuracy and displays it in ImGUI.
There is a small box in the corner of the room that periodically emits a knocking sound and rotates towards the player when the microphone input exceeds a certain level of loudness. The box is not destructible, shooting it does nothing.

## Build instructions
We're building this in Visual Studio, but other IDEs should work. If opening in Visual Studio use "open a local directory". OpenCV is required to be installed on the computer and either have an environment variable pointing to the OpenCV build directory or edit the CMakeLists.txt. Also if the build fails on Linux, try to comment out "find_package(TBB REQUIRED)" and "target_link_libraries(ICPProject1 PRIVATE TBB::tbb)".