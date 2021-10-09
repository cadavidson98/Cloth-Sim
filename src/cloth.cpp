#include "cloth.h"

#include <memory>
#include <fstream>
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#include "config.h"

/**
 * Create a cloth simulation using the provided force constants. By default, this constructor intializes the
 * cloth as a rectangular piece of fabric with num_ropes points in the x direction and num_columns points
 * in the y direction.
 */ 
Cloth::Cloth(int num_ropes, int num_columns, float k, float kv, float mass,
	float rest_length, float drag_coef, float start_x, float start_y, float z_val) : num_ropes_(num_ropes),
	pts_per_rope_(num_columns), k_(k), kv_(kv), mass_(mass), rest_length_(rest_length), drag_coef_(drag_coef) {
	// Create the mesh
	air_res_ = glm::vec3(0, 0, 0);
	num_pts_ = num_ropes * pts_per_rope_;
	cloth_pts_ = new glm::vec3[num_pts_];
	vels_ = new glm::vec3[num_pts_];
	accels_ = new glm::vec3[num_pts_];
	lock_ = new bool[num_pts_];
	norms_ = new glm::vec3[num_pts_];
	tans_ = new glm::vec3[num_pts_];
	uvs_ = new glm::vec2[num_pts_];
	indices_ = new unsigned int[6 * (num_ropes_ - 1) * (pts_per_rope_ - 1)];
	
	for (int j = 0; j < num_ropes_; j++) {
		for (int i = 0; i < pts_per_rope_; i++) {
			int index = j * pts_per_rope_ + i;
			cloth_pts_[index] = glm::vec3(start_x + j * rest_length_, start_y, z_val - i * rest_length);
			vels_[index] = glm::vec3(0, 0, 0);
			accels_[index] = glm::vec3(0, 0, 0);
			norms_[index] = glm::vec3(0, 0, -1);
			tans_[index] = glm::vec3(-1, 0, 0);
			lock_[index] = false;
			uvs_[index] = glm::vec2(j / (float)pts_per_rope_, i / (float)pts_per_rope_);
		}
	}
	// Create the cloth mesh, this means calculates the normals and setting
	// the triangle indices
	int index = 0;
	for (int j = 0; j < num_ropes_ - 1; j++) {
		for (int i = 0; i < pts_per_rope_ - 1; i++) {
			int index1 = j * pts_per_rope_ + i;
			int index2 = j * pts_per_rope_ + i + 1;
			int index3 = (j + 1) * pts_per_rope_ + i;
			int index4 = (j + 1) * pts_per_rope_ + i + 1;
			indices_[index++] = (index1);
			indices_[index++] = (index3);
			indices_[index++] = (index2);

			indices_[index++] = (index2);
			indices_[index++] = (index3);
			indices_[index++] = (index4);
		}
	}
	num_tris_ = 2 * (num_ropes_ - 1) * (pts_per_rope_ - 1);
	calcVertexNormals();
	calcVertexTangents();
}

Cloth::~Cloth() {
	delete [] cloth_pts_;
	delete [] vels_;
	delete [] accels_;
	delete [] lock_;

	delete [] norms_;
	delete [] tans_;
	delete [] uvs_;
	delete [] indices_;
	
	glDeleteVertexArrays(1, &cloth_vao_);
	glDeleteBuffers(1, &mesh_buffer_);
	glDeleteBuffers(1, &index_buffer_);
	glDeleteTextures(1, &diffuse_map_);
	glDeleteTextures(1, &normal_map_);
}

/**
 * Intialize the OpenGL buffers used for cloth rendering. This method assumes the argument shader contains the following
 * variables:
 * vertex input variable
 * normal input variable
 * tangent input variable
 * tex_coord input variable
 * 
 * diffuse_map uniform sampler
 * normal_map uniform sampler
 */ 
