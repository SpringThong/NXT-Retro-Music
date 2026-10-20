#ifndef __INITIAL_JUMP_H__
#define __INITIAL_JUMP_H__

// Pure first-letter group navigation for an alphabetically sorted list (the
// L1/R1 "jump by initial" behaviour shared by ListView and the IPTV channel
// browser). Header-only and widget-agnostic (the caller supplies an initial-
// lookup callback) so it can be unit-tested on the host without SDL: see
// tests/test_initial_jump.c.

// Returns the normalised initial of row `index` (see InitialJump_labelInitial).
typedef char (*InitialJump_initialFn)(void* ctx, int index);

// Normalised initial of a label: leading spaces skipped, a-z folded to A-Z,
// so rows compare by the same letter regardless of case or stray padding.
static inline char InitialJump_labelInitial(const char* s) {
	if (!s)
		return '\0';
	while (*s == ' ')
		s++;
	char c = *s;
	if (c >= 'a' && c <= 'z')
		c = (char)(c - 'a' + 'A');
	return c;
}

// Index of the first row of the next (dir > 0) or previous (dir < 0)
// first-letter group relative to `selected`, or -1 when `selected` is already
// in the last (resp. first) group or the list has fewer than two rows.
// `selected` is clamped into range before the walk.
static inline int InitialJump_target(int count, int selected, int dir, InitialJump_initialFn initial, void* ctx) {
	if (count <= 1 || !initial)
		return -1;
	int sel = selected;
	if (sel < 0)
		sel = 0;
	if (sel >= count)
		sel = count - 1;
	char cur = initial(ctx, sel);
	if (dir > 0) {
		int i = sel + 1;
		while (i < count && initial(ctx, i) == cur)
			i++;
		if (i >= count)
			return -1; // already in the last letter group
		return i;	   // first row of the next group
	}
	int i = sel - 1;
	while (i >= 0 && initial(ctx, i) == cur)
		i--;
	if (i < 0)
		return -1; // already in the first letter group
	// i is the last row of the previous group; walk back to its start.
	char prev = initial(ctx, i);
	while (i > 0 && initial(ctx, i - 1) == prev)
		i--;
	return i;
}

#endif // __INITIAL_JUMP_H__
