#include <windows.h>
#include <Fl/gl.h>
#include <gl/glu.h>

#include "camera.h"

#pragma warning(push)
#pragma warning(disable : 4244)

#ifndef M_PI
#define M_PI 3.141592653589793238462643383279502
#endif 

const float kMouseRotationSensitivity		= 1.0f/90.0f;
const float kMouseTranslationXSensitivity	= 0.03f;
const float kMouseTranslationYSensitivity	= 0.03f;
const float kMouseZoomSensitivity			= 0.08f;

void MakeDiagonal(Mat4f &m, float k)
{
	register int i,j;

	for (i=0; i<4; i++)
		for (j=0; j<4; j++)
			m[i][j] = (i==j) ? k : 0.0f;
}

void MakeHScale(Mat4f &m, const Vec3f &s)	
{
	MakeDiagonal(m,1.0f);
	m[0][0] = s[0]; m[1][1] = s[1];	m[2][2] = s[2];
}

void MakeHTrans(Mat4f &m, const Vec3f &s)
{
	MakeDiagonal(m,1.0f);
	m[0][3] = s[0]; m[1][3] = s[1]; m[2][3] = s[2];
}

void MakeHRotX(Mat4f &m, float theta)
{
	MakeDiagonal(m,1.0f);
	float cosTheta = cos(theta);
	float sinTheta = sin(theta);
	m[1][1] = cosTheta;
	m[1][2] = -sinTheta;
	m[2][1] = sinTheta;
	m[2][2] = cosTheta;
}

void MakeHRotY(Mat4f &m, float theta)
{
	MakeDiagonal(m,1.0f);
	float cosTheta = cos(theta);
	float sinTheta = sin(theta);
	m[0][0] = cosTheta;
	m[2][0] = -sinTheta;
	m[0][2] = sinTheta;
	m[2][2] = cosTheta;
}

void MakeHRotZ(Mat4f &m, float theta)
{
	MakeDiagonal(m,1.0f);
	float cosTheta = cos(theta);
	float sinTheta = sin(theta);
	m[0][0] = cosTheta;
	m[0][1] = -sinTheta;
	m[1][0] = sinTheta;
	m[1][1] = cosTheta;
}


void Camera::calculateViewingTransformParameters() 
{
	Mat4f dollyXform;
	Mat4f azimXform;
	Mat4f elevXform;
	Mat4f originXform;

	MakeHTrans(dollyXform, Vec3f(0,0,mDolly));
	MakeHRotY(azimXform, mAzimuth);
	MakeHRotX(elevXform, mElevation);
	MakeHTrans(originXform, mLookAt);
	
	mPosition = Vec3f(0,0,0);
	// grouped for (mat4 * vec3) ops instead of (mat4 * mat4) ops
	mPosition = originXform * (azimXform * (elevXform * (dollyXform * mPosition)));

	if ( fmod((double)mElevation, 2.0*M_PI) < 3*M_PI/2 && fmod((double)mElevation, 2.0*M_PI) > M_PI/2 )
		mUpVector= Vec3f(0,-1,0);
	else
		mUpVector= Vec3f(0,1,0);

	// Apply camera twist: rotate the up vector around the view axis
	Vec3f axis = mLookAt - mPosition;
	float axisLen = sqrt(axis * axis);
	if (axisLen > 1e-6f && fabs(mTwist) > 1e-6f)
	{
		axis /= axisLen;
		float c = cos(mTwist);
		float s = sin(mTwist);
		Vec3f u = mUpVector;
		// Rodrigues' rotation formula
		mUpVector = u * c
			+ (axis ^ u) * s
			+ axis * ((axis * u) * (1.0f - c));
		float upLen = sqrt(mUpVector * mUpVector);
		if (upLen > 1e-6f)
			mUpVector /= upLen;
	}

	mDirtyTransform = false;
}

Camera::Camera() 
{
	mElevation = mAzimuth = mTwist = 0.0f;
	mDolly = -20.0f;
	mElevation = 0.2f;
	mAzimuth = (float)M_PI;

	mLookAt = Vec3f( 0, 0, 0 );
	mCurrentMouseAction = kActionNone;

	calculateViewingTransformParameters();
}

void Camera::clickMouse( MouseAction_t action, int x, int y )
{
	mCurrentMouseAction = action;
	mLastMousePosition[0] = x;
	mLastMousePosition[1] = y;
}