void Cloth::initGL(GLuint shader) {
    glGenVertexArrays(1, &cloth_vao_);
    glBindVertexArray(cloth_vao_);
	
	glGenBuffers(1, &mesh_buffer_);
	updateGPU();
	glBindBuffer(GL_ARRAY_BUFFER, mesh_buffer_);
	size_t vec3_size = sizeof(glm::vec3);
	GLsizeiptr pts_size = num_pts_ * vec3_size;
	GLsizeiptr norm_size = pts_size;
	GLsizeiptr tans_size = pts_size;
	GLsizeiptr uv_size = num_pts_ * sizeof(glm::vec2);
	GLsizeiptr tot_size = pts_size + norm_size + tans_size + uv_size;
	
	GLint pos_attrib = glGetAttribLocation(shader, "vertex");
  	glVertexAttribPointer(pos_attrib, 3, GL_FLOAT, GL_FALSE, vec3_size, nullptr);
    glEnableVertexAttribArray(pos_attrib);

	GLint norm_attrib = glGetAttribLocation(shader, "normal");
	glVertexAttribPointer(norm_attrib, 3, GL_FLOAT, GL_FALSE, vec3_size, (void*)pts_size);
	glEnableVertexAttribArray(norm_attrib);

	GLint tan_attrib = glGetAttribLocation(shader, "tangent");
	glVertexAttribPointer(tan_attrib, 3, GL_FLOAT, GL_FALSE, vec3_size, (void*)(pts_size + norm_size));
	glEnableVertexAttribArray(tan_attrib);

	GLint uv_attrib = glGetAttribLocation(shader, "tex_coord");
	glVertexAttribPointer(uv_attrib, 2, GL_FLOAT, GL_FALSE, sizeof(glm::vec2), (void*)(pts_size + norm_size + tans_size));
	glEnableVertexAttribArray(uv_attrib);

	glGenBuffers(1, &index_buffer_);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, index_buffer_);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, 3 * num_tris_ * sizeof(unsigned int), indices_, GL_STATIC_DRAW);
	
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	// Load textures	
	loadTexture("fabric_diffuse.jpg", &diffuse_map_);
	loadTexture("fabric_normal.jpg", &normal_map_);
	
	// Set up textures in shader
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, diffuse_map_);
	
    glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, normal_map_);
	
	glUseProgram(shader);
	glUniform1i(glGetUniformLocation(shader, "diffuse_map"), 0);
	glUniform1i(glGetUniformLocation(shader, "normal_map"), 1);
    glUseProgram(0);
	
	glBindVertexArray(0);
}

/**
 * Enable/Disable the cloth point at the specified position. This effectively clamps a
 * point in place.
 */ 
void Cloth::LockNode(int x, int y, bool lock) {
	int index = x * pts_per_rope_ + y;
	lock_[index] = lock;
}

/**
 * Update the Cloth positions and Velocities using the Improved Euler Method
 */
void Cloth::Update(float dt) {
	// calculate the forces at the current timestep
	glm::vec3 *cur_forces = calcForces();
	// now integrate half a step into the future
	for(int i = 0; i < num_pts_; ++i) {
		if(lock_[i]) {
			continue;
		}
		vels_[i] += cur_forces[i] * 0.5f * dt / mass_;
		cloth_pts_[i] += vels_[i] * dt;
	}
	// now calculate the forces again
	glm::vec3 *new_forces = calcForces();
	for(int i = 0; i < num_pts_; ++i) {
		if(lock_[i]) {
			continue;
		}
		vels_[i] += new_forces[i] * 0.5f * dt / mass_;
	}
	delete [] cur_forces;
	delete [] new_forces;

	calcVertexNormals();
	calcVertexTangents();
	updateGPU();
}

/**
 * Draw the Cloth using OpenGL using the currently bound shader.
 */ 
void Cloth::Draw() {
	glBindVertexArray(cloth_vao_);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, index_buffer_);
	glDrawElements(GL_TRIANGLES, 3 * num_tris_, GL_UNSIGNED_INT, nullptr);
	glBindVertexArray(0);
}

/**
 * Update the Cloth Model on the GPU for OpenGL drawing. By default, this
 * method is called during Update().
 */ 
void Cloth::updateGPU() {
	glBindBuffer(GL_ARRAY_BUFFER, mesh_buffer_);
	size_t float_size = sizeof(float);
	size_t vec3_size = sizeof(glm::vec3);
	GLsizeiptr pts_size = num_pts_ * vec3_size;
	GLsizeiptr norm_size = num_pts_ * vec3_size;
	GLsizeiptr tans_size = num_pts_ * vec3_size;
	GLsizeiptr uv_size = num_pts_ * sizeof(glm::vec2);
	GLsizeiptr tot_size = pts_size + norm_size + tans_size + uv_size;
	
	glBufferData(GL_ARRAY_BUFFER, tot_size, NULL, GL_DYNAMIC_DRAW);
	glBufferSubData(GL_ARRAY_BUFFER, 0, pts_size, cloth_pts_);
	glBufferSubData(GL_ARRAY_BUFFER, pts_size, norm_size, norms_);
	glBufferSubData(GL_ARRAY_BUFFER, pts_size + norm_size, tans_size, tans_);
	glBufferSubData(GL_ARRAY_BUFFER, pts_size + norm_size + tans_size, uv_size, uvs_);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
}

