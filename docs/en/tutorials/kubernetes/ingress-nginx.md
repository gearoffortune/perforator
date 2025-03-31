# Installing NGINX Ingress Controller for Perforator
This tutorial explains how to install and configure the NGINX Ingress Controller on a Kubernetes cluster to provide access to your Perforator application deployed via Helm.
The Perforator application includes two user-facing parts: 
- **UI:** Accessed via HTTP. 
- **API:** Accessed via gRPC.
## Prerequisites
- Kubernetes cluster
- Helm 3+
- PostgreSQL database
- ClickHouse database
- S3 storage
## Install the NGINX Ingress Controller 
In this example we will deploy NGINX using the Bitnami Helm chart
### Install the Ingress Controller 
Create a namespace (e.g., `ingress-nginx`) and install the controller: 
```bash 
helm repo add ingress-nginx https://kubernetes.github.io/ingress-nginx && \
helm repo update && \
helm install my-ingress-nginx ingress-nginx/ingress-nginx --namespace ingress-nginx --create-namespace
``` 
### Local testing
For local testing, you might want to set NGINX service type to ClusterIP
```bash 
helm repo add ingress-nginx https://kubernetes.github.io/ingress-nginx && \
helm repo update && \
helm install my-ingress-nginx ingress-nginx/ingress-nginx --namespace ingress-nginx --create-namespace --set controller.service.type=ClusterIP
``` 
and configure port forwarding for TLS connection
```bash
kubectl port-forward svc/my-ingress-nginx-controller -n ingress-nginx 8080:443 
```
or for plaintext connection
```bash
kubectl port-forward svc/my-ingress-nginx-controller -n ingress-nginx 8080:80 
```
## Configure DNS Records 
To allow external access using domain names, create two DNS records with your DNS provider: 
- **HTTP UI Host:** `example.com` 
- **gRPC API Host:** `grpc.example.com` 

which point to the external IP address of the NGINX Ingress Controller.
### Local testing
For testing you can edit your `/etc/hosts` file to map the domain names to your ingress controller’s IP. For example:
```
<ingress_controller_IP> example.com
<ingress_controller_IP> grpc.example.com
```
or if you're connecting to the ingress controller via port forwarding
```
127.0.0.1 example.com 
127.0.0.1 grpc.example.com
```
## Generate TLS Certificates
Generate two key-certificate pairs, one for the HTTP endpoint and one for gRPC.
For TLS certificate management consider using tools such as [cert-manager](https://cert-manager.io/docs/).
### Local testing
For testing environments, you can generate self-signed certificates using OpenSSL.
#### Generate a Certificate for HTTP host 
```bash 
openssl req -x509 -nodes -days 365 \
 -newkey rsa:2048 \
 -keyout http-tls.key \
 -out http-tls.crt \
 -subj "/CN=example.com" 
``` 
#### Generate a Certificate for gRPC host
```bash 
openssl req -x509 -nodes -days 365 \
 -newkey rsa:2048 \
 -keyout grpc-tls.key \
 -out grpc-tls.crt \
 -subj "/CN=grpc.example.com" 
``` 
Adjust the certificate's Common Name (CN) accordingly.
#### Create Kubernetes TLS Secrets Replace `<namespace>` with the appropriate namespace for your Perforator deployment.
- **For HTTP host:** 
```bash 
kubectl create secret tls http-tls-secret \
 --key=http-tls.key --cert=http-tls.crt \
 -n <namespace> 
``` 
- **For gRPC host:** 
```bash 
kubectl create secret tls grpc-tls-secret \
 --key=grpc-tls.key --cert=grpc-tls.crt \
 -n <namespace> 
```
## Deploy Perforator Using Helm 
{% note info %}

If you haven't installed the Perforator Helm chart yet, see [this guide](../../guides/kubernetes/helm-chart.md) for step-by-step instructions

{% endnote %}

Add these ingress parameters to your values.yaml file, replacing the corresponding host and secret names as needed.

```yaml
ingress:
  http:
    enabled: true
    className: "nginx"
    hosts:
      - host: example.com
        paths:
          - path: /
            pathType: Prefix
    tls:
      - hosts:
          - example.com
        secretName: http-tls-secret
  grpc:
    enabled: true
    annotations: 
      nginx.ingress.kubernetes.io/backend-protocol: "GRPC"
    className: "nginx"
    hosts:
      - host: grpc.example.com
        paths:
          - path: /
            pathType: Prefix
    tls:
      - hosts:
          - grpc.example.com
        secretName: grpc-tls-secret
```
Upgrate your Perforator deploymnet:

```bash
helm upgrade <my-perforator-release> -n <namespace> perforator/perforator -f <my-values.yaml>
```
## Verify the Deployment 
To access the Perforator UI, open the corresponding HTTP host in your browser and accept the self-signed certificate warning if prompted.

To verify grpc endpoint list services with [perforator cli](../../guides/cli/install.md)
```bash
perforator list services --url grpc.example.com
```