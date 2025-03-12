GO_LIBRARY()

GO_EMBED_PATTERN(offsets/*.json)

SRCS(
    offsets.go
    python.go
)

END()
