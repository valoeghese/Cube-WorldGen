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

extern "C"  cube::BlockProperties<float>*  __fastcall NewCalculateTopBlock(
    cube::World* self,
    cube::BlockProperties<float>* block_properties,
    signed int x,
    signed int y,
    float z_coord,
    float field_field_8,
    float height_structure_west,
    float height_structure_east,
    float height_structure_north,
    float height_structure_south,
    float field_field_14,
    float field_field_C,
    float field_field_10,
    float field_field_28,
    float zero_maybe,
    float field_field_38,
    int field_field_18,
    int field_field_1C,
    int field_field_20,
    int zero_maybe_1,
    int zero_maybe_2)
{
    block_properties->blue = 25;
    block_properties->red = 150;
    block_properties->green = 70;
    block_properties->type = (cube::BlockProperties<float>::Type)1;
    return block_properties;
}

void SetupOverwriteWorldgen() {
	char* base = (char*)CWBase();
	WriteFarJMP(base + 0x2D8200, (void*)NewCalculateTopBlock);
}