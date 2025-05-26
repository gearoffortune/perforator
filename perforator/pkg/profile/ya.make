RECURSE(
    flamegraph
    labels
    merge
    parse
    python
    quality
    samplefilter
)

IF(NOT OPENSOURCE)
    RECURSE(
        ytconv
    )
ENDIF()