/**
 * Calculate the cloth-fiber forces at each cloth point using Hooke's Law and Rayleigh Number.
 */ 
glm::vec3* Cloth::calcForces() {
	glm::vec3 *forces = new glm::vec3[num_pts_];
	for (int i = 0; i < num_pts_; ++i) {
		forces[i] = glm::vec3(0, -.1, 0);
	}

	for (int j = 0; j < num_ropes_; j++) {
		// calculate all the vertical spring forces
		for (int i = 0; i < pts_per_rope_ - 1; i++) {
			int cur_pt = j * pts_per_rope_ + i;
			int lower_index = cur_pt + 1;
			glm::vec3 diff = cloth_pts_[cur_pt] - cloth_pts_[lower_index];
			float string_force = -k_ * (glm::length(diff) - rest_length_);

			// velocity stuff here...
			glm::vec3 string_dir = glm::normalize(diff);
			double proj_btm_pt = glm::dot(vels_[lower_index], string_dir);
			double proj_top_pt = glm::dot(vels_[cur_pt], string_dir);
			float damp_force = -kv_ * (proj_top_pt - proj_btm_pt);

			// combine it all together
			glm::vec3 force = string_dir * (string_force + damp_force);
			forces[cur_pt] = forces[cur_pt] + force;
			forces[lower_index] = forces[lower_index] + force * (-1.0f);
		}
	}

	for (int j = 0; j < pts_per_rope_; j++) {
		// calculate all the horizontal spring forces
		for (int i = 0; i < num_ropes_ - 1; i++) {
			int cur_pt = j + pts_per_rope_ * i;
			int right_index = j + (pts_per_rope_ * (i + 1));
			glm::vec3 diff = cloth_pts_[cur_pt] - cloth_pts_[right_index];
			float string_force = -k_ * (glm::length(diff) - rest_length_);

			// velocity stuff here...
			glm::vec3 string_dir = glm::normalize(diff);
			float proj_btm_pt = glm::dot(vels_[right_index], string_dir);
			float proj_top_pt = glm::dot(vels_[cur_pt], string_dir);
			float damp_force = -kv_ * (proj_top_pt - proj_btm_pt);

			// combine it all together
			glm::vec3 force = string_dir * (string_force + damp_force);
			forces[cur_pt] = forces[cur_pt] + force;
			forces[right_index] = forces[right_index] + force * (-1.0f);
		}
	}

	// calculate air drag - this means iterating over triangles
	for (int j = 0; j < num_ropes_ - 1; j++) {
		for (int i = 0; i < pts_per_rope_ - 1; i++) {
			int index1 = j * pts_per_rope_ + i;
			int index2 = j * pts_per_rope_ + i + 1;
			int index3 = (j + 1) * pts_per_rope_ + i;
			int index4 = (j + 1) * pts_per_rope_ + i + 1;

			// start with one triangle
			glm::vec3 avg_vel = (vels_[index1] + vels_[index2] + vels_[index3]) / 3.0f - air_res_;
			glm::vec3 normal = glm::cross(cloth_pts_[index1] - cloth_pts_[index2], cloth_pts_[index3] - cloth_pts_[index2]);
			float v_a_n = glm::length(avg_vel) * (glm::dot(avg_vel, normal)) * 0.5f;
			normal = glm::normalize(normal);
			glm::vec3 res = -0.5f * drag_coef_ * v_a_n * normal;
			// now divide by 3, then apply to each particle
			res = res / 3.0f;
			forces[index1] = forces[index1] + res;
			forces[index2] = forces[index2] + res;
			forces[index3] = forces[index3] + res;

			// now do the other triangle
			avg_vel = (vels_[index2] + vels_[index4] + vels_[index4]) / 3.0f;
			normal = glm::cross(cloth_pts_[index2] - cloth_pts_[index4], cloth_pts_[index3] - cloth_pts_[index4]);
			v_a_n = glm::length(avg_vel) * glm::dot(avg_vel, normal) * 0.75f;
			normal = glm::normalize(normal);
			res = -0.5f * drag_coef_ * v_a_n * normal;
			// now divide by 3, then apply to each particle
			res = res / 3.0f;
			forces[index2] = forces[index2] + res;
			forces[index4] = forces[index4] + res;
			forces[index3] = forces[index3] + res;
		}
	}
	
	return forces;
}

