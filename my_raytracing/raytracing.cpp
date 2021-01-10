#define _USE_MATH_DEFINES
#include <cmath>
#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "bvh.h"
#include "sphere.h"
#include "moving_sphere.h"
#include "hitable_list.h"
#include "material.h"
#include "camera.h"
#include "aarect.h"

vec3 color(const ray& r, hitable *world, int depth) {
	hit_record rec;
	if (world->hit(r, 0.001, FLT_MAX, rec)) {
		ray scattered;
		vec3 attenuation;
		vec3 emitted = rec.mat_ptr->emitted(rec.u, rec.v, rec.p);
		if (depth < 50 && rec.mat_ptr->scatter(r, rec, attenuation, scattered))
			return emitted + attenuation * color(scattered, world, depth + 1);
		else
			return emitted;
		}
	else {
		return vec3(0, 0, 0);
	}
}

hitable* random_scene() {
	int n = 50000;
	hitable** list = new hitable * [n + 1];
	list[0] = new sphere(vec3(0, -1000, 0), 1000, new lambertian(new constant_texture(vec3(0.5, 0.5, 0.5))));
	int i = 1;
	for (int a = -10; a < 10; a++) {
		for (int b = -10; b < 10; b++) {
			float choose_mat = (double)rand() / RAND_MAX;
			vec3 center(a + 0.9 * (double)rand() / RAND_MAX, 0.2, b + 0.9 * (double)rand() / RAND_MAX);
			if ((center - vec3(4, 0.2, 0)).length() > 0.9) {
				if (choose_mat < 0.8) {  // diffuse
					list[i++] = new moving_sphere(
						center, center+vec3(0,0.5* ((double)rand() / RAND_MAX),0), 0.0, 1.0, 0.2,
						new lambertian(new constant_texture(vec3(((double)rand() / RAND_MAX) * ((double)rand() / RAND_MAX),
							((double)rand() / RAND_MAX) * ((double)rand() / RAND_MAX),
							((double)rand() / RAND_MAX) * ((double)rand() / RAND_MAX))))
					);
				}
				else if (choose_mat < 0.95) { // metal
					list[i++] = new sphere(
						center, 0.2,
						new metal(vec3(0.5 * (1 + (double)rand() / RAND_MAX),
							0.5 * (1 + (double)rand() / RAND_MAX),
							0.5 * (1 + (double)rand() / RAND_MAX)),
							0.5 * (double)rand() / RAND_MAX)
					);
				}
				else {  // glass
					list[i++] = new sphere(center, 0.2, new dielectric(1.5));
				}
			}
		}
	}
	list[i++] = new sphere(vec3(0, 1, 0), 1.0, new dielectric(1.5));
	list[i++] = new sphere(vec3(-4, 1, 0), 1.0, new lambertian(new constant_texture(vec3(0.4, 0.2, 0.1))));
	list[i++] = new sphere(vec3(4, 1, 0), 1.0, new metal(vec3(0.7, 0.6, 0.5), 0.0));

	return new hitable_list(list, i);
}

hitable* cornell_box() {
	hitable** list = new hitable * [8];
	int i = 0;
	material* red = new lambertian(new constant_texture(vec3(0.65, 0.05, 0.05)));
	material* white = new lambertian(new constant_texture(vec3(0.73, 0.73, 0.73)));
	material* green = new lambertian(new constant_texture(vec3(0.12, 0.45, 0.15)));
	material* light = new diffuse_light(new constant_texture(vec3(15, 15, 15)));
	list[i++] = new flip_normals(new yz_rect(0, 555, 0, 555, 555, green));
	list[i++] = new yz_rect(0, 555, 0, 555, 0, red);
	list[i++] = new xz_rect(213, 343, 227, 332, 554, light);
	list[i++] = new flip_normals(new xz_rect(0, 555, 0, 555, 555, white));
	list[i++] = new xz_rect(0, 555, 0, 555, 0, white);
	list[i++] = new flip_normals(new xy_rect(0, 555, 0, 555, 555, white));
	return new hitable_list(list, i);
}

int main() {
	int nx = 800;
	int ny = 800;
	int ns = 100;
	std::cout << "P3\n" << nx << " " << ny << "\n255\n";
	//hitable* world = random_scene();
	hitable* world = cornell_box();
	vec3 lookfrom(278, 278, -800);
	vec3 lookat(278, 278, 0);
	float dist_to_focus = 10.0;
	float aperture = 0.0;
	float vfov = 40.0;
	camera cam(lookfrom, lookat, vec3(0,1,0), vfov, float(nx)/float(ny),aperture,dist_to_focus, 0.0, 1.0);
	for (int j = ny - 1; j >= 0; j--) {
		for (int i = 0; i < nx; i++) {
			vec3 col(0, 0, 0);
			for (int s = 0; s < ns; s++) {
				float u = float(i + (double)rand() / RAND_MAX) / float(nx);
				float v = float(j + (double)rand() / RAND_MAX) / float(ny);
				ray r = cam.get_ray(u,v);
				vec3 p = r.point_at_parameter(2.0);
				col += color(r, world, 0);
			}
			col /= float(ns);
			col = vec3(sqrt(col[0]), sqrt(col[1]), sqrt(col[2]));
			int ir = int(255.99 * col[0]);
			int ig = int(255.99 * col[1]);
			int ib = int(255.99 * col[2]);

			std::cout << ir << " " << ig << " " << ib << "\n";
		}
	}
}