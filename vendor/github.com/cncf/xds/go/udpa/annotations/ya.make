GO_LIBRARY()

LICENSE(Apache-2.0)

VERSION(v0.0.0-20240905190251-b4127c9b8d78)

SRCS(
    migrate.pb.go
    migrate.pb.validate.go
    security.pb.go
    security.pb.validate.go
    sensitive.pb.go
    sensitive.pb.validate.go
    status.pb.go
    status.pb.validate.go
    versioning.pb.go
    versioning.pb.validate.go
)

END()
