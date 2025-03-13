package samplefilter

import (
	"testing"

	pprof "github.com/google/pprof/profile"
	"github.com/stretchr/testify/assert"
	"github.com/stretchr/testify/require"

	"github.com/yandex/perforator/perforator/pkg/profilequerylang"
)

func TestBuildBuildIDFilter(t *testing.T) {
	for _, test := range []struct {
		name   string
		query  string
		error  bool
		filter string
	}{
		{
			name:   "EmptySelector",
			query:  "{}",
			filter: "",
		},
		{
			name:   "UnrelatedMatchers",
			query:  "{env.foo=\"123\"}",
			filter: "",
		},
		{
			name:  "UnsupportedMultiValue",
			query: "{build_ids=\"123|456\"}",
			error: true,
		},
		{
			name:  "UnsupportedNotEqual",
			query: "{build_ids!=\"123\"}",
			error: true,
		},
		{
			name:   "OK",
			query:  "{build_ids=\"123\"}",
			filter: "123",
		},
	} {
		t.Run(test.name, func(t *testing.T) {
			sel, err := profilequerylang.ParseSelector(test.query)
			require.NoError(t, err)
			f, err := BuildBuildIDFilter(sel)
			if test.error {
				assert.Error(t, err)
				return
			} else {
				if assert.NoError(t, err) {
					assert.Equal(t, buildIDFilter(test.filter), f)
				}
			}
		})
	}
}

func TestBuildIDFilterMatch(t *testing.T) {
	tests := []struct {
		name     string
		filter   string
		buildIDs []string
		expected bool
	}{
		{
			name:     "EmptyFilterMatchesAnyBuildID",
			filter:   "",
			buildIDs: []string{"123", "456"},
			expected: true,
		},
		{
			name:     "EmptyFilterMatchesMissingBuildID",
			filter:   "",
			buildIDs: []string{"123"},
			expected: true,
		},
		{
			name:     "SimpleMatch",
			filter:   "123",
			buildIDs: []string{"123"},
			expected: true,
		},
		{
			name:     "SimpleNoMatch",
			filter:   "123",
			buildIDs: []string{"456"},
			expected: false,
		},
	}
	for _, test := range tests {
		t.Run(test.name, func(t *testing.T) {
			f := buildIDFilter(test.filter)
			s := &pprof.Sample{}
			for _, b := range test.buildIDs {
				s.Location = append(s.Location, &pprof.Location{
					Mapping: &pprof.Mapping{
						BuildID: b,
					},
				})
			}
			assert.Equal(t, test.expected, f.Matches(s))
		})
	}
}
