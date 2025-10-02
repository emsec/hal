import sys
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

        print("")


if __name__ == "__main__":
    main()
