# Amusement-Park-Sim



To start off you want to make sure you have the X Windows Systems before you can run the make file.

To install the X11 on debian-based systems use: sudo apt-get install libx11-dev

Once the X11 is installed you can run 'make'

You'll now have 4 executables

[fairApp]

[guest]

[generator]

[stop]

You'll want to run fairApp in the background ./fairApp& which should just pop up the graphic interface.

![image](https://github.com/New101Leaf/Amusement-Park-Sim/assets/104871189/da10644f-b3f3-4dc7-ae0e-1f568c1773f5)

Now that the server thread and ride threads are booted up, it is now ready to accept guest threads.

guest: takes the format ./guest <ticket> <wait time> <ride number>]

is the amount of tickets the guest will contain

is the amount of time the guest that determines if the guest will wait in line

a number between 0-9 which will be the guest's first ride if available else the next ride is randomly selected

ex. ./guest 25 900 0

[generator: randomly generates 100 guest to enter the fair]

[stop: shutdown the fair]

https://github.com/New101Leaf/Amusement-Park-Sim/assets/104871189/6fff6227-63dc-43cf-89ed-9ae295dd6e51

