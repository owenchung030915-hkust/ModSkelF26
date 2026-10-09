#ifndef _MODELER_GLOBALS_H
#define _MODELER_GLOBALS_H

#ifndef M_PI
#define M_PI 3.141592653589793238462643383279502
#endif

// Controls for the Cat model
enum CatModelControls
{
	// Position
	XPOS, YPOS, ZPOS,

	// Individual joints
	HEAD_YAW, HEAD_PITCH,
	TAIL_BASE, TAIL_MID, TAIL_TIP,
	LEG_FL, LEG_FR, LEG_HL, LEG_HR,

	// Multi-joint / coupled actions
	STRETCH,		// spine + front legs
	SIT,			// hips + knees (hind)
	POUNCE,			// crouch then spring (multi)

	// Detail / animation / mood
	LOD,			// 0=high, 1=med, 2=low
	MOOD,			// 0=calm, 1=playful, 2=sleepy, 3=alert
	ANIM_SPEED,
	ANIM_MODE,		// 0=idle, 1=walk, 2=pounce-seq, 3=groom

	// Individuality (different-looking cats)
	FUR_ORANGE,
	FLUFFINESS,
	EAR_SIZE,
	LEG_LENGTH,
	FACE_WIDTH,
	STRIPE_AMOUNT,
	TAIL_FLUFF,		// thicker / fluffier tip

	// L-system plant next to the cat
	PLANT_DEPTH,
	PLANT_ANGLE,

	// Scene props / bells
	TEXTURE_ON,
	FRAME_ALL,
	LIGHT_DRAMA,
	SHOW_BOWL,
	SHOW_COLLAR,
	SHOW_SHADOW,
	SHOW_YARN,
	SHOW_CUSHION,
	SHOW_MOUSE,
	BLINK,
	BREATHE,

	NUMCONTROLS
};

// Colors
#define COLOR_RED		1.0f, 0.0f, 0.0f
#define COLOR_GREEN		0.0f, 1.0f, 0.0f
#define COLOR_BLUE		0.0f, 0.0f, 1.0f

#define VAL(x) (ModelerApplication::Instance()->GetControlValue(x))

#endif
