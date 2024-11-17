# Windows-KeyLogger

This project was created for educational purposes only, with no malicious intent. The goal is to understand how to interact with peripherals using the Windows API and explore what is technically possible. This project is not intended to harm anyone. **I am not responsible for any misuse of this code.**

---

## How to Build

To build the keylogger, ensure you are using Windows 11. Use the following commands to compile the main script and the server:

### config.h
Modify the template **config_ex.h** and put your information

### Main Script:
```bash
gcc main.c -o main.exe -lws2_32 -D_WIN32_WINNT=0x0601
```
### Server
```bash
gcc server.c -o server.exe -lws2_32 -D_WIN32_WINNT=0x0601
```
## Program Functionality
The program hooks into the keyboard to intercept keystrokes, sending the data to the server while also saving it to a local file.

## Configuration Options:
### SERVER_MODE:
Set to 0 to enable "offline" mode, where the keylogger does not communicate with the server.

### WRITE_FILE:
Set to 0 to disable saving intercepted keystrokes to a file.

Feel free to modify the code to customize the format or behavior of the program.

## Improvment 
I am still working on the following problems:
- the keylogger is just a shell programm, so if the user closes the shell the programs might stop, i need to modify it in order to let the executive launch as a deamon
- there should be less print statement in order to be more silent
- Right know i need to put the name of the server in the main, so i should find a way to obscure it 

## Disclaimer:
This project is for educational use only. Any illegal use of this software is strictly prohibited and entirely your responsibility.

