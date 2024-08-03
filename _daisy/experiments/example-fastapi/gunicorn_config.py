# gunicorn_config.py

import multiprocessing

# Calculate the number of workers based on the number of CPUs available
workers = multiprocessing.cpu_count() * 2 + 1

# Use Uvicorn as the worker class
worker_class = "uvicorn.workers.UvicornWorker"

# Address and port to bind
bind = "0.0.0.0:8000"

# Log level
loglevel = "info"

# Set timeout to 30 seconds to avoid hanging requests
timeout = 30

# Keep-alive settings to ensure connections are reused
keepalive = 120