void Camera::dragMouse( int x, int y )
{
	Vec3f mouseDelta   = Vec3f(x,y,0.0f) - mLastMousePosition;
	mLastMousePosition = Vec3f(x,y,0.0f);

	switch(mCurrentMouseAction)
	{
	case kActionTranslate:
		{
			calculateViewingTransformParameters();

			double xTrack =  -mouseDelta[0] * kMouseTranslationXSensitivity;
			double yTrack =  mouseDelta[1] * kMouseTranslationYSensitivity;

			Vec3f transXAxis = mUpVector ^ (mPosition - mLookAt);
			transXAxis /= sqrt((transXAxis*transXAxis));
			Vec3f transYAxis = (mPosition - mLookAt) ^ transXAxis;
			transYAxis /= sqrt((transYAxis*transYAxis));

			setLookAt(getLookAt() + transXAxis*xTrack + transYAxis*yTrack);
			
			break;
		}
	case kActionRotate:
		{
			float dAzimuth		=   -mouseDelta[0] * kMouseRotationSensitivity;
			float dElevation	=   mouseDelta[1] * kMouseRotationSensitivity;
			
			setAzimuth(getAzimuth() + dAzimuth);
			setElevation(getElevation() + dElevation);
			
			break;
		}
	case kActionZoom:
		{
			float dDolly = -mouseDelta[1] * kMouseZoomSensitivity;
			setDolly(getDolly() + dDolly);
			break;
		}
	case kActionTwist:
		{
			// Horizontal drag twists the camera about the view axis
			float dTwist = -mouseDelta[0] * kMouseRotationSensitivity;
			setTwist(getTwist() + dTwist);
			break;
		}
	default:
		break;
	}

}

void Camera::releaseMouse( int x, int y )
{
	mCurrentMouseAction = kActionNone;
}

void Camera::lookAt(double *eye, double *at, double *up)
{
	// Build an orthonormal basis matching gluLookAt, without calling it.
	Vec3f eyeV((float)eye[0], (float)eye[1], (float)eye[2]);
	Vec3f atV((float)at[0], (float)at[1], (float)at[2]);
	Vec3f upV((float)up[0], (float)up[1], (float)up[2]);

	Vec3f forward = atV - eyeV;
	float fLen = sqrt(forward * forward);
	if (fLen < 1e-8f)
		return;
	forward /= fLen;

	Vec3f side = forward ^ upV;
	float sLen = sqrt(side * side);
	if (sLen < 1e-8f)
	{
		// Degenerate up: pick an alternate
		upV = Vec3f(0, 1, 0);
		side = forward ^ upV;
		sLen = sqrt(side * side);
		if (sLen < 1e-8f)
		{
			upV = Vec3f(1, 0, 0);
			side = forward ^ upV;
			sLen = sqrt(side * side);
		}
	}
	side /= sLen;
	Vec3f upN = side ^ forward; // already unit-length

	// Column-major OpenGL matrix: rows are side, upN, -forward
	GLfloat m[16];
	m[0]  = side[0];  m[4]  = side[1];  m[8]  = side[2];  m[12] = 0.0f;
	m[1]  = upN[0];   m[5]  = upN[1];   m[9]  = upN[2];   m[13] = 0.0f;
	m[2]  = -forward[0]; m[6] = -forward[1]; m[10] = -forward[2]; m[14] = 0.0f;
	m[3]  = 0.0f;     m[7]  = 0.0f;     m[11] = 0.0f;     m[15] = 1.0f;

	glMultMatrixf(m);
	glTranslated(-eye[0], -eye[1], -eye[2]);
}

void Camera::frameAll(float centerX, float centerY, float centerZ, float radius)
{
	setLookAt(Vec3f(centerX, centerY, centerZ));
	// Perspective fovy is 30 degrees in ModelerView; fit sphere with margin
	const float fovyRad = 30.0f * (float)M_PI / 180.0f;
	float dist = (radius * 1.35f) / tan(fovyRad * 0.5f);
	setDolly(-dist);
	setElevation(0.35f);
	setAzimuth((float)M_PI);
	setTwist(0.0f);
}

void Camera::applyViewingTransform() {
	if( mDirtyTransform )
		calculateViewingTransformParameters();

	// Place the camera at mPosition, aim the camera at
	// mLookAt, and twist the camera such that mUpVector is up
	double eye[3] = { mPosition[0], mPosition[1], mPosition[2] };
	double at[3]  = { mLookAt[0],   mLookAt[1],   mLookAt[2] };
	double up[3]  = { mUpVector[0], mUpVector[1], mUpVector[2] };
	lookAt(eye, at, up);
}

#pragma warning(pop)