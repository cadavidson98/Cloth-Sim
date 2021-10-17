# Cloth-Sim
This cloth simulation was an assignment for the course *Animation and Planning in Games*. This project uses Partial Differential Equations and Mass-Spring systems to simulation cloth shape and movement. For an overview of this simulation's features, please visit [my website](https://cadavidson98.github.io/).

## Simulation Parameters
Creating a cloth requires a number of parameters, these are outlined below:
- k, The Spring Constant. This defines the cloth's stiffness/elasticity.
- kv, The velocity Constant. This defines the cloth's stiffness/elasticity when in motion.
- mass, The mass of each cloth point in the mass-spring system. This defines the weight of the cloth.
- rest length, the ideal distance between 2 cloth points in the mass-spring system. This defines the size of the cloth.
- drag coefficient, the strength of air resistance. This defines how the cloth should deccelerate when moving.

The simulation by default constructs a cloth as a planar piece of fabric. To control the cloth's size and location, use the following arguments:
- num ropes, the number of cloth points in the x dimension.
- num columns, the number of cloth points in the y dimension.
- start_x, the x coordinate of the first cloth point.
- start_y, the y coordinate of the first cloth point.
- start_z, the z coordinate of the first cloth point.

## Simulation Update
Updating the simulation consists of 2 steps, updating the cloth simulation and updating the cloth model. To update the cloth simulation, use the Update() method. Update() takes one argument, dt, the time elapsed since the last call to Update(). Use too large of a dt can cause numerical instability, which is why this simulation uses the Improved Euler's Method to update cloth points. Improved Euler's Method is a 2nd order Integrator, which allows the simulation to use much larger timesteps than a 1st order Integrator. As a result, the cloth simulation runs in real time.

Updating the cloth model is handled by the Update() method. Update will regenerate all the cloth points, normal, and tangent vectors and repopulate the OpenGL buffers.

## Simulation Rendering
Cloth Rendering is accomplished using OpenGL 3.2+. Cloth Rendering is done in 2 steps. First, after creating the cloth the InitGL() must be used to initialize all the OpenGL buffers. This method takes one argument, a shader variable. InitGL will query the shader for the following variables:
- vertex position
- vertex normal
- vertex tangent
- vertex texture coordinate
- diffuse map
- normal map

After calling InitGL(), drawing can be accomplished using Draw(). Draw() uses the currently bound shader to render the cloth, assuming the shader properly defines the aformentioned variables.