/**
 * Calculate the normal vector at each cloth point in the cloth model. These are used for illuminating
 * the cloth in OpenGL rendering.
 */ 
void Cloth::calcVertexNormals() {
	// reset each normal to 0
	for(int i = 0; i < num_pts_; ++i) {
		norms_[i] = glm::vec3(0, 0, 0);
	}
	// iterate through each face and accumulate the normals for each vertex
	for(int i = 0; i < num_tris_; ++i) {
		int idx1 = indices_[3 * i];
		int idx2 = indices_[3 * i + 1];
		int idx3 = indices_[3 * i + 2];
		glm::vec3 face_normal = glm::cross(cloth_pts_[idx2] - cloth_pts_[idx1], cloth_pts_[idx3] - cloth_pts_[idx1]);
		norms_[idx1] = norms_[idx1] + face_normal;
		norms_[idx2] = norms_[idx2] + face_normal;
		norms_[idx3] = norms_[idx3] + face_normal;
	}

	for(int i = 0; i < num_pts_; ++i) {
		norms_[i] = glm::normalize(norms_[i]);
	}
}

void Cloth::calcVertexTangents() {
	for (int i = 0; i < num_pts_; ++i) {
		tans_[i] = glm::vec3(0, 0, 0);
	}

	for (int i = 0; i < num_tris_; ++i) {
		int idx1 = indices_[3 * i];
		int idx2 = indices_[3 * i + 1];
		int idx3 = indices_[3 * i + 2];

		glm::vec3 edge1 = cloth_pts_[idx3] - cloth_pts_[idx1];
		glm::vec3 edge2 = cloth_pts_[idx2] - cloth_pts_[idx1];
		glm::vec2 uv1 = uvs_[idx3] - uvs_[idx1];
		glm::vec2 uv2 = uvs_[idx2] - uvs_[idx1];
		glm::vec3 tangent;
		float f = 1.0f / (uv1.x * uv2.y - uv2.x * uv1.y);

		tangent.x = f * (uv2.y * edge1.x - uv1.y * edge2.x);
		tangent.y = f * (uv2.y * edge1.y - uv1.y * edge2.y);
		tangent.z = f * (uv2.y * edge1.z - uv1.y * edge2.z);
		
		tans_[idx1] = tans_[idx1] + tangent;
		tans_[idx2] = tans_[idx2] + tangent;
		tans_[idx3] = tans_[idx3] + tangent;
	}

	for (int i = 0; i < num_pts_; ++i) {
		tans_[i] = glm::normalize(tans_[i]);
	}
}

/**
 * Load a texture to use as the cloth material. This texture is displayed during OpenGL rendering.
 */ 
void Cloth::loadTexture(std::string file_name, GLuint *texture) {
	int width, height, num_components;
	// try both the debug and install directories to find the file
    // start with debug
    std::string full_file_path = DEBUG_DIR + std::string("/") + file_name;
	std::ifstream img_file(full_file_path);
    if(!img_file.good()) {
		full_file_path = INSTALL_DIR + std::string("/") + file_name;
        img_file.open(full_file_path);
        if(!img_file.good()) {
            exit(1);
        }
    };
	img_file.close();
    unsigned char* img_data = stbi_load(full_file_path.c_str(), &width, &height, &num_components, 0);
    glGenTextures(1, texture);
    glBindTexture(GL_TEXTURE_2D, *texture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    GLuint img_type = (num_components == 4) ? GL_RGBA : GL_RGB;
	glTexImage2D(GL_TEXTURE_2D, 0, img_type, width, height, 0, img_type, GL_UNSIGNED_BYTE, img_data);
    glBindTexture(GL_TEXTURE_2D, 0);
    stbi_image_free(img_data);
}