#pragma once

#define STATS


#ifdef STATS
#include <cstdio>

struct Stats
{
	long unsigned started_at;

	long unsigned event;
	long unsigned net;
	long unsigned game;
	long unsigned display;

	// display buffer
	int nb_flushed;
	int total_nb_flushed;
	double size_flushed;

	long unsigned total_active;
	long unsigned slept;
	int ticks_passed;
	double average_dt=-1;
	long unsigned last;

	void reset(long unsigned now)
	{
		started_at = now;
		last = now;

		event = 0;
		net = 0;
		game = 0;
		display = 0;
		total_active = 0;
		slept = 0;
		ticks_passed = 0;

		nb_flushed = 0;
		total_nb_flushed = 0;
		size_flushed = 0;
	}

	void add_flush(int used)
	{
		nb_flushed += 1;
		total_nb_flushed += 1;
		size_flushed = used * 0.01 + size_flushed * 0.99;
	}

	void report()
	{
		printf("Stats report, %d ticks\n", ticks_passed);
		printf("Active/sleep time (ms) %lu/%lu ratio=%.5f\n", total_active, slept, (float)total_active/(slept+total_active));
		printf(" dt=%.2f fps=%.2f\n", average_dt, 1000/average_dt);
		printf(" event \t= %-10lu %.1f%%\n", event, 100.*event/total_active);
		printf(" net \t= %-10lu %.1f%%\n", net, 100.*net/total_active);
		printf(" game \t= %-10lu %.1f%%\n", game, 100.*game/total_active);
		printf(" display= %-10lu %.1f%%\n", display, 100.*display/total_active);
		printf("   %d flushed; total: %d avgsize: %.0f\n", nb_flushed, total_nb_flushed, size_flushed);
		nb_flushed = 0;
	}
};
extern Stats stats;
#define AT(something) long unsigned at_##something = get_now();
#else
#define AT(something)
#endif
