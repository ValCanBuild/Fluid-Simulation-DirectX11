Fluid-Simulation-DirectX11
==========================

Codebase for my Honours Project: Adaptive Fluid Simulation using DirectX 11

Trello board for this project: https://trello.com/b/0ueEoeJs/honours-project-board

This will feature different experiments of simulating fluid mechanics in a 2D or 3D environment and seeing how to algorithms
can be adapted to be used in Real-time. As the project progresses so will this description. 

This project requires:   
Windows 8 SDK: http://msdn.microsoft.com/en-us/windows/desktop/aa904949             
DirectX 11: Part of the Windows SDK    
DirectXTK (provided): http://directxtk.codeplex.com/    
AntTweakBar: http://anttweakbar.sourceforge.net/

Changelog so far:

v1.1.1: Implemented obstacles for 2D Fluid Simulation. Slightly improved performance by reducing floating point
precision for Textures to 16-bits. Changed advection calculation to use the MacCormack scheme.

v1.1: 2D Fluid Simulation now running on Compute Shaders. Added AntTweakBar to the project in order to edit simulation
parameters at runtime.

v1.0.1: Implemented proper boundary conditions for 2D Fluid Simulation, changed 2D Wave Equation simulation 
to use DirectCompute. Added simple UI overlay for basic information displaying.

v1.0.0: Initial working version of 2D Fluid Simulation with small interactivity. 
