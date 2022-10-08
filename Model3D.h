#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <string>
#include <iostream>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

class Model3D {
	private:
		float pos_x;
		float pos_y;
		float pos_z;
		float rot_x;
		float rot_y;
		float rot_z;
		float scl_x;
		float scl_y;
		float scl_z;
		unsigned int mesh_indices_size; // for drawing

	public:
		Model3D(glm::vec3 cameraPos, unsigned int mesh_indices_size, glm::vec3 cameraFront) {
			// the new model's position is based off of the camera's position and "center" (cameraFront)
			cameraPos = cameraPos + cameraFront;
			pos_x = cameraPos.x;
			pos_y = cameraPos.y;
			pos_z = cameraPos.z;
			rot_x = 0.f;
			rot_y = 0.f;
			rot_z = 0.f;
			scl_x = 1.f;
			scl_y = 1.f;
			scl_z = 1.f;
			this->mesh_indices_size = mesh_indices_size;
		}
		
		void draw(unsigned int transformLoc) {
			glm::mat4 transformation_matrix = glm::mat4(1.0f);
			transformation_matrix = glm::translate(transformation_matrix,
				glm::vec3(pos_x, pos_y, pos_z));
			// new uniform variable
			glUniformMatrix4fv(transformLoc, 1, GL_FALSE, value_ptr(transformation_matrix));
			glDrawElements(GL_TRIANGLES, mesh_indices_size, GL_UNSIGNED_INT, 0);
		}
};