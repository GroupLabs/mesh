from fastapi import FastAPI
from pydantic import BaseModel

app = FastAPI()

class Message(BaseModel):
    message: str

class Reply(BaseModel):
    reply: str

@app.post("/receive_message", response_model=Reply)
async def receive_message(message: Message):
    return {"reply": f"Server received: {message.message}"}



# gunicorn -c gunicorn_config.py main:app