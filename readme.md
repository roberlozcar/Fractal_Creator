# Fractal Creator

## Introduction

The Newton-Raphson method is usually used to find the roots of non-linear equation. But also, it can be used to create beautiful fractals. 

In this program, we use an OpenGL compute shader to calculate a root of a fourth-degree polynomial. This polynomial has four complex roots, counted with multiplicity, (due to the Fundamental Theorem of Algebra). But the Newton-Raphson method can only find one given starting point. So that we can paint an image where each pixel represents a starting point, its hue the root reached, and its luminosity the number of steps needed for reaching it.

We calculate the roots of the polynomial previously in CPU using the closed formula. The single precision of the GPU limits the quality and the number of self-similarities that can be achieved.

## Options

- In the console, you can input the five coefficients of the polynomial. At least, you must input a second-degree polynomial.
- You can change the resolution of the window and the resolution of the computation too.
- You can move and zoom the fractal using the mouse. It will be recalculated, so it is not like zooming in on a photo.
- You can change the gamma of the image with '+' and '-'.
- A screenshot can be done by pressing s and the view can be reset by pressing r.

## Installation

Build with CMake and move the bin dlls to the same folder as the compiled executable. 

You may have to move the shaders folder to the parent folder of the executable.

## Requirements

This project needs glm, glew, glut, and freeimage. The headers, libs, and dlls for win32 are included in this repository.

## Questions

Write an email to r.lozano93@gmail.com.
