Bluetooth Module:
	•	Upload the code in the ArduinoCode folder to an Arduino (we used a mega 2560).

Kinect Module:
	•	In the Windows8App folder, open the BluetoothCommunicationSample VS Solution.
	•	Click the green run button on the top toolbar to run the program on the local machine.
	•	When the pop-up window opens, click connect to establish a connection with the HS-06 bluetooth module. 

Kernel Module:
	•	Use the Makefile to compile the kernel module and make the binary.
	•	Using zmodem in minicom, upload the mysnake.ko file to the /home folder on the gumstix.
	•	Make a node using the following command.
		Mknod /dev/mysnake c 63 0
	•	Insert kernel module using the following command.
		Insmod mysnake.ko
	•	If needed, remove the kernel module using the following command.
		Rmmod mysnake.ko

LCD Module:
	•	Use the Makefile to compile the Qt main.cpp file and make the binary.
	•	Create library links in gumstix: 	cd /usr/lib 	ln -s /media/card/lib/libQtCore.so.4 libQtCore.so.4 	ln -s /media/card/lib/libQtGui.so.4 libQtGui.so.4 	ln -s /media/card/lib/libQtNetwork.so.4 libQtNetwork.so.4 	ln -s /media/card/lib/ld-uClibc.so.0 ld-uClibc.so.0 	ln -s /media/card/lib/libc.so.0 libc.so.0 	ln -s /media/card/lib/libm.so.0 libm.so.0 	ln -s /media/card/lib/libstdc\+\+.so.6 libstdc\+\+.so.6
	•	Export some variables in gumstix: 	export QWS_MOUSE_PROTO='tslib:/dev/input/touchscreen0' 	export TSLIB_CONFFILE=/etc/ts.conf 	export TSLIB_PLUGINDIR=/usr/lib 	export TSLIB_TSDEVICE=/dev/input/event0 	export TSLIB_FBDEVICE=/dev/fb0 	export TSLIB_CONSOLEDEVICE=/dev/tty 	export QT_QWS_FONTDIR=/media/card/lib/fonts 	export TSLIB_PLUGINDIR=/usr/lib/ts
	•	Run the Qt program
		./qt -qws
