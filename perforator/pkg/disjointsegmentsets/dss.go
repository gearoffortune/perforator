package disjointsegmentsets

// Item is a helper interface for the Prune alrogithm
type Item interface {
	SegmentBegin() uint64
	SegmentEnd() uint64
	GenerationNumber() int
}

// Prune removes all items which overlap with newer (as determined by Generation()) ones.
// Preconditions:
// - input slice must be sorted by Start()
// - generation must be distinct for overlapping segments
// Note that input slice is modified by the call.
// Return values: (items that were retained, items that were pruned)
func Prune[T Item](items []T) ([]T, []T) {

	// Let C(s) denote retained segments for input set.
	// Key observations:
	// (1) C(s) is deterministic
	// (2) if s1 is subset of s2, and I is in s1 / C(s1), then I is in s2 / C(s2)

	// Index of the rightmost item retained so far
	last := -1

	// Indices of items to prune. This set only grows because of (2)
	invalidated := make(map[int]struct{})

	for i, item := range items {
		// additional observation following from iteration order
		// (3) if s1 is subset of s2, I and J are in C(s1), and I.end < J.begin, then I is in C(s2)
		if last == -1 || items[last].SegmentEnd() <= item.SegmentBegin() {
			last = i
			continue
		}

		if items[last].GenerationNumber() > item.GenerationNumber() {
			invalidated[i] = struct{}{}
		} else {
			invalidated[last] = struct{}{}
			// No other item except for last can be invalidated (see (3))

			last = i
		}
	}

	pruned := make([]T, 0, len(invalidated))

	it := 0
	for i := range items {
		_, has := invalidated[i]
		if has {
			pruned = append(pruned, items[i])
		} else {
			items[it] = items[i]
			it++
		}
	}
	return items[:it], pruned
}
