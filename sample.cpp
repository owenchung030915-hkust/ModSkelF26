// Hierarchical cat model for Project 2 (Modeler)
// Bells: texture, triangle mesh, torus, SoR, extrusion, L-system,
//        multi anim modes, mood reactions, individuality, Frame All,
//        dramatic light, LOD, shadow, yarn toy, mouse, blink, breathe
#include "modelerview.h"
#include "modelerapp.h"
#include "modelerdraw.h"
#include "camera.h"

#include <FL/gl.h>
#include <cmath>
#include <cstdlib>

#include "modelerglobals.h"

#ifndef M_PI
#define M_PI 3.141592653589793238462643383279502
#endif

// ---------------------------------------------------------------------------
// Hierarchy (4+ levels):
//   world -> body -> neck -> head -> ears/eyes/whiskers/collar
//                 -> front legs -> lower legs -> paws
//                 -> hind legs  -> lower legs -> paws
//                 -> tail base -> mid -> tip
// ---------------------------------------------------------------------------
class CatModel : public ModelerView
{
public:
	CatModel(int x, int y, int w, int h, char *label)
		: ModelerView(x, y, w, h, label),
		  m_animPhase(0.0),
		  m_blinkPhase(0.0),
		  m_texReady(false),
		  m_texId(0),
		  m_lastFrameAll(0.0)
	{}

	virtual void draw();

private:
	double m_animPhase;
	double m_blinkPhase;
	bool   m_texReady;
	GLuint m_texId;
	double m_lastFrameAll;

	void ensureFurTexture();
	void applyDramaticLighting(float drama);
	void setFurColor(float orange, float stripeMix, float shade);

	void drawTexturedBody(double sx, double sy, double sz);
	void drawEarMesh(float earScale);
	void drawTorus(double R, double r, int majorSeg, int minorSeg);
	void drawSurfaceOfRevolution(const float *profile, int nProfile, int slices);
	void drawExtrudedSurface(const float *crossXY, int nCross, float pathLen, int pathSegs);
	void drawFoodBowl();
	void drawCushion();
	void drawYarnBall(float spin);
	void drawMouseToy(float lookYaw);
	void drawContactShadow(float sx, float sz);
	void drawLSystemPlant(int depth, float angleDeg, float length);
	void drawWhiskers(int lod, float twitch);
	void drawLeg(float hipAngle, float kneeAngle, float legLen, float fluff, int lod);
	void drawTail(float baseA, float midA, float tipA, float fluff, float tipFluff, int lod);
};

ModelerView* createCatModel(int x, int y, int w, int h, char *label)
{
	return new CatModel(x, y, w, h, label);
}

// ---------------------------------------------------------------------------
static void hsvToRgb(float h, float s, float v, float &r, float &g, float &b)
{
	h = fmod(h, 360.0f);
	if (h < 0) h += 360.0f;
	float c = v * s;
	float x = c * (1.0f - fabs(fmod(h / 60.0f, 2.0f) - 1.0f));
	float m = v - c;
	float rp = 0, gp = 0, bp = 0;
	if (h < 60)      { rp = c; gp = x; }
	else if (h < 120){ rp = x; gp = c; }
	else if (h < 180){ gp = c; bp = x; }
	else if (h < 240){ gp = x; bp = c; }
	else if (h < 300){ rp = x; bp = c; }
	else             { rp = c; bp = x; }
	r = rp + m; g = gp + m; b = bp + m;
}

void CatModel::ensureFurTexture()
{
	if (m_texReady) return;

	const int W = 64, H = 64;
	unsigned char data[W * H * 3];
	for (int y = 0; y < H; y++)
	{
		for (int x = 0; x < W; x++)
		{
			float stripe = 0.55f + 0.45f * (float)sin(x * 0.45 + sin(y * 0.15) * 2.0);
			float noise = ((x * 11 + y * 17) % 13) / 26.0f;
			float v = stripe * 0.85f + noise * 0.15f;
			int i = (y * W + x) * 3;
			data[i + 0] = (unsigned char)(220 * v);
			data[i + 1] = (unsigned char)(140 * v);
			data[i + 2] = (unsigned char)(60 * v);
		}
	}

	glGenTextures(1, &m_texId);
	glBindTexture(GL_TEXTURE_2D, m_texId);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, W, H, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
	m_texReady = true;
}

void CatModel::applyDramaticLighting(float drama)
{
	float t = drama;
	GLfloat pos0[] = { 6.0f - 2.0f * t, 8.0f, -3.0f + t, 0.0f };
	GLfloat dif0[] = { 1.0f, 0.95f - 0.2f * t, 0.85f - 0.3f * t, 1.0f };
	GLfloat amb0[] = { 0.05f, 0.05f, 0.08f, 1.0f };
	GLfloat pos1[] = { -5.0f, 2.0f + 3.0f * t, 4.0f, 0.0f };
	GLfloat dif1[] = { 0.2f + 0.3f * t, 0.35f, 0.7f + 0.2f * t, 1.0f };
	GLfloat pos2[] = { 0.0f, -1.0f, -6.0f, 0.0f };
	GLfloat dif2[] = { 0.35f * t, 0.1f * t, 0.05f * t, 1.0f };

	glEnable(GL_LIGHT0);
	glEnable(GL_LIGHT1);
	glLightfv(GL_LIGHT0, GL_POSITION, pos0);
	glLightfv(GL_LIGHT0, GL_DIFFUSE, dif0);
	glLightfv(GL_LIGHT0, GL_AMBIENT, amb0);
	glLightfv(GL_LIGHT1, GL_POSITION, pos1);
	glLightfv(GL_LIGHT1, GL_DIFFUSE, dif1);

	if (t > 0.2f)
	{
		glEnable(GL_LIGHT2);
		glLightfv(GL_LIGHT2, GL_POSITION, pos2);
		glLightfv(GL_LIGHT2, GL_DIFFUSE, dif2);
	}
	else
		glDisable(GL_LIGHT2);
}

