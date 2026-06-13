import asyncio
from someipy import (
    ServiceBuilder,
    ServerServiceInstance,
    EventGroup,
    Event,
    TransportLayerProtocol,
    connect_to_someipy_daemon,
)

SPEED_SERVICE_ID   = 0x2222
SPEED_INSTANCE_ID  = 0x3333
SPEED_EVENT_ID     = 0x8002
SPEED_EVENT_GROUP  = 0x0002
INTERFACE_IP       = "192.168.1.10"
SPEED_ECU_PORT     = 4000

async def main():
    someipy_daemon = await connect_to_someipy_daemon()

    speed_event = Event(
        id=SPEED_EVENT_ID,
        protocol=TransportLayerProtocol.UDP,
    )

    speed_eventgroup = EventGroup(
        id=SPEED_EVENT_GROUP,
        events=[speed_event],
    )

    speed_service = (
        ServiceBuilder()
        .with_service_id(SPEED_SERVICE_ID)
        .with_major_version(1)
        .with_eventgroup(speed_eventgroup)
        .build()
    )

    speed_ecu = ServerServiceInstance(
        daemon=someipy_daemon,
        service=speed_service,
        instance_id=SPEED_INSTANCE_ID,
        endpoint_ip=INTERFACE_IP,
        endpoint_port=SPEED_ECU_PORT,
        ttl=5,
        cyclic_offer_delay_ms=2000,
    )

    await speed_ecu.start_offer()
    print("Speed ECU started - broadcasting vehicle speed...")

    speed = 0
    direction = 10

    try:
        while True:
            await asyncio.sleep(2)
            payload = bytes([speed])
            speed_ecu.send_event(SPEED_EVENT_GROUP, SPEED_EVENT_ID, payload)
            print(f"Speed ECU: Broadcasting speed = {speed} km/h")
            speed += direction
            if speed >= 120: direction = -10
            if speed <= 0:   direction = 10
    finally:
        await speed_ecu.stop_offer()
        await someipy_daemon.disconnect_from_daemon()

if __name__ == "__main__":
    asyncio.run(main())
