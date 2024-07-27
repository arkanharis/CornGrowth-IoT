from fastapi import FastAPI, BackgroundTasks
from pydantic import BaseModel
from motor.motor_asyncio import AsyncIOMotorClient
from datetime import datetime, timedelta
import statistics
import asyncio
import logging

app = FastAPI()

# Konfigurasi koneksi MongoDB Atlas
MONGO_DETAILS = "mongodb+srv://irfnriza:monggoCorn123@cornenvironment.mwtc0sc.mongodb.net/?retryWrites=true&w=majority&appName=CornEnvironment"
client = AsyncIOMotorClient(MONGO_DETAILS)
db = client['environment']
collection = db['sensor_data']

# Memori sementara untuk menyimpan data sensor
sensor_data = []

# Model untuk data yang diterima dari ESP32
class SensorData(BaseModel):
    temperature: float
    humidity: float
    soil_moisture: float
    timestamp: datetime = datetime.utcnow()

@app.post("/data")
async def receive_data(data: SensorData):
    """
    Endpoint untuk menerima data sensor dari ESP32.
    
    Parameter:
    - data: SensorData - Data sensor yang dikirim oleh ESP32
    
    Return:
    - Dictionary dengan pesan konfirmasi
    """
    sensor_data.append(data.dict())
    return {"message": "Data received successfully"}

async def calculate_and_store_average():
    while True:
        try:
            await asyncio.sleep(120) 
            
            if sensor_data:
                temperatures = [data['temperature'] for data in sensor_data]
                humidities = [data['humidity'] for data in sensor_data]
                soil_moistures = [data['soil_moisture'] for data in sensor_data]
                
                average_data = {
                    "temperature": statistics.mean(temperatures),
                    "humidity": statistics.mean(humidities),
                    "soil_moisture": statistics.mean(soil_moistures),
                    "timestamp": datetime.utcnow()
                }

                await collection.insert_one(average_data)
                sensor_data.clear()

        except Exception as e:
            logging.error(f"An error occurred: {e}")

@app.on_event("startup")
async def startup_event():
    asyncio.create_task(calculate_and_store_average())