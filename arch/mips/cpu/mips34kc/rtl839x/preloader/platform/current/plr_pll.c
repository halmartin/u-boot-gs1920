#include <preloader.h>
#include <soc.h>
#include <cpu_utils.h>
#include <plr_pll_gen2.h>

void pll_setup(void) {
	//printf("\rII: PLL... ");

	parameters._pll_query_freq = pll_query_freq;
	parameters._udelay = udelay;
	pll_gen2_setup();

	//printf("OK\n");
	return;
}
