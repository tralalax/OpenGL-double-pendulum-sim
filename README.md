# OpenGL Double Pendulum Simulation
A basic simulation of a double/chaotic pendulum using the RK4 method to solve the Lagrangian equation of motion, then rendered with OpenGL.
This method of simulation is the one commonly used for this kind of project ; feel free to look at ``computeRK4()`` and ``functionForRK4()`` if you only want the maths.

![2026-02-20 19-57-06](https://github.com/user-attachments/assets/d9b8c59b-114b-4ff4-8c19-cdadc118b150)

## notes :
- in order to run the .exe, you need to have glew32.dll present in your system or in the same directory as the .exe, you can download it [here](https://glew.sourceforge.net/install.html)  
- idk about linux
- I hope the code isn't too bad, it's my first time doing something in C++ and with OpenGL ^^'

## dependencies & library
- glew32
- glfw32
- glm
