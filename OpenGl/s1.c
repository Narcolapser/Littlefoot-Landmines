/* You are actually ingluding:
#include <GL/gl.h>
#include <GL/glu.h>
#include <GL/glut.h>
* but because glut will include glu and gl you can get away with just including glut.
* for historical and clearity reasons, people still include gl and glu, this has no
* negative effect on your program aside from adding 2 extra lines of code that don't
* actually serve a purpose. */

#include <GL/glut.h>

void display() {}//for now this will be an empty function. GLUT requires a function
	//of some sort to call when you create the window. so this is ncessesary.

int main(int argc, char *argv[])
{
        glutInit(&argc, argv);//this is used to initialize the glut library, it also
		//will deal with the underlying system for you. usage of the glut
		//library will make your program cross platform with zero effort. 
		//the cost is that glut is very frill free. you don't get anything 
		//from the OS. the result is that your program will look exactly the
		//same on any platform. but that also means that it won't "blend in"
		//so to speak by taking the platform dependent looks and feels for
		//things like buttons and menus. not a big problem for a game engine.
		//the arguments here, argc, and argv are kinda just passed through.
		//you certainly can pass what ever arguements you want to it. argc
		//is the number of args and argv is the array of c-strings. the args
		//that are passed are parsed in platform dependent ways. so really
		//you should worry to much about them. but it needs something, so 
		//may as well give it those two, their ready made and extra args are
		//just ignored by it.
        glutInitDisplayMode(GLUT_RGB | GLUT_DEPTH | GLUT_DOUBLE);
		//this sets the initial display mode. those values aer passed in a
		//weird way no? this a little bit of a cruel way of doing things to
		//some one who is not familiar with bit wise functions. what happens
		//here is called "bitwise OR". each of the GLUT constants in the 
		//function call have their own binary value. say GLUT_RBG has 0001,
		//GLUT_DEPTH has 0010, and GLUT_DOUBLE has 1000. if you bitwise or
		//something, what you are really saying is that if there is a 1 in
		//this spot in value a or in value b. set the resulting value's
		//position to have a 1 there too. so in thise case it would be 1011.
		//now that is more than likely not what it actually is. but you get
		//my point. each of these constants have their own value. when you
		//bitwise or the values together you get a new value, but the info
		//of each individual constant is maintained. now, this same result
		//could have been achieved if they had an array and you passed
		//the values in that way, and that would have been easier for a newb
		//to pick up on. but when you understand how this works, it is
		//actually faster to program and a lot faster for the computer.
		//the values you can pass here can be found here:
		// http://www.opengl.org/documentation/specs/glut/spec3/node12.html
		//amongst many other places. The values passed here are:
		//	RGB: the color mode. RGB means you will give values to
		//		the red, green, and blue, individually.
		//	Depth: whether or not you want a depth buffer. just nod.
		//	Double: If you want a double buffered window. just nod.
		//depth buffer and double buffers will be explained later.
        glutInitWindowSize(800,600);
		//this sets how big you want your window to be. 800 wide, 600 tall.
		//pretty self explainatory, but at this point you still have no 
		//window. but i will take this moment to mention one other function
		//glutInitWindowPosition(int x, int y) sets where the window is to
		//appear when it finally does appear which is next!:
        glutCreateWindow("Hello World");
		//creates the bloody window! really, this is all this command does.
		//the arguement is the string to put into the title bar. yes, it is
		//a string not a pointer to a string so you may not change it later.
		//if you wanted to have the label be something different later, glut
		//is not going to help you. technically glut may not even get it
		//there in the first place, quote from the glut specs:
		//	The name will be provided to the window system as the window's name. 
		//	 The intent is that the window system will label the window with the name. 
		//yea, confidence instilling no? but that's what you've got.
		//also, i lied. the window has been created, but you still can't
		//see it. stay tuned for detail.
        glutDisplayFunc(display);
		//this is used to set the display call back function. if you are
		//a newb to c as well as openGL, that probably went in one ear
		//and out the other. let me address the c pros who are opengl
		//newbs quick first, you might get a little out of this too:
		//during the opengl main loop, when a redisplay is needed, the 
		//function you pass here is going to be the one that is called
		//when it needs to do such.
		//ok, for c newbs. i'm going to assume you are coming from java,
		//(yes winkle, i'm thinking of you. <3) in which case this is going
		//to be a little crazy, if you are coming from python, this isn't
		//to strange. but what is happening here is you are passing a
		//pointer to a function. this doesn't happen in Java, it can in
		//Python. the glut main loop will run through all of its tasks
		//every frame and then it will need to come to your program to ask
		//you to redraw the window. this is the kick off point for you to
		//do everything you need/want to do when drawing the next frame.
        glutMainLoop();
		//this function !!!!!NEVER RETURNS!!!
		//do not put anything after this function. at least if you ever
		//want that code to run ever. this may raise concerns on your part 
		//in terms of "but, le gasp! what about those files i opened, and
		//ports i set to listen to! if i don't close them, bad things will
		//happen!" this is true, and how this will be fixed, I don't know
		//yet. but at the level we are dealing with here, it doesn't matter.
		//sit back and enjoy the ride.
		//Now that you know to never put anything after this function that
		//you ever want to see in anyway. when you run this command it 
		//actually starts your glut program. you now have a bloody window.
		//of course because the display function is yet un-declared, it has
		//rather comical results. i don't know what it will do on windows.
		//but here on ubuntu the window is created as if there was 
		//absolutely nothing in it, and so you can see right through it. 
		//but then you move the window and it keeps the contents it had.
		//what is happening is the graphics memory buffer is never getting
		//erased. that will be taken care of in the next step in this 
		//little openGL escipade! on ward and sideways!
        return 0;
}
/*.S.D.G*/
