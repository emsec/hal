import sys
import redis

try:
    import helix_pb2 as hx
except ImportError:
    sys.stderr.write("Error: Missing generated Python code. Please compile 'helix.proto' using 'protoc' before running this script.\n")
    sys.exit(1)


CHANNELS: list[str] = ["hal", "application1", "application2"]


def main() -> None:
    client: redis.Redis = redis.Redis(host='localhost', port=6379, db=0)
    pubsub: redis.client.PubSub = client.pubsub()

    for channel in CHANNELS:
        pubsub.subscribe(channel)
        print(f"Subscribed to '{channel}'")
    print()

    for msg in pubsub.listen():
        if msg.get("type") != "message":
            continue

        message: hx.Message = hx.Message()
        message.ParseFromString(msg.get("data"))
        print(f"{message}")


if __name__ == "__main__":
    main()
