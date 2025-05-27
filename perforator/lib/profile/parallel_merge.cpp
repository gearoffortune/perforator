#include "parallel_merge.h"
#include "merge.h"

#include <util/generic/deque.h>
#include <util/generic/hash_set.h>
#include <util/datetime/base.h>
#include <library/cpp/threading/blocking_queue/blocking_queue.h>
#include <library/cpp/threading/future/future.h>
#include <library/cpp/threading/future/async.h>
#include <library/cpp/threading/future/wait/wait.h>

namespace NPerforator::NProfile {

class TParallelProfileMerger::TImpl {
public:
    TImpl(
        NProto::NProfile::Profile* merged,
        TParallelProfileMergerOptions options,
        IThreadPool* pool
    );

    void Add(const NProto::NProfile::Profile& profile);

    void Finish() &&;

private:
    void WorkerThread(ui32 workerId);

private:
    NProto::NProfile::Profile* Merged_;
    TParallelProfileMergerOptions Options_;
    IThreadPool* Pool_;

    NThreading::TBlockingQueue<NProto::NProfile::Profile> PendingProfiles_;
    TDeque<NProto::NProfile::Profile> IntermediateProfiles_;
    TVector<NThreading::TFuture<void>> WorkerFutures_;
};

TParallelProfileMerger::TImpl::TImpl(
    NProto::NProfile::Profile* merged,
    TParallelProfileMergerOptions options,
    IThreadPool* pool
)
    : Merged_(merged)
    , Options_(options)
    , Pool_(pool)
    , PendingProfiles_(Options_.BufferSize)
    , IntermediateProfiles_(Options_.ConcurrencyLevel)
{
    WorkerFutures_.reserve(Options_.ConcurrencyLevel);
    for (ui32 i = 0; i < Options_.ConcurrencyLevel; ++i) {
        WorkerFutures_.push_back(
            NThreading::Async([this, i] {
                WorkerThread(i);
            }, *Pool_)
        );
    }
}

void TParallelProfileMerger::TImpl::Add(const NProto::NProfile::Profile& profile) {
    PendingProfiles_.Push(profile);
}

void TParallelProfileMerger::TImpl::Finish() && {
    // Stop the queue to signal workers to finish
    PendingProfiles_.Stop();

    // Wait for all workers to complete - will throw on first worker exception
    NThreading::WaitAll(WorkerFutures_).GetValueSync();

    // All workers succeeded - proceed with final merge
    TProfileMerger finalMerger{Merged_, Options_.MergeOptions};

    for (auto& profile : IntermediateProfiles_) {
        if (profile.samples().key_size() > 0) {
            finalMerger.Add(profile);
        }
    }

    std::move(finalMerger).Finish();
}

void TParallelProfileMerger::TImpl::WorkerThread(ui32 workerId) {
    TProfileMerger merger{&IntermediateProfiles_[workerId], Options_.MergeOptions};

    while (auto maybeProfile = PendingProfiles_.Pop()) {
        merger.Add(maybeProfile.GetRef());
    }

    std::move(merger).Finish();
}

TParallelProfileMerger::TParallelProfileMerger(
    NProto::NProfile::Profile* merged,
    TParallelProfileMergerOptions options,
    IThreadPool* pool
)
    : Impl_(new TImpl(merged, options, pool))
{}

TParallelProfileMerger::~TParallelProfileMerger() {
}

void TParallelProfileMerger::Add(const NProto::NProfile::Profile& profile) {
    Impl_->Add(profile);
}

void TParallelProfileMerger::Finish() && {
    std::move(*Impl_).Finish();
}

} // namespace NPerforator::NProfile