void CatModel::setFurColor(float orange, float stripeMix, float shade)
{
	float r, g, b;
	float hue = 45.0f - orange * 25.0f;
	float sat = 0.25f + orange * 0.55f;
	float val = (0.55f + 0.35f * (1.0f - stripeMix)) * shade;
	hsvToRgb(hue, sat, val, r, g, b);
	setAmbientColor(r * 0.25f, g * 0.25f, b * 0.25f);
	setDiffuseColor(r, g, b);
}

void CatModel::drawTexturedBody(double sx, double sy, double sz)
{
	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, m_texId);
	glColor3f(1, 1, 1);

	glBegin(GL_QUADS);
	glNormal3d(0, 0, 1);
	glTexCoord2d(0, 0); glVertex3d(0, 0, sz);
	glTexCoord2d(1, 0); glVertex3d(sx, 0, sz);
	glTexCoord2d(1, 1); glVertex3d(sx, sy, sz);
	glTexCoord2d(0, 1); glVertex3d(0, sy, sz);
	glNormal3d(0, 0, -1);
	glTexCoord2d(0, 0); glVertex3d(sx, 0, 0);
	glTexCoord2d(1, 0); glVertex3d(0, 0, 0);
	glTexCoord2d(1, 1); glVertex3d(0, sy, 0);
	glTexCoord2d(0, 1); glVertex3d(sx, sy, 0);
	glNormal3d(0, 1, 0);
	glTexCoord2d(0, 0); glVertex3d(0, sy, 0);
	glTexCoord2d(1, 0); glVertex3d(0, sy, sz);
	glTexCoord2d(1, 1); glVertex3d(sx, sy, sz);
	glTexCoord2d(0, 1); glVertex3d(sx, sy, 0);
	glNormal3d(0, -1, 0);
	glTexCoord2d(0, 0); glVertex3d(0, 0, sz);
	glTexCoord2d(1, 0); glVertex3d(0, 0, 0);
	glTexCoord2d(1, 1); glVertex3d(sx, 0, 0);
	glTexCoord2d(0, 1); glVertex3d(sx, 0, sz);
	glNormal3d(1, 0, 0);
	glTexCoord2d(0, 0); glVertex3d(sx, 0, sz);
	glTexCoord2d(1, 0); glVertex3d(sx, 0, 0);
	glTexCoord2d(1, 1); glVertex3d(sx, sy, 0);
	glTexCoord2d(0, 1); glVertex3d(sx, sy, sz);
	glNormal3d(-1, 0, 0);
	glTexCoord2d(0, 0); glVertex3d(0, 0, 0);
	glTexCoord2d(1, 0); glVertex3d(0, 0, sz);
	glTexCoord2d(1, 1); glVertex3d(0, sy, sz);
	glTexCoord2d(0, 1); glVertex3d(0, sy, 0);
	glEnd();

	glDisable(GL_TEXTURE_2D);
}

void CatModel::drawEarMesh(float earScale)
{
	float s = earScale;
	drawTriangle(0, 0, 0,   0.35f * s, 0, 0.1f * s,   0.15f * s, 0.7f * s, 0);
	drawTriangle(0, 0, 0,   0.15f * s, 0.7f * s, 0,  -0.05f * s, 0.1f * s, -0.12f * s);
	drawTriangle(0.35f * s, 0, 0.1f * s,  0.15f * s, 0.7f * s, 0,  0.2f * s, 0.05f * s, -0.1f * s);
	setDiffuseColor(0.95f, 0.65f, 0.7f);
	drawTriangle(0.05f * s, 0.08f * s, 0.02f * s,
		0.25f * s, 0.08f * s, 0.05f * s,
		0.14f * s, 0.45f * s, 0.01f * s);
}

void CatModel::drawTorus(double R, double r, int majorSeg, int minorSeg)
{
	for (int i = 0; i < majorSeg; i++)
	{
		double a0 = (i / (double)majorSeg) * 2.0 * M_PI;
		double a1 = ((i + 1) / (double)majorSeg) * 2.0 * M_PI;
		for (int j = 0; j < minorSeg; j++)
		{
			double b0 = (j / (double)minorSeg) * 2.0 * M_PI;
			double b1 = ((j + 1) / (double)minorSeg) * 2.0 * M_PI;
			double c0 = cos(a0), s0 = sin(a0);
			double c1 = cos(a1), s1 = sin(a1);
			double u0 = cos(b0), v0 = sin(b0);
			double u1 = cos(b1), v1 = sin(b1);
			double x00 = (R + r * u0) * c0, y00 = r * v0, z00 = (R + r * u0) * s0;
			double x01 = (R + r * u1) * c0, y01 = r * v1, z01 = (R + r * u1) * s0;
			double x10 = (R + r * u0) * c1, y10 = r * v0, z10 = (R + r * u0) * s1;
			double x11 = (R + r * u1) * c1, y11 = r * v1, z11 = (R + r * u1) * s1;
			drawTriangle(x00, y00, z00, x10, y10, z10, x11, y11, z11);
			drawTriangle(x00, y00, z00, x11, y11, z11, x01, y01, z01);
		}
	}
}

