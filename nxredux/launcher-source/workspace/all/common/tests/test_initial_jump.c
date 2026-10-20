// Host unit test for ui/initial_jump.h (pure, no SDL). Run via run_tests.sh,
// which builds it with AddressSanitizer.
#include <assert.h>
#include <stdio.h>
#include "../ui/initial_jump.h"

static char label_initial(void* ctx, int i) {
	const char** labels = ctx;
	return InitialJump_labelInitial(labels[i]);
}

int main(void) {
	// Normalisation: case-folded, leading spaces skipped.
	assert(InitialJump_labelInitial("abc") == 'A');
	assert(InitialJump_labelInitial("  Zed") == 'Z');
	assert(InitialJump_labelInitial("9 News") == '9');
	assert(InitialJump_labelInitial("") == '\0');
	assert(InitialJump_labelInitial(NULL) == '\0');

	// Groups: A(0,1) B(2) C(3,4,5) D(6)
	const char* rows[] = {"Alpha", "apple", "Bravo", "Charlie", " chocolate", "CNN", "Delta"};
	int n = 7;

	// Forward: from inside A -> first B; from the B singleton -> first C.
	assert(InitialJump_target(n, 0, +1, label_initial, rows) == 2);
	assert(InitialJump_target(n, 1, +1, label_initial, rows) == 2);
	assert(InitialJump_target(n, 2, +1, label_initial, rows) == 3);
	// " chocolate" belongs to C (leading space ignored), so C -> D.
	assert(InitialJump_target(n, 3, +1, label_initial, rows) == 6);
	assert(InitialJump_target(n, 4, +1, label_initial, rows) == 6);
	// Already in the last group.
	assert(InitialJump_target(n, 6, +1, label_initial, rows) == -1);

	// Backward: lands on the START of the previous group, not its end.
	assert(InitialJump_target(n, 6, -1, label_initial, rows) == 3);
	assert(InitialJump_target(n, 5, -1, label_initial, rows) == 2);
	assert(InitialJump_target(n, 3, -1, label_initial, rows) == 2);
	assert(InitialJump_target(n, 2, -1, label_initial, rows) == 0);
	// Already in the first group (even from its second row).
	assert(InitialJump_target(n, 1, -1, label_initial, rows) == -1);
	assert(InitialJump_target(n, 0, -1, label_initial, rows) == -1);

	// Out-of-range selection is clamped, never read past the list.
	assert(InitialJump_target(n, 99, -1, label_initial, rows) == 3);
	assert(InitialJump_target(n, -5, +1, label_initial, rows) == 2);

	// Degenerate lists.
	assert(InitialJump_target(1, 0, +1, label_initial, rows) == -1);
	assert(InitialJump_target(0, 0, -1, label_initial, rows) == -1);
	// Single group: nowhere to go either way.
	const char* same[] = {"a1", "A2", "a3"};
	assert(InitialJump_target(3, 1, +1, label_initial, same) == -1);
	assert(InitialJump_target(3, 1, -1, label_initial, same) == -1);

	printf("test_initial_jump: OK\n");
	return 0;
}
