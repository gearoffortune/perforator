# Perforator Helm Chart
In this tutorial we will deploy Perforator on a Kubernetes cluster via the Helm package manager without additional database setup.

{% note warning %}

This setup is intended solely for demonstration.
There is no guarantee for compatibility with older versions of the chart.
Please do not use it in production environments.

{% endnote %}

## Prerequisites

- Kubernetes cluster
- Helm 3+

## Adding Helm Repository

```
helm repo add perforator https://helm.perforator.tech
helm repo update
```

## Installing Helm Chart

Create file `my-values.yaml` with values:

my-values.yaml example
```yaml
databases:
  postgresql:
    migrations:
      enabled: true
    db: "perforator"
    user: "perforator"
    password: "perforator"
  clickhouse:
    migrations:
      enabled: true
    insecure: true
    db: "perforator"
    user: "perforator"
    password: "perforator"
  s3:
    buckets:
      profiles: "perforator-profile"
      binaries: "perforator-binary"
      taskResults: "perforator-task-results"
      binariesGSYM: "perforator-binary-gsym"
    insecure: true
    force_path_style: true
    accessKey: "perforator"
    secretKey: "perforator"

proxy:
  url_prefix: "http://localhost:8080/static/results/"

testing:
  enableTestingDatabases: true
```

Use created values to install chart:

```console
helm install perforator-release -n perforator perforator/perforator -f my-values.yaml --create-namespace
```

## Connecting to Perforator UI

To access the Perforator UI, configure port forwarding to the local machine:

```console
kubectl port-forward svc/perforator-release-perforator-web-service -n perforator 8080:80
```
Then open `http://localhost:8080` in your browser

## Uninstalling Helm Chart

```console
helm uninstall perforator-release -n perforator
```
