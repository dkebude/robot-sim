# robot-sim

A very (and I mean **very**) simple robot arm simulator using OpenGL and [Dean Butcher](https://github.com/ButchDean/interactive_computer_graphics)'s Interactive Computer Graphics implementation for Linux. Edward Angel's original codes of Interactive Computer Graphics for Windows and Mac can be reached [here](http://www.cs.unm.edu/%7Eangel/BOOK/INTERACTIVE_COMPUTER_GRAPHICS/SIXTH_EDITION/CODE/).

To be able to use the makefile, you should place your robot-sim folder inside a folder located in Dean Butcher's implementation of Interactive Computer Graphics.

Here is a rapid walkthrough to use the simulator (which is also reachable by pressing "h" after opening):

To control the robot:
First, select an arm member (either the base, the lower arm or the upper arm) by clicking on it.

* Then, use the following keys for rotation around the joints:
	* **S** for counter-clockwise (CCW) and **W** for clockwise (CW) rotation around *x-axis*
	* **Z** for counter-clockwise (CCW) and **X** for clockwise (CW) rotation around *y-axis*
	* **A** for counter-clockwise (CCW) and **D** for clockwise (CW) rotation around *z-axis*

* To manipulate the scene use the following keys:
	* **L** key to zoom *in* and **shift + L** keys to zoom *out*
    * **J** and **K** keys to move along *x-axis* (left and right)
    * **U** and **M** keys to move along *y-axis* (up and down)

* Use **I** key for going back to the initial position of the robot.
* Use **F** key for going to the shut-off position of the robot.

* Press Q to quit the simulator.