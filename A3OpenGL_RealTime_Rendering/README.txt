CSC305 Assignment3 OpenGL
Tong Zhang
V00926513
03-15-2020

Basic Features (All implemented)
	Render a cube (implemented)
	Use a pinhole camera with perspective projection (implemented)
	Uses a point light (use small light cube to represent)
	Uses diﬀuse shading (implemented)

Advanced Features (5 implemented) 
	Implements a directional light (the green directional light, postion represented by a green panel lamp)
	Implements specular reﬂection (implemented)
	Implements specular shading with multiple light sources (1 directional light + 3 point lights)
	Implements a ﬁrst-person view camera (use keyboard 'w' 'a' 's' 'd' to move, use mouse control to tilt camera)
	Loads and renders a simple mesh (mesh of suzanne)

Files: please put all files under the same directory, if the program cannot find "suzanne.obj" it will only renders a cube
	main.cpp ( 33 KB ) 
	assignment.hpp ( 4 KB ) 
	cube.vert ( 1 KB ) 
	cube.frag ( 3 KB ) 
	Mesh.vert ( 1 KB ) 
	Mesh.frag ( 3 KB ) 
	point_light.frag ( 1 KB ) 	//for the light cube
	point_light.vert ( 1 KB ) 	//for the light cube
	CMakeLists.txt ( 1 KB ) 
	suzanne.obj ( 987 KB ) 		//simple mesh model
	README.txt ( 1 KB ) 

