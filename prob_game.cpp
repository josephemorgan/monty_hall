#include <cstdlib>
#include <climits>
#include <ctime>
#include <iostream>
#include <thread>
#include <chrono>

enum strategy {STAY = 0, SWITCH = 1};
unsigned ITERATIONS = 1;
unsigned THREADS = 5;

struct counts
{
	unsigned stay_correct_c;
	unsigned switch_correct_c;
};

struct seed_buffered
{
	// Due to a weird quirk, not having a buffer after the seed led to different
	// threads waiting on access time for the seed variable.
	unsigned seed;
	unsigned buffer[32];
};

void simulate (struct counts&, unsigned&);
bool game (enum strategy, unsigned&);

int main (int argc, char **argv)
{
	if (argc < 2)
		std::cout << "Usage: " << argv[0] << " -i [t|s|m|l|x] -t [1|2|4|5|10]\n", exit(1);

	if (argv[1][1] == 'i') {
		switch (argv[2][0]) {
	case 's':
			ITERATIONS = 1000;
			break;
		case 'm':
			ITERATIONS = 100000;
			break;
		case 'l':
			ITERATIONS = 10000000;
			break;
		case 'x':
			ITERATIONS = 1000000000;
			break;
		default:
			std::cerr << "Invalid argument.\n", exit(1);
		}
	}

	if (argv[3][1] == 't') {
		switch (argv[4][0])
		{
		case '1':
			if (argv[4][1] != '0')
				THREADS = 1;
			else if (argv[4][1] == '0')
				THREADS = 10;
			break;
		case '2':
			THREADS = 2;
			break;
		case '4':
			THREADS = 4;
			break;
		case '5':
			THREADS = 5;
			break;
		}
	}

	srand(time(NULL));

	auto start = std::chrono::high_resolution_clock::now();
	struct counts total_counts;
	total_counts.stay_correct_c = 0;
	total_counts.switch_correct_c = 0;
	struct counts per_thread_count[THREADS];
	std::thread* threads[THREADS];
	seed_buffered seeds[THREADS];

	for (unsigned i = 0; i < THREADS; ++i) {
		seeds[i].seed = rand() % UINT_MAX;
		threads[i] = new std::thread (simulate, std::ref(per_thread_count[i]), std::ref(seeds[i].seed));
	}

	for (unsigned i = 0; i < THREADS; ++i) {
		std::cout << "Waiting for thread " << i << " to finish...\n";
		threads[i]->join();
	}

	for (unsigned i = 0; i < THREADS; ++i) {
		total_counts.stay_correct_c += per_thread_count[i].stay_correct_c;
		total_counts.switch_correct_c += per_thread_count[i].switch_correct_c;
	}

	auto stop = std::chrono::high_resolution_clock::now();
	int ex_duration = std::chrono::duration_cast<std::chrono::milliseconds>(stop - start).count();
	std::cout <<
		"The simulation performed " << ITERATIONS <<
		" iterations on " << THREADS << " threads of both the stay and switch strategies " <<
		"taking " << ex_duration <<
		" ms." << std::endl <<
		"Score:" << std::endl <<
		"  Stay Strategy: " << total_counts.stay_correct_c << std::endl <<
		"  Switch Strategy: " << total_counts.switch_correct_c << std::endl << std::endl <<
		"Ratios:" << std::endl <<
		"  Stay Strategy: " << (double)total_counts.stay_correct_c / (double)ITERATIONS << std::endl <<
		"  Switch Strategy: " << (double)total_counts.switch_correct_c / (double)ITERATIONS << std::endl << std::endl;
	return ex_duration;
}

void simulate (struct counts& c, unsigned& seed)
{
	c.stay_correct_c = 0;
	c.switch_correct_c = 0;
	for (unsigned i = 0; i < (ITERATIONS / THREADS); ++i) {
		if (game (STAY, seed))
			++c.stay_correct_c;
		if (game (SWITCH, seed))
			++c.switch_correct_c;
	}
}

bool game (enum strategy player_strat, unsigned& seed)
{
	unsigned correct_door = rand_r(&seed) % 3;
	unsigned player_choice = rand_r(&seed) % 3;
	unsigned elim_door = rand_r(&seed) % 3;
	while ((elim_door != correct_door) && (elim_door != player_choice))
		elim_door = (elim_door + 1) % 3;
	if (player_strat == SWITCH) {
		do
			player_choice = (player_choice + 1) % 3;
		while (player_choice != elim_door);
		return correct_door == player_choice;
	}
	else
		return correct_door == player_choice;
}