void CatModel::drawSurfaceOfRevolution(const float *profile, int nProfile, int slices)
{
	for (int i = 0; i < nProfile - 1; i++)
	{
		float r0 = profile[i * 2], y0 = profile[i * 2 + 1];
		float r1 = profile[(i + 1) * 2], y1 = profile[(i + 1) * 2 + 1];
		for (int s = 0; s < slices; s++)
		{
			double a0 = (s / (double)slices) * 2.0 * M_PI;
			double a1 = ((s + 1) / (double)slices) * 2.0 * M_PI;
			double x00 = r0 * cos(a0), z00 = r0 * sin(a0);
			double x01 = r0 * cos(a1), z01 = r0 * sin(a1);
			double x10 = r1 * cos(a0), z10 = r1 * sin(a0);
			double x11 = r1 * cos(a1), z11 = r1 * sin(a1);
			drawTriangle(x00, y0, z00, x10, y1, z10, x11, y1, z11);
			drawTriangle(x00, y0, z00, x11, y1, z11, x01, y0, z01);
		}
	}
}

// Bell: extruded surface — sweep a 2D cross-section along +Z
void CatModel::drawExtrudedSurface(const float *crossXY, int nCross, float pathLen, int pathSegs)
{
	for (int s = 0; s < pathSegs; s++)
	{
		float z0 = (s / (float)pathSegs) * pathLen;
		float z1 = ((s + 1) / (float)pathSegs) * pathLen;
		for (int i = 0; i < nCross; i++)
		{
			int j = (i + 1) % nCross;
			float x0 = crossXY[i * 2], y0 = crossXY[i * 2 + 1];
			float x1 = crossXY[j * 2], y1 = crossXY[j * 2 + 1];
			drawTriangle(x0, y0, z0,  x1, y1, z0,  x1, y1, z1);
			drawTriangle(x0, y0, z0,  x1, y1, z1,  x0, y0, z1);
		}
	}
	// Caps
	for (int i = 1; i < nCross - 1; i++)
	{
		drawTriangle(crossXY[0], crossXY[1], 0,
			crossXY[i * 2], crossXY[i * 2 + 1], 0,
			crossXY[(i + 1) * 2], crossXY[(i + 1) * 2 + 1], 0);
		drawTriangle(crossXY[0], crossXY[1], pathLen,
			crossXY[(i + 1) * 2], crossXY[(i + 1) * 2 + 1], pathLen,
			crossXY[i * 2], crossXY[i * 2 + 1], pathLen);
	}
}

void CatModel::drawFoodBowl()
{
	const float profile[] = {
		0.05f, 0.00f,
		0.55f, 0.05f,
		0.70f, 0.25f,
		0.65f, 0.45f,
		0.50f, 0.55f,
	};
	setAmbientColor(0.1f, 0.1f, 0.12f);
	setDiffuseColor(0.75f, 0.75f, 0.8f);
	drawSurfaceOfRevolution(profile, 5, 24);
	setDiffuseColor(0.55f, 0.35f, 0.15f);
	glPushMatrix();
	glTranslated(0, 0.25, 0);
	drawSphere(0.28);
	glPopMatrix();
}

void CatModel::drawCushion()
{
	// Rounded pillow via extrusion of an oval cross-section (new primitive bell)
	const int N = 16;
	float cross[N * 2];
	for (int i = 0; i < N; i++)
	{
		float a = (i / (float)N) * 2.0f * (float)M_PI;
		cross[i * 2] = 0.55f * (float)cos(a);
		cross[i * 2 + 1] = 0.18f * (float)sin(a);
	}
	setAmbientColor(0.1f, 0.05f, 0.08f);
	setDiffuseColor(0.75f, 0.25f, 0.4f);
	glPushMatrix();
	glTranslated(0, 0.18, -0.7);
	drawExtrudedSurface(cross, N, 1.4f, 8);
	glPopMatrix();
	// Button
	setDiffuseColor(0.9f, 0.85f, 0.3f);
	glPushMatrix();
	glTranslated(0, 0.36, 0);
	drawSphere(0.08);
	glPopMatrix();
}

void CatModel::drawYarnBall(float spin)
{
	setDiffuseColor(0.2f, 0.55f, 0.9f);
	glPushMatrix();
	glTranslated(0, 0.22, 0);
	glRotated(spin, 0.3, 1, 0.2);
	drawSphere(0.22);
	setDiffuseColor(0.95f, 0.95f, 0.2f);
	glRotated(90, 1, 0, 0);
	drawTorus(0.18, 0.03, 16, 8);
	glRotated(60, 0, 1, 0);
	drawTorus(0.16, 0.025, 14, 8);
	glPopMatrix();
	// Loose yarn strand (extruded ribbon)
	setDiffuseColor(0.25f, 0.6f, 0.95f);
	const float strand[] = {
		-0.03f, 0.0f,  0.03f, 0.0f,  0.03f, 0.04f,  -0.03f, 0.04f
	};
	glPushMatrix();
	glTranslated(0.15, 0.02, 0.1);
	glRotated(spin * 0.5f, 0, 1, 0);
	glRotated(20, 1, 0, 0);
	drawExtrudedSurface(strand, 4, 0.9f, 6);
	glPopMatrix();
}

