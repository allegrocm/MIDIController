Ken's Midi Input Readme


Hello!  This is my easy Midi input software that I put together for using a Midi mixer board to control parameters in a C++ application. It's super useful for quickly tweaking parameters in a game at runtime.  Mostly because you can just poll the slider positions, rather than hooking up GUI elements to specific variables. I wrote it for the KORG NanoKontrol 2 ( http://www.korg.com/us/products/controllers/nanokontrol2/ , about $50 from Amazon), but it should work with others.  It's heavily based on the PortMIDI project ( http://portmedia.sourceforge.net/portmidi/ ), and I've included the necessary (and slightly modified) bits of that project.  

Originally, I used ControllerMate (available for Mac) to transform Midi signals into inputs from set of virtual gamepad.  This makes them accessible from other languages and frameworks, such as Unity, but had the drawback of requiring purchasing ControllerMate if you wanted more than 8 analog inputs.  Also, it was Mac-only.  But I'm including the ControllerMate setup file I made if you'd like to go that route.  Even with the free version of ControllerMate, you can get input from the 8 sliders. (You may have to delete some connections from the file, but that's pretty easy to do).


Happy Midiing!  If you found this useful, please drop me a line!


Ken

http://www.kenkopecky.com