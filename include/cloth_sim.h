#include "mingfx.h"
#include "cloth.h"
#include "water.h"
#include "multi_mesh.h"

using namespace mingfx;

class cloth_sim : public GraphicsApp {
public:
	cloth_sim();

	void InitOpenGL();

	void UpdateSimulation(double dt);

	void DrawUsingOpenGL();

	void DrawCloth();

	void OnRightMouseDrag(const Point2& pos, const Vector2& delta);

	void LoadShaderAndTextures();

	void OnSpecialKeyDown(int key, int scancode, int modifiers);
private:
	
	Point3 camera_pos_;
	Matrix4 view_matrix_;
	Matrix4 proj_matrix_;
	QuickShapes quick_shapes_;

	Cloth sail_;
	Cloth sail2_;
	Cloth sail3_;
	Cloth sail4_;
	Cloth flag_;

	Water water_;

	Mesh cloth_;
	MultiMesh boat_;
	CraftCam camera_;

	DefaultShader d_shader_;

	Texture2D background_;
	Texture2D surface_tex_;
	Texture2D normal_tex_;
	std::vector<unsigned int> tri_IDs_;
	std::vector<Point3> verts_;

	bool paused_ = true;
	double tot_time = 0;
	int pts_per_rope_ = 10;
	int num_ropes_ = 10;
	
	double k_;
	double kv_;
	double mass_;
	double rest_length_;
	double drag_coef_;

	Point3 cloth_pts[100];
	Vector3 vels_[100];
	Vector3 accels_[100];

	Point3 ball_pos;
	double ball_r;

	std::vector<std::string> search_path_;
	ShaderProgram shader_;
};