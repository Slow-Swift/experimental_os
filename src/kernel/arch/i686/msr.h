#pragma once

#include <stdbool.h>
#include <stdint.h>

bool is_msr_present();

void msr_get(uint32_t msr, uint32_t *low, uint32_t *high);

void msr_set(uint32_t msr, uint32_t low, uint32_t hight);