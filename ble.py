import asyncio
from bleak import BleakClient, BleakScanner
import time
import logging

logging.basicConfig(level=logging.DEBUG)

async def write_hex_characteristic(client, message, characteristic):
    try:
        command = bytes.fromhex(message)  # Command to send
        print(f"Writing '{command}' to characteristic {characteristic}...")
        await client.write_gatt_char(characteristic, command, response=False)
    except Exception as e:
        print(f"Failed to write/read characteristic {characteristic}: {e}")

async def write_string_characteristic(client, message, characteristic):
    try:
        command = str.encode(message)  # Command to send
        print(f"Writing '{command}' to characteristic {characteristic}...")
        await client.write_gatt_char(characteristic, command, response=False)
    except Exception as e:
        print(f"Failed to write/read characteristic {characteristic}: {e}")

# Define the callback for handling notifications
async def notification_handler(sender, data):
    print(f"Notification from {sender}: {data.hex()}")

async def list_and_interact_with_attributes(device_address):
    print(f"Connecting to device {device_address}...")
    try:
        async with BleakClient(device_address) as client:
            if await client.is_connected():
                print(f"Connected to {device_address}")
                time.sleep(1)
                await client.start_notify("6d581e74-15c6-11ec-82a8-0002a5d5c51b", notification_handler)
                time.sleep(1)
                await write_string_characteristic(client, "+VER?", "6d581e71-15c6-11ec-82a8-0002a5d5c51b")
            #     time.sleep(1)
            #     await write_string_characteristic(client, "+PM?", "6d581e71-15c6-11ec-82a8-0002a5d5c51b")
            #     challenge_response = input("Challenge response: ")
            #     await write_string_characteristic(client, f"+PM<{challenge_response}", "6d581e71-15c6-11ec-82a8-0002a5d5c51b")
            #     challenge = input("Challenge for scooter: ")
            #     await write_string_characteristic(client, f"+PA<{challenge}", "6d581e71-15c6-11ec-82a8-0002a5d5c51b")
            #     time.sleep(1)
            #     await write_string_characteristic(client, "CODE=000000", "6d581e71-15c6-11ec-82a8-0002a5d5c51b")
            #     time.sleep(1)
            #     await write_string_characteristic(client, "GETDEVID", "6d581e71-15c6-11ec-82a8-0002a5d5c51b")
            #     time.sleep(1)
            #     # await write_string_characteristic(client, "+UNIT=?", "6d581e71-15c6-11ec-82a8-0002a5d5c51b")
            #     await write_string_characteristic(client, "+UNIT=?", "6d581e71-15c6-11ec-82a8-0002a5d5c51b")
            #     time.sleep(1)
            #     # await write_string_characteristic(client, "+MODE=?", "6d581e71-15c6-11ec-82a8-0002a5d5c51b")
            #     await write_string_characteristic(client, "+MODE=?", "6d581e71-15c6-11ec-82a8-0002a5d5c51b")
            #     time.sleep(1)
            #     # await write_string_characteristic(client, "HLGT=?", "6d581e71-15c6-11ec-82a8-0002a5d5c51b")
            #     await write_string_characteristic(client, "HLGT=?", "6d581e71-15c6-11ec-82a8-0002a5d5c51b")
            #     time.sleep(1)
            #     # await write_string_characteristic(client, "+LOCK=?", "6d581e71-15c6-11ec-82a8-0002a5d5c51b")
            #     await write_string_characteristic(client, "+LOCK=?", "6d581e71-15c6-11ec-82a8-0002a5d5c51b")
            #     for hex in [
            #         "aa03040488010020bb", #lock
            #         "aa03040488000021bb", #unlock
            #         "aa03040288010026bb", #lamp aan
            #         "aa03040288000027bb", #lamp uit
            #         "aa03040188010025bb", #eco
            #         "aa03040188020026bb", #drive
            #         "aa03040188030027bb", #sport
            #         "aa03040188000024bb", #walk
            #         "aa03040188010025bb", #eco
            #     ]:
            #         time.sleep(1)
            #         await write_hex_characteristic(client, hex, "6d581e71-15c6-11ec-82a8-0002a5d5c51b")
            # else:
            #     print(f"Failed to connect to {device_address}.")
    except Exception as e:
        print(f"An error occurred: {e}")

async def main():
    print("Scanning for Bluetooth devices...")
    devices = await BleakScanner.discover()

    if not devices:
        print("No devices found.")
        return

    # print("Available devices:")
    # for idx, device in enumerate(devices, start=1):
    #     print(f"{idx}. {device.name or 'Unknown'} ({device.address})")

    # # Let the user select a device to connect to
    # selected_idx = int(input("Select a device by number: ")) - 1
    # if 0 <= selected_idx < len(devices):
    #     selected_device = devices[selected_idx]
    #     await list_and_interact_with_attributes("11:22:38:03:01:FA")
    # else:
    #     print("Invalid selection.")

    await list_and_interact_with_attributes("11:22:38:03:01:FA")

if __name__ == "__main__":
    asyncio.run(main())
