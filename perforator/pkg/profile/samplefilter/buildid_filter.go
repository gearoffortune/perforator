package samplefilter

import (
	"fmt"

	pprof "github.com/google/pprof/profile"

	"github.com/yandex/perforator/observability/lib/querylang"
	"github.com/yandex/perforator/perforator/pkg/profilequerylang"
)

type buildIDFilter string

const (
	nopBuildIDFilter    buildIDFilter = ""
	buildIDMatcherField string        = "build_ids"
)

func (bf buildIDFilter) Matches(sample *pprof.Sample) bool {
	if bf == nopBuildIDFilter {
		return true
	}
	for _, location := range sample.Location {
		if location.Mapping != nil && location.Mapping.BuildID == string(bf) {
			return true
		}
	}
	return false
}

func BuildBuildIDFilter(selector *querylang.Selector) (SampleFilter, error) {
	for _, matcher := range selector.Matchers {
		if matcher.Field != buildIDMatcherField {
			continue
		}
		val, err := profilequerylang.ExtractEqualityMatch(matcher)
		if err != nil {
			return nil, fmt.Errorf("failed to extract desired build id: %w", err)
		}
		return buildIDFilter(val), nil
	}
	return nopBuildIDFilter, nil
}
