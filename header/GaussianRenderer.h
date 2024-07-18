#include "Window.h"
#include "Vertex.h"

class GaussianRenderer : public Window {
public:
	GaussianRenderer(LPCTSTR WindowName, int width, int height, bool fullScreen, HINSTANCE hInstance, int nShowCmd, const std::vector<Vertex>& vertices, const std::vector<Vertex>& quads);
	

	void draw() override;
	
	std::vector<Vertex> prepareTriangle();
  std::vector<VertexPos> prepareIndices(const std::vector<Vertex>& vertices);

	// Setter method for vertices
	void setVertices(const std::vector<Vertex>& vertices);

	// Getter for vertices
	const std::vector<Vertex>& getVertices() const; // Const method does not modify the object

	void setQuadVertices(const std::vector<Vertex>& vertices);
	
	const std::vector<Vertex>& getQuadVertices(); // Const method does not modify the object

	std::vector<Vertex> quadVertices;
	std::vector<Vertex> m_quadVertices;

private:
	// Member variable to store vertices
	std::vector<Vertex> m_vertices;
	std::vector<Vertex> m_quads;

};