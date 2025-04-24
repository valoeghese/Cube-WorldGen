/**
 * Hooks for Worldgen.
 */
#include <cwsdk.h>
/*
__attribute__((naked)) void ASMReturn0() {
	asm(".intel_syntax \n"
		"mov eax, 0 \n"
		"ret \n"
	);
}
*/
/*__attribute__((naked)) void ASMFloatReturn0() {
	asm(".intel_syntax \n"
		"xorps xmm0, xmm0 \n"
		"ret \n"
		".att_syntax \n"
	);
}*/
/*
__attribute__((naked)) void ASMNop() {
	asm(".intel_syntax \n"
		"ret \n"
	);
}*/

extern "C" float  __fastcall NewOverwriteWorldgen(
	cube::World *world,
	int x,
	int y,
	float density_maybe,
	float west,
	float east,
	float north,
	float south)
{
	return 1.0;
}
__attribute__((naked)) void ASMOverrideASB() {
	asm(".intel_syntax \n"
		"mov eax, 5;\n"
		"ret\n"
		".att_syntax \n");
}
/*extern "C" float __cdecl NewWorldGetZoneStructureHeight(cube::World* world, int x, int y) {
	return 80.0f;
}*/

void SetupOverwriteWorldgen() {
	char* base = (char*)CWBase();
	//WriteFarJMP(base + 0x2B4320, (void*)NewOverwriteWorldgen);
	//WriteFarJMP(base + 0x35EA0, (void*)NewWorldGetZoneStructureHeight);
	WriteFarJMP(base + 0x2AB0A0, (void*)ASMOverrideASB);
}