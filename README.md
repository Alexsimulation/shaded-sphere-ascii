# shaded-sphere-ascii

![result](sphere_gif.gif)

## What?
Shaded sphere shaped C code that generates a shaded sphere.

## How?
Let's start with the general purpose of this project: to show a sphere with a moving light source, moving perspective camera, using ascii characters for light levels. We'll dive into each part of the code to see how the math works.

### Setup
For each pixel (characters on the command window), we send a ray in a specific direction. Where the ray lands defines which character we should choose for it. To define that direction, we first start with a constant position vector `oc` for the base of the camera, which is also the base of the pixel ray.
```C
float oc[3] = {-1.2, 0.5, 0.7};
```
The above variable is a three dimensional vector that stores the `{x,y,z}` position of the origin point of the camera. The next line allows the camera to change perspective at each step. `k` is a variable that steps up with each frame rendered. We move the camera from its base position in a sinusoidal pattern in the x dimension.
```C
oc[0] += sin(k/10);
```
After that, we define a variable position vector `o` to store the end position of the camera direction at the pixel `i,j`.
```C
float o[3] = {0, ((float)j)/39, 1-((float)i)/39}; 
```
Why use two points? It's easier to visualise than working with angles or components of a direction vector, and the pixel ray needs an origin point anyway. We then compute the direction vector `u` to get the ray direction.
```C
float u[3] = {o[0]-oc[0], o[1]-oc[1], o[2]-oc[2]};
```
The above is simply `u = o - oc`, where all variables are vectors. The lines right after that in the code are just there to normalize the vector `u`, we want the length of the direction vector `u` to be 1, it simplifies things later.

The camera is now defined, we can move to the sphere. It's simple to define its geometry and position, we only need a center point and a radius.
```C
const float c[3] = {1, 0.5, 0.5};
const float r = 0.6;
```
With the sphere defined, we can move to the light source definition. This code uses a global light direction, like the sun, all light rays are in the same direction. To define the direction, we use a three dimensional vector `l`.
```C
float l[3] = {cos(k/10)*(float)sin(1.1), sin(k/10)*(float)sin(1.1), cos(1.1)};
```
We use the frame step variable `k` to change the angle of the light source over time, to get the desired animation.

After all that, we need to actually compute the ray-sphere intercept. To do that, we'll use [wikipedia](https://en.wikipedia.org/wiki/Line%E2%80%93sphere_intersection). The formula used is the following equation (1):
```
(1) d = -u*(o-c) +/- sqrt(delta)
```
It's basically just a quadratic equation. We get from this `d`, the distance between the origin `o` of the ray and the point on the surface of the sphere. The square root is computer intensive, since it requires an iterative process. The parameter `delta` is key to finding if the ray does actually intercept the sphere. If `d=0`, the line just touches the sphere at one point, they're tangent. In practice this will probably never happen with floating point numbers, so the only condition to check is `delta > 0`. That means the square root of `delta` is real, and the ray enters the sphere.

To simplify computations, we use two parameters, A and B. In vector form, the two equations that define A and B are:
```
A = u*(o-c)
B = |o-c|^2 = dot(o-c,o-c)
```
Where `|x|` is the norm of `x` and `dot(a,b)` is the dot product of the two inputs. From these parameters, delta can be computed with the following code:
```C
float delta = A*A - (B - r*r);
```
If `delta > 0`, the ray intercepts the sphere, we can then compute the intersection point with:
```C
d = -1*sqrt(delta) - A;
```
Equation (1) return two values, but we only use the smaller value of `d`. The intercept closer to the camera will always be the one we want to render, since the sphere has no transparency. After that, we can compute the point on the sphere using:
```
(2) p = o + d*u
```
That just means that `p` is the start point `o` plus a distance `d` in the ray direction `u`. We know where the sphere point viewed by the camera is, we can then compute an illuminance value for the pixel. For that, the intuitive way to think about it is: if the surface is perpendicular to the light source, we want a high illuminance value. If the surface is in the opposite way, we want no illuminance. If the surface is in between, we want a value in between. The mathematical way to do this is to look at the difference in direction between the normal surface vector at the point and the light source direction. The fast way to compute this is the dot product of the normal vector and the light direction vector.

To compute the normal vector, we use the fact that the normal vector to a sphere always points away from its center. We can easily compute the normal vector with:
```
n = (p-c)/r
```
The illuminance value is between 0 and 1. We get it with the following equation:
```
sqrt(I) = [dot(n,l)+1]/2
```
And here we go, we have the illuminance value to display at this pixel.

### Wait, what if the ray doesn't touch the sphere?
Good question, we need to compute the floor intersection. To do that, we also use [wikipedia](https://en.wikipedia.org/wiki/Line%E2%80%93plane_intersection). The equation to use, in vector form, is:
```
d = dot(o - a, n) / dot(u, n)
```
Since the normal vector of the xy plane is `n = {0, 0, 1}`, and a point `a` on the plane is `a = {0, 0, 0}`, the equation simplifies to:
```C
d = -1*o[2]/u[2];
```
We can get the position vector of the intercept point between the ray and the plane with equation (2) presented above. We need to check if the distance is positive, and that the x coordinate of the point is smaller than a predefined length (the floor is not infinite). If the distance is positive, the point is in front of the camera. 

We then do another sphere interception, with the starting point being the point on the plane, and the direction being the light direction. If we find an interception, that means the sphere is casting a shadow on this point. To compute this, we repeat the `A, B, delta` steps explained above. If we find an interception, the floor lighting will depend on the value of delta, since the square root of delta gives the half distance between the two intercepts on the sphere. If `sqrt(delta)` is large, we are on a point close to the center of the sphere, so the sphere should cast a stronger shadow on the floor. 

We start with an illuminance value of `I = 0.4`. Then, if the sphere is casting a shadow at the point, we multiply that value with a function of `delta`:
```C
I *= (1 - sqrt(delta)/(2*r))*(1 - sqrt(delta)/(2*r));
```
It scales down delta depending on how close the ray and the center of the sphere are.

### And that's all?
Yeah, that's all the meat of it. We convert the illuminance value to a 1 to 12 index for the 13 ascii characters to choose from, and we use that character for the given pixel.
```
// ASCII shading characters
const char asc[] = {' ','.',',',':',';','<','i','I','E','X','%','@'};
const int asclen = 12;

// Convert illumination value to ascii character
char S = asc[ (int)(I*asclen) ];
```
We loop over all the pixels, and then step up `k` to render the next frame, and that's all.

## Why?
don't ask
