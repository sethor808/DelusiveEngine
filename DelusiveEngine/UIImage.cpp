#include "UIImage.h"
#include "Renderer.h"
#include <imgui/imgui.h>
#include <glm/glm.hpp>
#include <glm/ext/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/vec4.hpp>
#include <filesystem>

float uiVertices[] = {
	// pos       // tex
	-0.5f, -0.5f,  0.0f, 0.0f,
	 0.5f, -0.5f,  1.0f, 0.0f,
	 0.5f,  0.5f,  1.0f, 1.0f,

	 0.5f,  0.5f,  1.0f, 1.0f,
	-0.5f,  0.5f,  0.0f, 1.0f,
	-0.5f, -0.5f,  0.0f, 0.0f
};

UIImage::UIImage()
	: UIElement("UIImage", { 0,0 }), size({ 1,1 })
{
	Init();
}

UIImage::UIImage(const glm::vec2& pos, const glm::vec2& siz)
	: UIElement("UIImage", pos), size(siz)
{
	Init();
}

UIImage::~UIImage() {
	glDeleteBuffers(1, &VBO);
	glDeleteVertexArrays(1, &VAO);
	delete shader;
	delete texture;
}

void UIImage::Init() {
	name = "UIImage";
	shader = new Shader("shaders/ui.vert", "shaders/ui.frag");
	texturePath = "assets/sprites/star.jpg";
	texture = new Texture("assets/sprites/star.jpg");

	glGenVertexArrays(1, &VAO);
	glGenBuffers(1, &VBO);

	glBindVertexArray(VAO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(uiVertices), uiVertices, GL_STATIC_DRAW);

	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);

	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));
	glEnableVertexAttribArray(1);

	glBindVertexArray(0);
}

std::unique_ptr<UIElement> UIImage::Clone() const{
	auto copy = std::make_unique<UIImage>(position, size);
	copy->SetName(name);
	copy->SetEnabled(enabled);
	copy->SetPosition(position);
	copy->SetSize(size);
	copy->SetTexturePath(texturePath);

	for (const auto& child : children) {
		copy->AddChild(std::move(child->Clone()));
	}

	return copy;
}

void UIImage::SetTexturePath(const std::string& path) {
	if (texturePath == path) return;

	texturePath = path;

	if (texture) {
		delete texture;
		texture = nullptr;
	}

	texture = new Texture(texturePath.c_str());

	if (!texture) {
		std::cerr << "[UIImage] Failed to load texture: " << path << std::endl;
	}
	else {
		std::cout << "[SpriteComponent] Texture set to: " << path << std::endl;
	}
}

void UIImage::SetShader(Shader* s) {
	shader = s;
}

void UIImage::Draw(const glm::mat4& projection) {
	if (!enabled || !texture || !shader) return;

	glm::vec2 worldPos = position / Renderer::GetPixelScale();
	glm::vec2 worldSize = size * Renderer::GetPixelScale();

	glm::mat4 model = glm::translate(glm::mat4(1.0f), glm::vec3(position, 0.0f)) *
		glm::scale(glm::mat4(1.0f), glm::vec3(worldSize, 1.0f));

	shader->Use();

	shader->SetMat4("model", glm::value_ptr(model));
	shader->SetMat4("projection", glm::value_ptr(projection));

	glActiveTexture(GL_TEXTURE0);
	texture->Bind();

	GLint loc = glGetUniformLocation(shader->GetID(), "tex");
	if (loc < 0) {
		std::cerr << "[Error] Uniform 'tex' not found in shader." << std::endl;
	}
	else {
		glUniform1i(loc, 0);
	}	

	glBindVertexArray(VAO);
	glDrawArrays(GL_TRIANGLES, 0, 6);
	glBindVertexArray(0);

	GLenum err = glGetError();
	
	if (err != GL_NO_ERROR) {
		std::cerr << "[UIImage::Draw] OpenGL error: " << err << std::endl;
		std::cerr << "Verbos: " << glewGetErrorString(err) << std::endl;
	}

	for (auto& child : children) {
		child->Draw(projection);
	}
}

void UIImage::DrawImGui() {
	ImGui::DragFloat2("Position", &position.x, 1.0f);
	ImGui::DragFloat2("Size", &size.x, 1.0f);

	char buffer[256];
	ImGui::Text("Texture: %s", std::filesystem::path(texturePath).filename().string().c_str());
	if (ImGui::Button("Change Texture")) {
		ImGui::OpenPopup("TextureBrowser");
	}

	if (ImGui::BeginPopup("TextureBrowser")) {
		std::function<void(const std::filesystem::path&)> DrawDirectory;
		DrawDirectory = [&](const std::filesystem::path& path) {
			for (const auto& entry : std::filesystem::directory_iterator(path)) {
				if (entry.is_directory()) {
					if (ImGui::BeginMenu((entry.path().filename().string() + "/").c_str())) {
						DrawDirectory(entry.path()); // recursive submenu
						ImGui::EndMenu();
					}
				}
				else if (entry.is_regular_file()) {
					std::string ext = entry.path().extension().string();
					std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);
					if (ext == ".png" || ext == ".jpg" || ext == ".jpeg") {
						std::string fullPath = entry.path().string();
						std::string filename = entry.path().filename().string();
						if (ImGui::Selectable(filename.c_str())) {
							texturePath = fullPath;
							delete texture;
							texture = new Texture(fullPath.c_str());
							ImGui::CloseCurrentPopup();
						}
					}
				}
			}
			};

		DrawDirectory("assets/sprites");

		ImGui::EndPopup();
	}
}

void UIImage::Serialize(std::ostream& out) const {
	out << "UIImage " << name << "\n";
	out << "pos " << position.x << " " << position.y << "\n";
	out << "size " << size.x << " " << size.y << "\n";
	out << "---\n";
	for (auto& child : children) {
		child->Serialize(out);
	}
}

void UIImage::Deserialize(std::istream& in) {
	std::string token;
	while (in >> token) {
		if (token == "---") break;
		else if (token == "pos") in >> position.x >> position.y;
		else if (token == "size") in >> size.x >> size.y;
	}
	for (auto& child : children) {
		child->Deserialize(in);
	}
}

const std::string& UIImage::GetTypeName() const {
	static std::string type = "UIImage";
	return type;
}