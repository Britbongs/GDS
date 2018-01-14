#include <iostream>


#include <SFML\Graphics.hpp>
#include <Maths\KMaths.hpp>

#include "Krawler.h"

using namespace std;
using namespace sf;

#define GridNodeSize 1
Texture bg;

class Grid : public Drawable
{
public:
	Grid(int width, int height)
		: m_width(width), m_height(height)
	{

		m_vertArray.setPrimitiveType(Quads);
		m_vertArray.resize(width * height * 4);

		for (int i = 0; i < width; ++i)
		{
			for (int j = 0; j < height; ++j)
			{
				Vertex* vert = &m_vertArray[(i + j * width) * 4];
				vert[0].position = Vector2f(i * GridNodeSize, j * GridNodeSize);
				vert[1].position = Vector2f((i + 1)* GridNodeSize, j * GridNodeSize);
				vert[2].position = Vector2f((i + 1) * GridNodeSize, (j + 1)* GridNodeSize);
				vert[3].position = Vector2f(i * GridNodeSize, (j + 1) * GridNodeSize);

				vert[0].color = Color::Magenta;
				vert[1].color = Color::Magenta;
				vert[2].color = Color::Magenta;
				vert[3].color = Color::Magenta;
				const float texSize = 8;
				vert[0].texCoords = Vector2f(0, 0);
				vert[1].texCoords = Vector2f(texSize, 0);
				vert[2].texCoords = Vector2f(texSize, texSize);
				vert[3].texCoords = Vector2f(0, texSize);
			}
		}

	}

	virtual void draw(RenderTarget& rTarget, RenderStates rStates) const override
	{
		rStates.texture = &bg;
		rTarget.draw(m_vertArray, rStates);
	}

	~Grid() = default;
private:

	VertexArray m_vertArray;

	int m_width;
	int m_height;
};

int main(void)
{
	Vector2u screenSize(1024, 576);
	srand(time(0));
	RenderWindow rw(VideoMode(screenSize.x, screenSize.y), "Heatmap Test");
	rw.setFramerateLimit(60);
	Grid grid(screenSize.x / GridNodeSize, screenSize.y / GridNodeSize);
	RectangleShape background(Vector2f(screenSize.x, screenSize.y));
	background.setFillColor(Color::White);
	background.setOutlineColor(Color::White);
	bg.loadFromFile("res//space2.png");
	bg.setRepeated(true);
	background.setTexture(&bg);
	background.setTextureRect(IntRect(0, 0, 256.0f * (screenSize.x / 256), 256.0f * (screenSize.y / 256)));

	Shader shader;
	shader.loadFromFile("res//vert.glsl", "res//frag.glsl");
	float rad = 32;
	constexpr int circleCount = 4;
	CircleShape circles[circleCount];
	Clock delta;
	for (auto& circle : circles)
	{
		circle = CircleShape(rad);
		circle.setFillColor(Color::Green);

		circle.setPosition(rand() % screenSize.x, rand() % screenSize.y);
	}
	int scale = 100;//6.371e6 / rad;
	Vector2f centres[circleCount];
	float masses[circleCount];
	Glsl::Vec4 colours[circleCount];

	for (int i = 0; i < circleCount; ++i)
	{
		colours[i].x = (float)(rand() % 256) / 256.0f;
		colours[i].y = (float)(rand() % 256) / 256.0f;
		colours[i].z = (float)(rand() % 256) / 256.0f;
		colours[i].w = 1.0f;
		circles[i].setFillColor(Color(colours[i].x*256.0f, colours[i].y*256.0f, colours[i].z*256.0f));

	}

	for (float& m : masses)
	{
		m = Krawler::Maths::RandFloat(1.0e1f, 1.0e26f);
	}
	for (int i = 0; i < circleCount; ++i)
	{
		centres[i] = Vector2f(circles[i].getPosition() + Vector2f(rad, rad));
	}

	while (rw.isOpen())
	{
		float dt = delta.restart().asSeconds();
		Event evnt;
		while (rw.pollEvent(evnt))
		{
			if (evnt.type == Event::Closed)
			{
				rw.close();
			}
			if (evnt.type == Event::KeyPressed)
			{
				if (evnt.key.code == Keyboard::R)
				{
					for (int i = 0; i < circleCount; ++i)
					{
						circles[i].setPosition(rand() % screenSize.x, rand() % screenSize.y);
						centres[i] = Vector2f(circles[i].getPosition() + Vector2f(rad, rad));
						masses[i] = Krawler::Maths::RandFloat(1.0e10f, 1.0e26f);
						colours[i].x = (float)(rand() % 256) / 256.0f;
						colours[i].y = (float)(rand() % 256) / 256.0f;
						colours[i].z = (float)(rand() % 256) / 256.0f;
						colours[i].w = 1.0f;
						circles[i].setFillColor(Color(colours[i].x*256.0f, colours[i].y*256.0f, colours[i].z*256.0f));
						cout << masses[i] << " kg" << endl;
					}


					shader.loadFromFile("res//vert.glsl", "res//frag.glsl");

				}

				if (evnt.key.code == Keyboard::Up)
				{
					scale *= 1.25f;
					cout << scale << endl;
				}
				if (evnt.key.code == Keyboard::Down)
				{
					scale /= 1.25f;
					cout << scale << endl;
				}

			}
		}
		shader.setUniform("scale", (float)scale);
		shader.setUniformArray("planetPos", centres, circleCount);
		shader.setUniformArray("masses", masses, circleCount);
		shader.setUniformArray("colours", colours, circleCount);
		rw.clear();
		//rw.draw(background, &shader);
		rw.draw(grid, &shader);
		for (auto& circle : circles)
			rw.draw(circle);
		rw.display();
	}

	return 0;
}
