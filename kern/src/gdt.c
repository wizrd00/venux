#include "gdt.h"

static int
gdt_validate_entry_conf(const struct gdt_entry_conf *conf)
{
	if (conf->limit > 0xfffffU)
		return KERN_ERROR_INVALID_GDT_ENTRY_LIMIT;
	if (conf->flags > 0xfU)
		return KERN_ERROR_INVALID_GDT_ENTRY_FLAGS;
	return 0;
}

int
gdt_modify_entry(uint64_t *gdt, const struct gdt_entry_conf *conf,
    int index)
{
	int ret = 0;
	ret = gdt_validate_entry_conf(conf);
	if (RET_ERROR(ret))
		return ret;
	uint8_t *ent = (uint8_t *)(gdt + index);
	ent[0] = (uint8_t)(conf->limit & 0xffU);
	ent[1] = (uint8_t)((conf->limit >> 8) & 0xffU);
	ent[2] = (uint8_t)(conf->base & 0xffU);
	ent[3] = (uint8_t)((conf->base >> 8) & 0xffU);
	ent[4] = (uint8_t)((conf->base >> 16) & 0xffU);
	ent[5] = conf->access;
	ent[6] = (conf->flags << 4) | (uint8_t)(conf->limit >> 16);
	ent[7] = (uint8_t)(conf->base >> 24);
	return ret;
}

int
gdt_modify_system_entry(uint64_t *gdt, const struct gdt_entry_conf *conf,
    int index)
{
	int ret = 0;
	ret = gdt_validate_entry_conf(conf);
	if (RET_ERROR(ret))
		return ret;
	uint8_t *ent = (uint8_t *)(gdt + index);
	ent[0] = (uint8_t)(conf->limit & 0xffU);
	ent[1] = (uint8_t)((conf->limit >> 8) & 0xffU);
	ent[2] = (uint8_t)(conf->base & 0xffU);
	ent[3] = (uint8_t)((conf->base >> 8) & 0xffU);
	ent[4] = (uint8_t)((conf->base >> 16) & 0xffU);
	ent[5] = conf->access;
	ent[6] = (conf->flags << 4) | (uint8_t)(conf->limit >> 16);
	ent[7] = (uint8_t)(conf->base >> 24);
	ent[8] = (uint8_t)((conf->base >> 32) & 0xffU);
	ent[9] = (uint8_t)((conf->base >> 40) & 0xffU);
	ent[10] = (uint8_t)((conf->base >> 48) & 0xffU);
	ent[11] = (uint8_t)(conf->base >> 56);
	ent[12] = ent[13] = ent[14] = ent[15] = (uint8_t)0;
	return ret;
}