void CatModel::drawMouseToy(float lookYaw)
{
	glPushMatrix();
	glRotated(lookYaw, 0, 1, 0);
	setDiffuseColor(0.55f, 0.55f, 0.58f);
	glPushMatrix();
	glScaled(1.2, 0.7, 0.7);
	drawSphere(0.18);
	glPopMatrix();
	// Head
	glPushMatrix();
	glTranslated(0.18, 0.02, 0);
	drawSphere(0.1);
	setDiffuseColor(0.9f, 0.5f, 0.55f);
	glTranslated(0.08, 0, 0);
	glScaled(0.5, 0.4, 0.4);
	drawSphere(0.06);
	glPopMatrix();
	// Ears
	setDiffuseColor(0.65f, 0.65f, 0.68f);
	for (int s = -1; s <= 1; s += 2)
	{
		glPushMatrix();
		glTranslated(0.12, 0.1, s * 0.08);
		glScaled(0.5, 1, 0.8);
		drawSphere(0.06);
		glPopMatrix();
	}
	// Tail
	setDiffuseColor(0.9f, 0.45f, 0.5f);
	glPushMatrix();
	glTranslated(-0.2, 0, 0);
	glRotated(-70, 0, 0, 1);
	drawCylinder(0.45, 0.025, 0.01);
	glPopMatrix();
	glPopMatrix();
}

void CatModel::drawContactShadow(float sx, float sz)
{
	// Soft ground blob (whistle) — dark flat ellipsoid
	setAmbientColor(0, 0, 0);
	setDiffuseColor(0.05f, 0.05f, 0.05f);
	glPushMatrix();
	glTranslated(0, 0.015, 0);
	glScaled(sx, 0.02, sz);
	drawSphere(1.0);
	glPopMatrix();
}

void CatModel::drawLSystemPlant(int depth, float angleDeg, float length)
{
	if (depth <= 0 || length < 0.05f)
	{
		setDiffuseColor(0.2f, 0.7f, 0.25f);
		drawSphere(0.06);
		return;
	}
	setDiffuseColor(0.35f, 0.22f, 0.1f);
	drawCylinder(length, 0.04 * depth / 4.0, 0.02 * depth / 4.0);
	glTranslated(0, 0, length);

	glPushMatrix();
	glRotated(angleDeg, 0, 1, 0);
	glRotated(25, 1, 0, 0);
	drawLSystemPlant(depth - 1, angleDeg, length * 0.7f);
	glPopMatrix();

	glPushMatrix();
	glRotated(-angleDeg, 0, 1, 0);
	glRotated(20, 1, 0, 0);
	drawLSystemPlant(depth - 1, angleDeg, length * 0.65f);
	glPopMatrix();

	glPushMatrix();
	glRotated(8, 0, 0, 1);
	drawLSystemPlant(depth - 1, angleDeg * 0.9f, length * 0.55f);
	glPopMatrix();
}

void CatModel::drawWhiskers(int lod, float twitch)
{
	if (lod >= 2) return;
	setDiffuseColor(0.95f, 0.95f, 0.9f);
	int n = (lod == 0) ? 3 : 2;
	for (int side = -1; side <= 1; side += 2)
	{
		for (int i = 0; i < n; i++)
		{
			glPushMatrix();
			glTranslated(side * 0.25, 0.05 - i * 0.06, 0.35);
			glRotated(side * (55 + i * 12 + twitch), 0, 1, 0);
			glRotated(-10 + i * 8 + twitch * 0.3f, 1, 0, 0);
			drawCylinder(0.55 - i * 0.05, 0.012, 0.006);
			glPopMatrix();
		}
	}
}

void CatModel::drawLeg(float hipAngle, float kneeAngle, float legLen, float fluff, int lod)
{
	glRotated(hipAngle, 0, 0, 1);
	glRotated(90, 1, 0, 0);

	const float upper = legLen * 0.55f;
	const float lower = legLen * 0.45f;
	const float pawR = 0.13f * fluff;

	setFurColor((float)VAL(FUR_ORANGE), (float)VAL(STRIPE_AMOUNT), 0.9f);
	drawCylinder(upper, 0.14 * fluff, 0.11 * fluff);

	glTranslated(0, 0, upper);
	glRotated(kneeAngle, 1, 0, 0);
	drawCylinder(lower, 0.11 * fluff, 0.09 * fluff);

	glTranslated(0, 0, lower);
	setFurColor((float)VAL(FUR_ORANGE), 0.2f, 0.75f);
	glPushMatrix();
	glScaled(1.15, 0.75, 1.25);
	drawSphere(pawR);
	glPopMatrix();

	if (lod == 0)
	{
		setDiffuseColor(0.95f, 0.8f, 0.75f);
		for (int t = -1; t <= 1; t++)
		{
			glPushMatrix();
			glTranslated(t * 0.07, -0.02, 0.06);
			drawSphere(0.035);
			glPopMatrix();
		}
	}
}

