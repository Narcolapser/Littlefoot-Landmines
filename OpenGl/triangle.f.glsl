#version 120

void main(void) 
{
//	gl_FragColor[0] = 0.0;
//	gl_FragColor[1] = 0.0;
//	gl_FragColor[2] = 1.0;
//	gl_FragColor[3] = floor(mod(gl_FragCoord.y, 2.0));
//these 4 lines are all that was changed. not that there really was much in this file anyway.
//what is happening here is it is taking the current x and the current y location and dividing
//those values by the width and heigth respectively so that there is a color gradient from one
//vertex to the next. so in the top right those two, red and green will be full on but in the bottom
//left they will be almost all off. 
//the alpha channel, the 4th line, takes a the mod2 of the y coord, and the floor of that value. 
//now, mod2 should return either 0 or 1. but this was written by people who are probably much smart
//or experienced than I, so I'm not going to argue. the floor operation isn't particularly expensive
//anyway. so to get 100% predictable results, the floor function everyone.
	gl_FragColor[0] = gl_FragCoord.x/640.0;
	gl_FragColor[1] = gl_FragCoord.y/480.0;
	gl_FragColor[2] = 0.5;
	gl_FragColor[3] = floor(mod(gl_FragCoord.y, 2.0));
}
