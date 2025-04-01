package disjointsegmentsets_test

import (
	"testing"

	"github.com/stretchr/testify/assert"

	"github.com/yandex/perforator/perforator/pkg/disjointsegmentsets"
)

type item struct {
	b uint64
	e uint64
	g int
}

func (i item) SegmentBegin() uint64 {
	return i.b
}

func (i item) SegmentEnd() uint64 {
	return i.e
}

func (i item) GenerationNumber() int {
	return i.g
}

func TestPrune(t *testing.T) {
	tests := []struct {
		name  string
		input []item
		want  []item
	}{
		{
			name: "regr1",
			input: []item{
				{b: 1, e: 3, g: 0},
				{b: 2, e: 400, g: 2},
				{b: 5, e: 10, g: 1},
			},
			want: []item{
				{b: 2, e: 400, g: 2},
			},
		},
		{
			name: "regr2",
			input: []item{
				{b: 1, e: 300, g: 0},
				{b: 2, e: 10, g: 2},
				{b: 50, e: 60, g: 1},
			},
			want: []item{
				{b: 2, e: 10, g: 2},
				{b: 50, e: 60, g: 1},
			},
		},
	}
	for _, tt := range tests {
		t.Run(tt.name, func(t *testing.T) {
			for i := range tt.input {
				if i == 0 {
					continue
				}
				if tt.input[i-1].b > tt.input[i].b {
					panic("incorrect test data: slice not sorted by SegmentBegin()")
				}
			}
			got, _ := disjointsegmentsets.Prune(tt.input)
			assert.Equal(t, tt.want, got)
		})
	}
}
