// Commented version of shaded sphere
// by https://github.com/Alexsimulation
#include <stdio.h>
#include <math.h>
#include <stdlib.h>

char shade( int i, int j, float k ) {
    
    // Sphere and camera parameters
    float o[3] = {0, ((float)j)/39, 1-((float)i)/39}; 
    const float c[3] = {1, 0.5, 0.5};
	float oc[3] = {-1.2, 0.5, 0.7};
	oc[0] += sin(k/10);
    float u[3] = {o[0]-oc[0], o[1]-oc[1], o[2]-oc[2]};
	float un = 1/sqrt(u[0]*u[0] + u[1]*u[1] + u[2]*u[2]);
	u[0] *= un;
	u[1] *= un;
	u[2] *= un;
    const float r = 0.6;
	
	// Light source inverse direction
	float l[3] = {cos(k/10)*(float)sin(1.1), sin(k/10)*(float)sin(1.1), cos(1.1)};
	
	// ASCII shading characters
	const char asc[] = {' ','.',',',':',';','<','i','I','E','X','%','@'};
	const int asclen = 12;
    
    // Sphere-ray interception
    float A = u[0]*(o[0]-c[0]) + u[1]*(o[1]-c[1]) + u[2]*(o[2]-c[2]);
    float B = (o[0]-c[0])*(o[0]-c[0]) + (o[1]-c[1])*(o[1]-c[1]) + (o[2]-c[2])*(o[2]-c[2]);
    
    float delta = A*A - (B - r*r);
    float d, I;
	char S;
    if (delta > 0) {
		// Sphere intersection exists
        d = -1*sqrt(delta) - A;
		
		// Point on sphere
		float p[3] = {o[0] + d*u[0], o[1] + d*u[1], o[2] + d*u[2]};
		
		// Normal at point
		float n[3] = {(p[0]-c[0])/r, (p[1]-c[1])/r, (p[2]-c[2])/r};
		
		// Dot product of normal and light vector gives illumination value (0 to 1)
		I = (n[0]*l[0] + n[1]*l[1] + n[2]*l[2] + 1)/2;
		I = I * I;
		
		// Convert illumination value to ascii character
		S = asc[ (int)(I*asclen) ];
    } else {
		// No sphere intersect: do a floor intersect
		d = -1*o[2]/u[2]; // Distance where floor intercept
		
		// Point on plane
		float p[3] = {o[0] + d*u[0], o[1] + d*u[1], o[2] + d*u[2]};
		
		if ((p[0] < 2)&(d > 0)) {
			// Sphere-ray interception to see if sphere casts shadow
			A = l[0]*(p[0]-c[0]) + l[1]*(p[1]-c[1]) + l[2]*(p[2]-c[2]);
			B = (p[0]-c[0])*(p[0]-c[0]) + (p[1]-c[1])*(p[1]-c[1]) + (p[2]-c[2])*(p[2]-c[2]);
			delta = A*A - (B - r*r);
			
			I = 0.4;
			if (delta > 0) {
				// Sphere casts a shadow, function of delta
				I *= (1 - sqrt(delta)/(2*r))*(1 - sqrt(delta)/(2*r));
			}
			
		} else {
			I = 0;
		}
		
    }
	// Convert illumination value to ascii character
	S = asc[ (int)(I*asclen) ];
	
    return S;
}

void render() {
	char c[80];
	for (int k=0; k<100000; ++k) {
		system("cls");
		for (int i=0; i<40; ++i) {
			for (int j=0; j<40; ++j) {
				c[2*j] = shade(i,j,(float)k);
				c[2*j+1] = c[2*j];
			}
			c[80]=' ';
			printf("%s\n",c);
		}
	}
}

int main() { render(); return 0; }