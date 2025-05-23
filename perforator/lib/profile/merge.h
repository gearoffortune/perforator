#pragma once

#include <perforator/proto/profile/profile.pb.h>
#include <perforator/proto/profile/merge_options.pb.h>

#include <util/generic/array_ref.h>


namespace NPerforator::NProfile {

////////////////////////////////////////////////////////////////////////////////

// NB: @TProfileMerger is single-threaded and not thread-safe.
class TProfileMerger {
public:
    TProfileMerger(
        NProto::NProfile::Profile* merged,
        const NProto::NProfile::MergeOptions& options
    );

    ~TProfileMerger();

    // Merge one profile into the resulting one.
    // This function is not thread safe.
    void Add(const NProto::NProfile::Profile& profile);

    // Do some bookkeeping work to finish merging.
    // You must call TProfileMerger::Finish() after TProfileMerger::Add().
    // This function is not thread safe.
    void Finish() &&;

private:
    class TImpl;
    THolder<TImpl> Impl_;
};

////////////////////////////////////////////////////////////////////////////////

// Convenience function for a small number of profiles. Prefer to use
// TProfileMerger directly to save memory: you do not have to keep all
// the profiles in RAM.
void MergeProfiles(
    TConstArrayRef<NProto::NProfile::Profile> profiles,
    NProto::NProfile::Profile* merged,
    const NProto::NProfile::MergeOptions& options = {}
);

////////////////////////////////////////////////////////////////////////////////

} // namespace NPerforator::NProfile
