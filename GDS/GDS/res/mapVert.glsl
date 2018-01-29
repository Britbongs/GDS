#version 120
#define PLANET_COUNT 4
#define SCALE 10000000

uniform float maxG;
uniform vec2 planetPos[PLANET_COUNT];
uniform vec4 colours[PLANET_COUNT];
//uniform float scale;
const float G = 6.67e-11;

vec4 toRGB(int r, int g, int b)
{
	vec4 col;
	col.w = 1.0f;
	col.x = float(r) / 256.0f;
	col.y = float(g) / 256.0f;
	col.z = float(b) / 256.0f;
	return col;
}

const vec4 WEAK_GRAVITY_COL = vec4(0.0f, 0.0f, 0.0f, 1.0f);

void main()
{
	// transform the vertex position
	gl_Position = gl_ModelViewProjectionMatrix * gl_Vertex;

	// transform the texture coordinates
	gl_TexCoord[0] = gl_TextureMatrix[0] * gl_MultiTexCoord0;

	float t[PLANET_COUNT];

	for (int i = 0; i < PLANET_COUNT; ++i)
	{
		t[i] = 0.0f;
		float r = distance(gl_Vertex.xy, planetPos[i]);
		t[i] += (1.0f / (r*r));
		t[i] *= SCALE;
	}

	//for (int i = 0; i < PLANET_COUNT; ++i)
		//t[i] = clamp(t[i], 0.0f, 1.0f);

	vec4 col;
	for (int i = 0; i < PLANET_COUNT; ++i)
	{
		col += mix(WEAK_GRAVITY_COL, colours[i], t[i]);
	}
	vec4 normalizedColour = normalize(col);
	//normalizedColour *= ;
	gl_FrontColor = vec4(normalizedColour.x, normalizedColour.y, normalizedColour.z, 0.9f);
}
