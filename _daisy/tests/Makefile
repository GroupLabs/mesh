# Variables
FLATC = ../flatc
SCHEMA = ../flatbuffers/model_and_tensor.fbs
OUTPUT_DIR = ./
SERVICE_DEF_PATH = ./myservice
MODEL_PATH=../torchscript/simple_model.pt

# Default target: Generate Python classes
fbs:
	@mkdir -p $(OUTPUT_DIR)  # Create output directory if it doesn't exist
	$(FLATC) --python -o $(OUTPUT_DIR) $(SCHEMA)

test: .venv/bin/activate
	.venv/bin/python client.py $(MODEL_PATH) 10

clean:
	@rm -rf $(SERVICE_DEF_PATH)

# Phony targets
.PHONY: all clean
