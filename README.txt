Libraries Required:
	opengl4
	glm
	glew
	sfml
	
Implementation:
	This engine uses the concept of pre-calculating particle's behavior as a polynomial equation, and setting each particle up with the coefficients at the beginning of it's life.
	This way, the only thing required to be updated for each particle every frame is the time since it's life began. Only one variable for the entire particle!
	This allows for huge numbers of particles to be moving at once, the current hard coded amount is 10,000 a second.
	The coefficients are batched as one push, and only happens when a new particle is added, or a particle is deleted.
	It's possible for particles to avoid being deleted, only having it's lifetime recycled to 0. This allows for the particle buffer to be filled, and then enter a state only requiring time update indefinititely.
	The downside of this however is that it locks the particles randomly sampled coefficients to a set number, but when there are so many on the screen at once, it's hard to tell.
	I'm also using OpenGL's instanced rendering feature, allowing huge numbers of the same textured object to be drawn at once.
	The shaders are hard coded as strings since this was meant to be a quick and dirty prototype.
	
Current parameters for particles are:
Note: All of these can be randomly sampled on particle creation

	position start
	position over time
	(position over time) over time
	
	rotation start
	rotation over time
	(rotation over time) over time
	
	scale start
	scale over time
	
	transparency start
	transparency over time
	
	color start
	color over time
	
	spritesheet animation frame duration (yes, these particles can be animated!)
	
	