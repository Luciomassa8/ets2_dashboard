# et2_dashboard
Dashboard for Euro Truck Simulator 2.

Configuration: the ets_2_arduino.py script read data from the game using the scs-sdk-plugin available on "https://github.com/RenCloud/scs-sdk-plugin/tree/master" repo, then write it on the serial port on which Arduino is listening, then Arduino show the data on the OLED 0.96" I2C display.
The other py scripts are just for debugging and improvements:
- serial_scan.py reads all the available COM port and show it to you to make you able to choose the correct Arduino port.
- data_list.py print on the terminal all the keys exposed by the plugin, so you can use them to add new features to the project (you can find the complete data list in the attached data_list.txt file).

The .ino file is the one for Arduino. You can use every type of Arduino (Uno, Leonardo, Mega, etc.) because its function is just to send read messages on the serial port.
Once it read these data, it does its elaboration and show it on the display. On the first publication, I used a 0.96" OLED I2C display to ease the debug of the application.

All the comments in the scripts are in Italian.

Usage: build the Arduino setup and connect it to your PC, take the correct COM port and write it in the ets_2_arduino.py script, then start the script and, lastly, start the game.

Next update will envolve 8-segments displays to show the gears and so on, and I will make also an executable file instead of the .py script, as so as a config file to modify the COM port instead of doing it inside the script.