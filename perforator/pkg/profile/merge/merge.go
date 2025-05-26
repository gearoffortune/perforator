package merge

import (
	"errors"

	"github.com/google/pprof/profile"
)

var (
	ErrNoProfilesToMerge = errors.New("no profiles to merge")
)

func Merge(profiles []*profile.Profile) (*profile.Profile, error) {
	if len(profiles) == 0 {
		return nil, ErrNoProfilesToMerge
	}

	return profile.Merge(profiles)
}
