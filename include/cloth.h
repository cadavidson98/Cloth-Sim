#include <string>

#include <glm/glm.hpp>
#include <glad/glad.h>

class Cloth {
public:
	Cloth(int num_ropes, int num_columns, float k, float kv, float mass,
		float rest_length, float drag_coef, float start_x, float start_y, float z_val);

	~Cloth();

	void initGL(GLuint shader);
	
	void LockNode(int x, int y, bool skip);

	void Update(float dt);

	void Draw();

private:

	int pts_per_rope_;
	int num_ropes_;
	int num_pts_;
	
	float k_;
	float kv_;
	float mass_;
	float rest_length_;
	float drag_coef_;

	glm::vec3 air_res_;
	glm::vec3 *cloth_pts_;
	glm::vec3 *vels_;
	glm::vec3 *accels_;

	bool *lock_;

	glm::vec3 *norms_;
	glm::vec3 *tans_;
	glm::vec2 *uvs_;
	unsigned int *indices_;

	// Rendering info- includes the mesh, indices, and textures
	int num_tris_;
	GLuint cloth_vao_;
	GLuint mesh_buffer_;
	GLuint index_buffer_;
	GLuint diffuse_map_;
	GLuint normal_map_;

	glm::vec3* calcForces();
	void calcVertexNormals();
	void calcVertexTangents();
	void loadTexture(std::string file_name, GLuint *texture);
	void updateGPU();
};