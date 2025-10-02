import json
import redis


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

        channel: str = msg.get("channel").decode("utf-8")
        payload: dict = json.loads(msg.get("data").decode("utf-8"))
        payload["channel"]: str = channel

        print(json.dumps(payload, indent=4), "\n")


if __name__ == "__main__":
    main()
