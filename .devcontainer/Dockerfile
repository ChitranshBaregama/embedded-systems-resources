FROM debian:bookworm-slim
RUN apt-get update && apt-get install -y --no-install-recommends \
    gcc g++ make gdb python3 git ca-certificates \
    && rm -rf /var/lib/apt/lists/*
WORKDIR /workspaces/embedded-systems-resources
