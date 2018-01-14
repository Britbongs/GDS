// #version 110
#define PLANET_COUNT 4

uniform float maxG;
uniform float masses[PLANET_COUNT];
uniform vec2 planetPos[PLANET_COUNT];
uniform vec4 colours[PLANET_COUNT];
uniform float scale;
const float G = 6.67e-11;
//rgb(239, 31, 31)
vec4 toRGB(int r, int g, int b)
{
	vec4 col;
	col.w = 1.0f;
	col.x = float(r) / 256.0f;
	col.y = float(g) / 256.0f;
	col.z = float(b) / 256.0f;
	return col;
}

const vec4 STRONG_GRAVITY_COL = toRGB(211, 133, 38);//vec4(0.0f, 1.0f, 0.0f, 1.0f);
const vec4 WEAK_GRAVITY_COL = toRGB(0, 0, 0);
// const vec4 STRONG_GRAVITY_COL = toRGB(256,0,0);//vec4(0.0f, 1.0f, 0.0f, 1.0f);
// const vec4 WEAK_GRAVITY_COL = toRGB(0,256,0);

void main()
{
	// transform the vertex position
	gl_Position = gl_ModelViewProjectionMatrix * gl_Vertex;

	// transform the texture coordinates
	gl_TexCoord[0] = gl_TextureMatrix[0] * gl_MultiTexCoord0;

	//float rConvert = r* 1.0e5f;

	/*float g = (G * mass);
	g /= (rConvert*rConvert);
	float ratio =  g / maxG;
	ratio = clamp(ratio, 0.0f, 1.0f);
	*/
	float t[PLANET_COUNT];

	for (int i = 0; i < PLANET_COUNT; ++i)
	{
		if (i == 0)
		{
			t[i] = 0.0f;
		}

		float r = distance(gl_Vertex.xy, planetPos[i]);
		t[i] += 1.0f / (r*r);
		t[i] = clamp(t[i], 0.0f, 1.0f);
		t[i] *= scale;
	}

	for (int i = 0; i < PLANET_COUNT; ++i)
	{
		//for (int j = 0; j < PLANET_COUNT; ++j)
			//t[i] += (masses[i] * (1.0e-26));
	}

	vec4 col;
	for (int i = 0; i < PLANET_COUNT; ++i)
	{
		col += mix(WEAK_GRAVITY_COL, colours[i], t[i]);
	}
	col *= 1.0f / PLANET_COUNT;
	col = normalize(col);
	// forward the vertex color
	// gl_FrontColor = gl_Color ;
	col.w = 0.5f;
	gl_FrontColor = col;
}