void CatModel::drawTail(float baseA, float midA, float tipA, float fluff, float tipFluff, int lod)
{
	glRotated(baseA, 1, 0, 0);
	setFurColor((float)VAL(FUR_ORANGE), (float)VAL(STRIPE_AMOUNT), 1.0f);
	drawCylinder(0.55, 0.12 * fluff, 0.10 * fluff);

	glTranslated(0, 0, 0.55);
	glRotated(midA, 1, 0, 0);
	drawCylinder(0.5, 0.10 * fluff, 0.08 * fluff * (0.8f + 0.4f * tipFluff));

	glTranslated(0, 0, 0.5);
	glRotated(tipA, 1, 0, 0);
	float tipR = 0.08f * fluff * (0.7f + 0.8f * tipFluff);
	drawCylinder(0.4, tipR, 0.04f * fluff);

	if (lod < 2)
	{
		glTranslated(0, 0, 0.4);
		drawSphere(0.06f * fluff * (1.0f + tipFluff));
	}
}

// ---------------------------------------------------------------------------
void CatModel::draw()
{
	ModelerView::draw();
	ensureFurTexture();

	const int lod = (int)(VAL(LOD) + 0.5);
	const int mood = (int)(VAL(MOOD) + 0.5);
	const int animMode = (int)(VAL(ANIM_MODE) + 0.5);
	const float fluff = 0.85f + 0.35f * (float)VAL(FLUFFINESS);
	const float earS = 0.7f + 0.7f * (float)VAL(EAR_SIZE);
	const float legLen = 0.85f + 0.5f * (float)VAL(LEG_LENGTH);
	const float faceW = 0.75f + 0.5f * (float)VAL(FACE_WIDTH);
	const float orange = (float)VAL(FUR_ORANGE);
	const float stripes = (float)VAL(STRIPE_AMOUNT);
	const float tipFluff = (float)VAL(TAIL_FLUFF);

	applyDramaticLighting((float)VAL(LIGHT_DRAMA));

	m_animPhase += 0.08 * VAL(ANIM_SPEED);
	if (m_animPhase > 2.0 * M_PI) m_animPhase -= 2.0 * M_PI;
	m_blinkPhase += 0.05 * (0.5 + VAL(BLINK));
	if (m_blinkPhase > 2.0 * M_PI) m_blinkPhase -= 2.0 * M_PI;

	float moodAnim = 1.0f, moodDroop = 0.0f, moodAlert = 0.0f, moodPlay = 0.0f;
	switch (mood)
	{
	case 1: moodPlay = 1.0f; moodAnim = 1.5f; break;
	case 2: moodDroop = 1.0f; moodAnim = 0.35f; break;
	case 3: moodAlert = 1.0f; moodAnim = 1.1f; break;
	default: break;
	}

	float anim = (float)sin(m_animPhase * moodAnim);
	float anim2 = (float)sin(m_animPhase * moodAnim * 2.0);
	float walk = (float)sin(m_animPhase * 2.2 * moodAnim);
	float walkOpp = (float)sin(m_animPhase * 2.2 * moodAnim + M_PI);

	float stretch = (float)VAL(STRETCH);
	float sit = (float)VAL(SIT);
	float pounce = (float)VAL(POUNCE);

	// Mood changes how coupled controls feel (storytelling)
	if (mood == 2) { stretch *= 0.3f; sit = sit * 0.5f + 0.4f; pounce *= 0.15f; }
	if (mood == 1) { stretch = stretch * 0.7f + 0.3f * fabs(anim); pounce = pounce * 0.8f + 0.2f * fabs(anim2); }
	if (mood == 3) { stretch *= 1.2f; pounce *= 1.15f; }

	// Animation modes (extra animated sequences — turn on Control→Animate)
	float modeLegFL = 0, modeLegFR = 0, modeLegHL = 0, modeLegHR = 0;
	float modeKneeF = 0, modeKneeH = 0;
	float modeHeadPitch = 0, modeHeadYaw = 0;
	float modeSpine = 0, modeBodyBob = 0;
	float modeGroomPaw = 0;

	switch (animMode)
	{
	case 1: // Walk
		modeLegFL = walk * 28.0f;
		modeLegFR = walkOpp * 28.0f;
		modeLegHL = walkOpp * 22.0f;
		modeLegHR = walk * 22.0f;
		modeKneeF = 8.0f + fabs(walk) * 12.0f;
		modeKneeH = 10.0f + fabs(walkOpp) * 14.0f;
		modeBodyBob = fabs(walk) * 0.06f;
		modeSpine = walk * 4.0f;
		break;
	case 2: // Pounce sequence (crouch → spring)
		{
			float t = 0.5f + 0.5f * anim; // 0..1
			modeSpine = -t * 18.0f + (1.0f - t) * 10.0f;
			modeLegFL = modeLegFR = -t * 25.0f + (1.0f - t) * 40.0f;
			modeLegHL = modeLegHR = t * 50.0f;
			modeKneeH = t * 55.0f;
			modeBodyBob = -t * 0.2f + (1.0f - t) * 0.15f;
			modeHeadPitch = -t * 15.0f;
		}
		break;
	case 3: // Groom
		modeHeadPitch = -35.0f + anim * 8.0f;
		modeHeadYaw = 20.0f + anim2 * 10.0f;
		modeGroomPaw = 55.0f + anim * 15.0f;
		modeLegFL = modeGroomPaw;
		modeKneeF = 40.0f;
		modeSpine = 8.0f;
		break;
	default: // Idle breathe / ear flick handled below
		break;
	}

	// Coupled pounce slider (independent of anim mode)
	float pounceCrouch = pounce;
	float pounceSpring = pounce; // same input drives hip+knee+spine
	modeSpine += -pounceCrouch * 20.0f + pounceSpring * 5.0f;
	modeLegFL += -pounce * 20.0f;
	modeLegFR += -pounce * 20.0f;
	modeLegHL += pounce * 40.0f;
	modeLegHR += pounce * 40.0f;
	modeKneeH += pounce * 50.0f;
	modeBodyBob += -pounce * 0.25f;

	float spineArch = stretch * 25.0f - sit * 10.0f + moodAlert * 8.0f + modeSpine;
	float frontReach = stretch * 35.0f - sit * 5.0f;
	float hindFold = sit * 55.0f + stretch * (-15.0f);
	float hindKnee = sit * 70.0f;

	float headYaw = (float)VAL(HEAD_YAW) + moodPlay * anim * 12.0f + moodAlert * 5.0f + modeHeadYaw;
	float headPitch = (float)VAL(HEAD_PITCH) - moodDroop * 25.0f + moodAlert * (-8.0f) + modeHeadPitch;

	float tailBase = (float)VAL(TAIL_BASE) + moodPlay * anim * 20.0f + moodAlert * 15.0f;
	float tailMid = (float)VAL(TAIL_MID) + moodPlay * anim2 * 18.0f;
	float tailTip = (float)VAL(TAIL_TIP) + moodPlay * anim * 25.0f - moodDroop * 10.0f;
	if (animMode == 1) { tailBase += walk * 10.0f; tailTip += walkOpp * 15.0f; }

	float legFL = (float)VAL(LEG_FL) + frontReach + moodPlay * anim * 15.0f + modeLegFL;
	float legFR = (float)VAL(LEG_FR) + frontReach - moodPlay * anim * 15.0f + modeLegFR;
	float legHL = (float)VAL(LEG_HL) + hindFold - moodPlay * anim * 10.0f + modeLegHL;
	float legHR = (float)VAL(LEG_HR) + hindFold + moodPlay * anim * 10.0f + modeLegHR;
	float kneeF = 12.0f + stretch * 15.0f + modeKneeF;
	float kneeH = 18.0f + hindKnee + modeKneeH;

	// Breathing (whistle)
	float breathe = 1.0f;
	if (VAL(BREATHE) > 0.5)
		breathe = 1.0f + 0.04f * (float)sin(m_animPhase * 0.9) * (1.0f - 0.5f * moodAlert);

	// Blink: periodic eye close; sleepy almost always half-closed
	float eyeOpen = 1.0f;
	if (VAL(BLINK) > 0.5)
	{
		float b = (float)sin(m_blinkPhase * 3.0);
		eyeOpen = (b > 0.92f) ? 0.08f : 1.0f; // quick blink near peaks
	}
	if (mood == 2) eyeOpen *= 0.35f;
	if (mood == 3) eyeOpen = eyeOpen * 0.85f + 0.15f; // wider

	float whiskerTwitch = moodPlay * anim2 * 8.0f + moodAlert * 3.0f;
	float earFlick = (mood == 3) ? anim2 * 12.0f : ((animMode == 0) ? anim * 6.0f * moodPlay : 0.0f);

	const float hipAttachY = -0.28f;
	const float pawR = 0.13f * fluff;
	const float kneeBendRad = 12.0f * (float)M_PI / 180.0f;
	const float stanceLen = legLen * (0.55f + 0.45f * cos(kneeBendRad)) + pawR;
	const float bodyRootY = (float)VAL(YPOS) + stanceLen - hipAttachY - sit * 0.35f + modeBodyBob;

	double frameVal = VAL(FRAME_ALL);
	if (frameVal > 0.5 && m_lastFrameAll <= 0.5)
	{
		m_camera->frameAll((float)VAL(XPOS), (float)VAL(YPOS) + 0.8f, (float)VAL(ZPOS), 3.8f);
	}
	m_lastFrameAll = frameVal;

	// Floor
	setAmbientColor(0.08f, 0.08f, 0.08f);
	setDiffuseColor(0.35f, 0.4f, 0.35f);
	glPushMatrix();
	glTranslated(-6, -0.01, -6);
	drawBox(12, 0.02, 12);
	glPopMatrix();

	// Cushion (extruded surface)
	if (VAL(SHOW_CUSHION) > 0.5 && lod < 2)
	{
		glPushMatrix();
		glTranslated(VAL(XPOS) - 0.2, 0, VAL(ZPOS) - 2.2);
		drawCushion();
		glPopMatrix();
	}

	if (VAL(SHOW_BOWL) > 0.5)
	{
		glPushMatrix();
		glTranslated(VAL(XPOS) + 2.2, 0, VAL(ZPOS) + 0.5);
		drawFoodBowl();
		glPopMatrix();
	}

	// Yarn ball — playful mood bats it around
	if (VAL(SHOW_YARN) > 0.5)
	{
		float yarnX = (float)VAL(XPOS) + 1.6f + moodPlay * anim * 0.35f;
		float yarnZ = (float)VAL(ZPOS) + 1.2f + moodPlay * anim2 * 0.25f;
		float yarnY = 0.0f + moodPlay * fabs(anim) * 0.35f;
		glPushMatrix();
		glTranslated(yarnX, yarnY, yarnZ);
		drawYarnBall((float)(m_animPhase * 40.0 * (1.0 + moodPlay)));
		glPopMatrix();
	}

	// Mouse toy faces the cat; alert cats stare, sleepy ignore
	if (VAL(SHOW_MOUSE) > 0.5 && lod < 2)
	{
		float mx = (float)VAL(XPOS) + 2.8f;
		float mz = (float)VAL(ZPOS) - 0.8f;
		float look = 180.0f + moodAlert * 20.0f - moodDroop * 40.0f + moodPlay * anim * 15.0f;
		glPushMatrix();
		glTranslated(mx, 0.12, mz);
		drawMouseToy(look);
		glPopMatrix();
	}

	int plantDepth = (int)(VAL(PLANT_DEPTH) + 0.5);
	if (plantDepth > 0 && lod < 2)
	{
		glPushMatrix();
		glTranslated(VAL(XPOS) - 2.3, 0, VAL(ZPOS) - 1.0);
		glRotated(-90, 1, 0, 0);
		drawLSystemPlant(plantDepth, (float)VAL(PLANT_ANGLE), 0.55f);
		glPopMatrix();
	}

	// Contact shadow under cat
	if (VAL(SHOW_SHADOW) > 0.5)
	{
		glPushMatrix();
		glTranslated(VAL(XPOS), 0, VAL(ZPOS));
		float shadowScale = 1.3f - sit * 0.2f + stretch * 0.15f;
		drawContactShadow(1.1f * shadowScale, 0.7f * shadowScale);
		glPopMatrix();
	}

	// ===== Cat root =====
	glPushMatrix();
	glTranslated(VAL(XPOS), bodyRootY, VAL(ZPOS));
	glRotated(spineArch, 0, 0, 1);
	glRotated(modeSpine * 0.3f, 1, 0, 0);

		// BODY
		setFurColor(orange, stripes, 1.0f);
		glPushMatrix();
		glTranslated(-1.0, -0.35, -0.55);
		glScaled((1.0 + 0.15 * (fluff - 1.0)) * breathe,
			fluff * breathe,
			(1.0 + 0.1 * (fluff - 1.0)) * breathe);
		if (VAL(TEXTURE_ON) > 0.5)
			drawTexturedBody(2.0, 0.9, 1.1);
		else
			drawBox(2.0, 0.9, 1.1);
		if (lod < 2)
		{
			glTranslated(1.0, 0.2, 0.55);
			glScaled(1.3, 0.8, 1.0);
			drawSphere(0.55);
		}
		glPopMatrix();

		// NECK
		glPushMatrix();
		glTranslated(0.95, 0.15, 0);
		glRotated(-8 - stretch * 5, 0, 0, 1);
			setFurColor(orange, stripes * 0.7f, 0.95f);
			drawSphere(0.42 * fluff);

			if (VAL(SHOW_COLLAR) > 0.5 && lod < 2)
			{
				setDiffuseColor(0.85f, 0.15f, 0.2f);
				glPushMatrix();
				glTranslated(0.05, -0.05, 0);
				glRotated(90, 1, 0, 0);
				int segs = (lod == 0) ? 24 : 12;
				drawTorus(0.38, 0.05, segs, segs / 2);
				setDiffuseColor(0.95f, 0.8f, 0.1f);
				glTranslated(0.38, 0, 0);
				drawSphere(0.08);
				glPopMatrix();
			}

			// HEAD
			glPushMatrix();
			glTranslated(0.45, 0.15, 0);
			glRotated(headYaw, 0, 1, 0);
			glRotated(headPitch, 0, 0, 1);
				setFurColor(orange, stripes * 0.5f, 1.0f);
				glPushMatrix();
				glScaled(faceW, 1.0, 0.95);
				drawSphere(0.48);
				glPopMatrix();

				glPushMatrix();
				glTranslated(0.32, -0.08, 0);
				glScaled(0.7, 0.55, 0.65 * faceW);
				setFurColor(orange, 0.15f, 1.05f);
				drawSphere(0.22);
				glPopMatrix();

				setDiffuseColor(0.95f, 0.55f, 0.6f);
				glPushMatrix();
				glTranslated(0.48, -0.02, 0);
				glScaled(0.5, 0.4, 0.45);
				drawSphere(0.1);
				glPopMatrix();

				if (lod < 2)
				{
					for (int s = -1; s <= 1; s += 2)
					{
						glPushMatrix();
						glTranslated(0.28, 0.12, s * 0.22 * faceW);
						setDiffuseColor(0.15f, 0.75f, 0.35f);
						glScaled(0.5, eyeOpen, 1.0);
						drawSphere(0.1);
						setDiffuseColor(0.05f, 0.05f, 0.05f);
						glTranslated(0.06, 0, 0);
						drawSphere(0.045);
						glPopMatrix();
					}
				}

				for (int s = -1; s <= 1; s += 2)
				{
					glPushMatrix();
					glTranslated(-0.05, 0.35, s * 0.28 * faceW);
					glRotated(s * (25 + earFlick), 0, 1, 0);
					glRotated(-20 - moodDroop * 30 + moodAlert * 10 + earFlick * 0.5f, 1, 0, 0);
					setFurColor(orange, stripes, 0.9f);
					drawEarMesh(earS);
					glPopMatrix();
				}

				drawWhiskers(lod, whiskerTwitch);
			glPopMatrix();
		glPopMatrix();

		glPushMatrix();
		glTranslated(0.7, hipAttachY, 0.38);
		drawLeg(legFL, kneeF, legLen, fluff, lod);
		glPopMatrix();

		glPushMatrix();
		glTranslated(0.7, hipAttachY, -0.38);
		drawLeg(legFR, kneeF, legLen, fluff, lod);
		glPopMatrix();

		glPushMatrix();
		glTranslated(-0.75, hipAttachY + 0.04f, 0.38);
		drawLeg(legHL, kneeH, legLen * 1.02f, fluff, lod);
		glPopMatrix();

		glPushMatrix();
		glTranslated(-0.75, hipAttachY + 0.04f, -0.38);
		drawLeg(legHR, kneeH, legLen * 1.02f, fluff, lod);
		glPopMatrix();

		glPushMatrix();
		glTranslated(-1.05, 0.15, 0);
		glRotated(-90 + tailBase * 0.2f, 0, 0, 1);
		drawTail(tailBase, tailMid, tailTip, fluff, tipFluff, lod);
		glPopMatrix();

	glPopMatrix();
}

// ---------------------------------------------------------------------------
int main()
{
	ModelerControl controls[NUMCONTROLS];

	controls[XPOS] = ModelerControl("X Position", -5, 5, 0.1f, 0);
	controls[YPOS] = ModelerControl("Y Position", 0, 3, 0.1f, 0);
	controls[ZPOS] = ModelerControl("Z Position", -5, 5, 0.1f, 0);

	controls[HEAD_YAW] = ModelerControl("Head Yaw", -60, 60, 1, 0);
	controls[HEAD_PITCH] = ModelerControl("Head Pitch", -40, 40, 1, 0);
	controls[TAIL_BASE] = ModelerControl("Tail Base", -80, 80, 1, 20);
	controls[TAIL_MID] = ModelerControl("Tail Mid", -80, 80, 1, 15);
	controls[TAIL_TIP] = ModelerControl("Tail Tip", -80, 80, 1, 10);

	controls[LEG_FL] = ModelerControl("Front Left Leg", -40, 60, 1, 0);
	controls[LEG_FR] = ModelerControl("Front Right Leg", -40, 60, 1, 0);
	controls[LEG_HL] = ModelerControl("Hind Left Leg", -40, 60, 1, 0);
	controls[LEG_HR] = ModelerControl("Hind Right Leg", -40, 60, 1, 0);

	controls[STRETCH] = ModelerControl("Stretch (multi)", 0, 1, 0.01f, 0);
	controls[SIT] = ModelerControl("Sit (multi)", 0, 1, 0.01f, 0);
	controls[POUNCE] = ModelerControl("Pounce (multi)", 0, 1, 0.01f, 0);

	controls[LOD] = ModelerControl("Level of Detail", 0, 2, 1, 0);
	controls[MOOD] = ModelerControl("Mood 0c/1p/2s/3a", 0, 3, 1, 0);
	controls[ANIM_SPEED] = ModelerControl("Anim Speed", 0, 3, 0.05f, 1);
	controls[ANIM_MODE] = ModelerControl("Anim 0idle/1walk/2pounce/3groom", 0, 3, 1, 0);

	controls[FUR_ORANGE] = ModelerControl("Fur Orange", 0, 1, 0.01f, 0.7f);
	controls[FLUFFINESS] = ModelerControl("Fluffiness", 0, 1, 0.01f, 0.4f);
	controls[EAR_SIZE] = ModelerControl("Ear Size", 0, 1, 0.01f, 0.5f);
	controls[LEG_LENGTH] = ModelerControl("Leg Length", 0, 1, 0.01f, 0.4f);
	controls[FACE_WIDTH] = ModelerControl("Face Width", 0, 1, 0.01f, 0.5f);
	controls[STRIPE_AMOUNT] = ModelerControl("Stripe Amount", 0, 1, 0.01f, 0.5f);
	controls[TAIL_FLUFF] = ModelerControl("Tail Fluff", 0, 1, 0.01f, 0.4f);

	controls[PLANT_DEPTH] = ModelerControl("Plant L-System Depth", 0, 5, 1, 3);
	controls[PLANT_ANGLE] = ModelerControl("Plant Branch Angle", 10, 45, 1, 28);

	controls[TEXTURE_ON] = ModelerControl("Fur Texture", 0, 1, 1, 1);
	controls[FRAME_ALL] = ModelerControl("Frame All (trigger)", 0, 1, 1, 0);
	controls[LIGHT_DRAMA] = ModelerControl("Dramatic Light", 0, 1, 0.01f, 0.65f);
	controls[SHOW_BOWL] = ModelerControl("Show Food Bowl", 0, 1, 1, 1);
	controls[SHOW_COLLAR] = ModelerControl("Show Collar", 0, 1, 1, 1);
	controls[SHOW_SHADOW] = ModelerControl("Contact Shadow", 0, 1, 1, 1);
	controls[SHOW_YARN] = ModelerControl("Yarn Ball Toy", 0, 1, 1, 1);
	controls[SHOW_CUSHION] = ModelerControl("Cushion (extrude)", 0, 1, 1, 1);
	controls[SHOW_MOUSE] = ModelerControl("Mouse Toy", 0, 1, 1, 1);
	controls[BLINK] = ModelerControl("Blinking Eyes", 0, 1, 1, 1);
	controls[BREATHE] = ModelerControl("Breathing", 0, 1, 1, 1);

	ModelerApplication::Instance()->Init(&createCatModel, controls, NUMCONTROLS);
	return ModelerApplication::Instance()->Run();
}
